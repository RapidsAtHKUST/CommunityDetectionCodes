#include <cerrno>
#include <bitset> // to help find the distinct vertices
#include <algorithm>
#include <set>

#include <fcntl.h>
#include <string.h>
#include <sys/mman.h> // for mmap(2)
#include <sys/stat.h> // for fstat64(2)
#include <signal.h>

#include "graph_loading.hpp"
#include "options.hpp"

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>

int option_maxDegree = 0; 

using namespace boost::multi_index;

namespace graph_loading {

struct readEdgeInvalidLineInDataException : public exception { }; // TODO: Proper error message, to be throw by readEdge()
static const char *readEdge(const char *cur, int &l, int &r) throw (readEdgeInvalidLineInDataException);
static const char *readEdge(const char *cur, string &l, string &r) throw (readEdgeInvalidLineInDataException);

template <class Name> class RangeOfEdges {
private:
	const char * p;
	const char * updating_p;
	const char * const fileEnd;
	Name l,r;
	void read_a_row() { updating_p = readEdge(p, l, r); }
public:
	RangeOfEdges(const char* b, const char* e) : p(b), fileEnd(e) { if(p!=fileEnd) read_a_row(); }
	pair<Name, Name> front() { return make_pair(l,r); }
	bool empty() { return p == fileEnd; }
	void popFront() { p = updating_p ; if(p!=fileEnd) read_a_row(); }
};
	
template <class Name> static void loadBloomGraphMMAP(bloomGraph<Name> &bg, const char *fileName);
pair<const char *, const char *> mmapFile(const char *fileName);
void loadBloomGraphMMAPFastButFussy(SimpleIntGraph &bg, const char *fileName);
void findDistinctVertices(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg);
void countDegrees(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg);
void convertDegreesToOffsets(SimpleIntGraph &bg);
static void populateNameToIDHash(SimpleIntGraph &bg); // A hash used temporarily during graph loading to test if a given edge has new nodes in it.
static void populateNameToIDHash(SimpleStringGraph &bg); // A hash used temporarily during graph loading to test if a given edge has new nodes in it.
static void addNeighbour(VertexIDType edge_source, VertexIDType edge_target, VertexIDType _defaultVertexID, V &_edges_added, SimpleIntGraph &_bg);
void loadEdges(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg);

void loadSimpleStringGraphFromFile(SimpleStringGraph &g, const char *fileName) {
	Timer timer(__FUNCTION__);
	loadBloomGraphMMAP<string>(g, fileName);
}
void loadSimpleIntGraphFromFile(SimpleIntGraph &bg, const char *fileName) {
	Timer timer(__FUNCTION__);
	loadBloomGraphMMAP<int>(bg, fileName);
}

struct DegreeTag {} ;
struct NameTag {} ;
template <class N> void DegreeIncrementer(pair<N,V> &p) { ++p.second; }
template <class N> void DegreeDecrementer(pair<N,V> &p) { --p.second; }

enum state_flag { LOADING_NONE, LOADING_NODES, LOADING_DEGREEMAP, LOADING_EDGES, LOADING_COMPLETE };
struct lsdkfj {
	enum state_flag state;
	int64 edge_counter;
} sigUSR1_state;
void sigUSR1_handler(int) {
	Pn("SIGUSR1: %20s edge_counter %Ld"
		, sigUSR1_state.state == LOADING_NONE ? "LOADING_NONE"
		: sigUSR1_state.state == LOADING_NODES ? "LOADING_NODES"
		: sigUSR1_state.state == LOADING_DEGREEMAP ? "LOADING_DEGREEMAP"
		: sigUSR1_state.state == LOADING_EDGES ? "LOADING_EDGES"
		: "LOADING_COMPLETE"
		, sigUSR1_state.edge_counter
		);
}

template <class Name> static void loadBloomGraphMMAP(bloomGraph<Name> &bg, const char *fileName) {
	const char *fileBegin, *fileEnd;
	make_refpair(fileBegin, fileEnd) = mmapFile(fileName);
	{ // skip over option_ignoreNheaderlines;
		Pn("--ignoreNlines=%d", option_ignoreNlines);
		for(int i=0; i<option_ignoreNlines; i++) {
			const char *nextLine = 1+strchr(fileBegin, '\n');
			fileBegin = nextLine;
		}
	}
	const RangeOfEdges<Name> roe(fileBegin, fileEnd); // Never un-const this. Take a copy of it if necessary.

	{
		sigUSR1_state.state = LOADING_NONE;
		signal(SIGUSR1, sigUSR1_handler);
		Pn("installing handler for SIGUSR1. Send it to get data-loading stats");
	}

	typedef Name N;
	typedef pair<N,N> Pair;

	set <N> *names = new set<N>;

	typedef pair<Name,V> PNV;
	multi_index_container <
		PNV,
		indexed_by<
			ordered_non_unique < tag<DegreeTag>, member<PNV,V,&PNV::second>, greater<V> >, // keep the highest degree nodes at the front of this index
			hashed_unique      < tag<NameTag>, member<PNV,N,&PNV::first>              > // for ease up updating the degree of any given node
		>
	> degree;
	//PP(degree.size());
	//PP(degree.get<DegreeTag>().count(0));

	set <pair<N ,N> > edges;

	{ // populate the list of distinct vertices
		sigUSR1_state.state = LOADING_NODES;
		sigUSR1_state.edge_counter = 0;
		Timer timer("list of distinct vertices");
		RangeOfEdges<Name> roe2 (roe);
		pair<Name,Name> edge;
		Foreach(edge , roe2) {
			sigUSR1_state.edge_counter ++;
			if(edge.first != edge.second) {
				names->insert(edge.first);
				names->insert(edge.second);
				degree.insert(make_pair(edge.first, 0));
				degree.insert(make_pair(edge.second, 0));
			}
		}
	}
	{
		Timer timer("make the degree map, while gathering all (distinct) edges");
		RangeOfEdges<Name> roe2 (roe);
		Pair edge;
		sigUSR1_state.state = LOADING_DEGREEMAP;
		sigUSR1_state.edge_counter = 0;
		Foreach(edge , roe2) {
			sigUSR1_state.edge_counter++;
			if(edge.first != edge.second) {
				assert(degree.get<NameTag>().count(edge.first)==1);
				assert(degree.get<NameTag>().count(edge.second)==1);
				bool was_inserted = edges.insert(edge).second;
				swap(edge.first, edge.second);
				bool was_inserted2= edges.insert(edge).second; // we have to add it both ways here. Let's not worry about the efficiency of this yet!
				assert(was_inserted == was_inserted2);

				if(was_inserted) {
					degree.get<NameTag>().modify(
						degree.get<NameTag>().find(edge.first)
						, DegreeIncrementer<Name>
					);
					degree.get<NameTag>().modify(
						degree.get<NameTag>().find(edge.second)
						, DegreeIncrementer<Name>
					);
				}
			}
		}
	}
	{

	if(option_maxDegree!=0) { // 0 will mean do nothing, the default, don't delete any edges.
		V max_degree;
		if(option_maxDegree==-1) {
			max_degree = int(2.0 * double(edges.size()) / names->size());
			Pn("Deleting edges such that max degree (%d) equals TWICE average (uncorrected) degree (%g)", max_degree, double(edges.size()) / names->size());
		} else {
			max_degree = option_maxDegree;
			Pn("Deleting edges such that max degree is %d", max_degree);
		}
		cout << "before deletion, "; PP(edges.size() / 2);
		cout << "before deletion, "; PP(names->size());
		Timer timer("deleting high degree nodes");
		PP(degree.get<DegreeTag>().begin()->second);
		// we could remove nodes and edges here, just remember to delete from names if necessary.
		// const V max_degree = 1000; // fail at neighbours_left==3
		// const V max_degree = 400,000; // 2 nodes  3.2 s
		// const V max_degree = 100,000; // 4 nodes 11s
		// const V max_degree =  10,000; // 7 nodes 14s
		// const V max_degree =   1,000; //   nodes 16s
		// const V max_degree =     100; //   nodes 20s
		while(degree.get<DegreeTag>().begin()->second > max_degree) {
			const N n = degree.get<DegreeTag>().begin()->first; // this is the node with highest degree. Delete one of its edges
			const V current_degree = degree.get<DegreeTag>().begin()->second;
			// cout << '\"' << n << '\"' << "\tdegree" << current_degree << endl;
			typename set <pair<N ,N> >::iterator i;
			i = edges.lower_bound(make_pair(n,N()) );
			assert(i->first == n);
			V to_survive = max_degree; // could make this zero if we want to nuke this node altogether. Or make it some decreasing function of the degree?
			V deleted_from_me = 0;
			V neighbours_left = current_degree;
			while(i != edges.end() && i->first == n) {
				assert(neighbours_left>0);
				N delete_this = i->second; // i will quickly become invalid
				//V deg = degree.get<NameTag>().find(delete_this)->second;
				V criteria = rand() % neighbours_left;
				if( (/*deg>7?criteria*criteria:*/criteria) >= to_survive) {
					// P("from %d delete", current_degree); cout << delete_this << endl;
					deleted_from_me++;
					edges.erase(i++);
					if(i!=edges.end() && i->first != n)
						i=edges.end(); // just in case the next line screws up the iterators
					edges.erase(make_pair(delete_this, n));
					degree.get<NameTag>().modify( degree.get<NameTag>().find(delete_this) , DegreeDecrementer<Name>);
					degree.get<NameTag>().modify( degree.get<NameTag>().find(n          ) , DegreeDecrementer<Name>);
				} else {
					to_survive--;
					i++;
				}
				--neighbours_left;
			}
			assert(i == edges.end() || i->first != n);
			DYINGWORDS(neighbours_left==0) {
				PP(neighbours_left);
				PP( (i==edges.end()) );
				PP( i->first );
			}
			assert(neighbours_left==0);
			//cout << '\"' << n << '\"' << '\t' << current_degree << '\t' << deleted_from_me << '\t' << current_degree - deleted_from_me << endl;
			// TODO: what if this leaves a node with degree zero in names?
		}
		PP(degree.get<DegreeTag>().begin()->second);
	}

		bg.edge_count = edges.size();

	}
	{
		ForeachContainer(const N &n, *names) {
			if(degree.get<NameTag>().find(n)->second>0)
				bg.vertex_mappings.push_back(n);
		}
		free(names); names=NULL; // free this asap to save a few scraps of RAM. All its data is in bg.vertex_mappings now.
		bg.vertex_count = bg.vertex_mappings.size();
	}
	populateNameToIDHash(bg); 
	{ 
		Timer timer("Finally populating the edges");
		sigUSR1_state.state = LOADING_EDGES;
		sigUSR1_state.edge_counter = 0;
		typename set <pair<N ,N> >::const_iterator edge = edges.begin();
		size_t offset = 0;
		for (V v = 0; v < bg.vcount(); v++) {
			N n = bg.name_of_one_node(v);
			int my_degree = 0;
			while(edge != edges.end() && edge->first == n) {
				sigUSR1_state.edge_counter++;
				my_degree++;
				bg.edge_targets.push_back(bg.key_for_vertexName(edge->second));
				edge++;
			}
			bg.degrees.push_back(my_degree);
			DYINGWORDS ( degree.get<NameTag>().find(n)->second == my_degree) {
				PP (       degree.get<NameTag>().find(n)->second);
				PP ( my_degree);
			}
			bg.offsets.push_back(offset);
			offset += my_degree;
		}
		assert(offset == bg.edge_targets.size());
		bg.offsets.push_back(offset);
	}
	assert(bg.edge_targets.size()==(size_t)bg.edge_count);

}
void loadBloomGraphMMAPFastButFussy(SimpleIntGraph &bg, const char *fileName) { 

	const char *fileBegin, *fileEnd;
	make_refpair(fileBegin, fileEnd) = mmapFile(fileName);
	{ // skip over option_ignoreNheaderlines;
		Pn("--ignoreNlines=%d", option_ignoreNlines);
		for(int i=0; i<option_ignoreNlines; i++) {
			const char *nextLine = 1+strchr(fileBegin, '\n');
			fileBegin = nextLine;
		}
	}
	const RangeOfEdges<SimpleIntGraph::Name> roe(fileBegin, fileEnd);

	findDistinctVertices(roe, bg);
	countDegrees(roe, bg);
	populateNameToIDHash(bg); 
	convertDegreesToOffsets(bg);
	loadEdges(roe, bg);


	//Pn("Sleeping for 5 seconds"); sleep(5);
	// To load vertices in, no edges yet. 10m nodes 7.5m edges. // 7767637 distinct, I think
	//  151M     9s
	// To load vertices in, no edges yet. 20m nodes 75m edges. // 19988969 distinct, I think
	// 1303M   100s
	// To load vertices in, no edges yet. 40m nodes 75m edges. // 39058948 distinct, I think
	// 1264M   125s // lower mem! Not really as surprising as it seems though.
	// To load vertices in, no edges yet. 50m nodes 350m edges. //  distinct, I think
	//         618s

	// To load vertices in, counting degree. 40m nodes 75m edges. // 39058948 distinct, I think
	//         140s 
	// To load vertices in, no edges yet. 50m nodes 350m edges. // 49999965 distinct, I think
	//         710

	// Edges loaded                        10m nodes   7.5m edges.
	//             17s
	// Edges loaded                        20m nodes  75m edges.
	// 100:120    262s
	// Edges loaded                        40m nodes  75m edges.
	// Edges loaded                        50m nodes 350m edges.

	// Hash                                20m nodes  75m edges.
	//            199s (+84 for edges)
	// Hash                                40m nodes  75m edges.
	//            236s (+93 for edges)
}

void findDistinctVertices(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg) {
	typedef unsigned int ID;
	vector< SimpleIntGraph::Name > & vertices = bg.vertex_mappings; // This is empty at the start. It'll be grown here.

#define BLOOM_SIZE 800000000 // 100MB should be useful, and not a waste of memory
	bitset<BLOOM_SIZE> *bloom = new bitset<BLOOM_SIZE>; // Just to help, quickly, remember if a vertex has already been found. 


	/* TODO
	 * Use a set, not extendin the vector, for the most recent additions.
	 * Store as binary file.
	 * Calculate a good value for k (the bloom filter) and use it.
	 */
		struct nested {
			static void addNewOrIncreaseDegree(vector<SimpleIntGraph::Name > &_vertices, const V _numSorted, const SimpleIntGraph::Name &addThis, bitset<BLOOM_SIZE> *_bloom) {
				if(_vertices.size() >= 2) { // OPTIMIZE BASED on idea that, in many data files, consecutive edges will mention similar edges
					typeof(_vertices.rbegin()) last = _vertices.rbegin();
					if(*last == addThis) { return; }
					last++;
					if(*last == addThis) { return; }
				}

				unsigned int h = hash_integer<unsigned int>(addThis, BLOOM_SIZE);
				unless(h<BLOOM_SIZE) {
					PP(BLOOM_SIZE);
					PP(h);
				}
				assert(h<BLOOM_SIZE);
				assert(h< _bloom->size());
				//PP(h);
				if(!_bloom->test(h)) { // definitely haven't seen this vertex before. Add it and get outta here.
					_bloom->set(h);
					_vertices.push_back(addThis);
					return;
				}

				typeof(_vertices.end()) splitHere = _vertices.begin()+_numSorted;
				typeof(_vertices.end()) lb = lower_bound(_vertices.begin(), splitHere, addThis);
				if(!binary_search(_vertices.begin(), splitHere, addThis))
					_vertices.push_back(addThis); // This is (probably) a new node. It's degree is 1.
			}
			static void mergeInTheRecentAdditions(vector<SimpleIntGraph::Name > &_vertices, V & _numSorted) {
				sort                            (_vertices.begin()+_numSorted, _vertices.end()); // Sort those that have recently been added.
				_vertices.erase(          unique(_vertices.begin()+_numSorted, _vertices.end()), _vertices.end()); // Remove the duplicates. among them
				inplace_merge(_vertices.begin(), _vertices.begin()+_numSorted, _vertices.end()); // We know we're merging non-overlapping sets, so we can finish now.
				_numSorted = _vertices.size();
			}
		};


	V numSorted = 0 ; // This many elements of vertices are known to be sorted.
	pair<SimpleIntGraph::Name,SimpleIntGraph::Name> edge;
	Foreach(edge , roe) {
		const SimpleIntGraph::Name & l = edge.first;
		const SimpleIntGraph::Name & r = edge.second;

		nested::addNewOrIncreaseDegree(vertices, numSorted, l, bloom);
		nested::addNewOrIncreaseDegree(vertices, numSorted, r, bloom);

		if(vertices.size()*0.8 - numSorted >= 10000000) // This could be tweaked. It shouldn't be too aggressive until we think we're running low on memory.
			nested::mergeInTheRecentAdditions(vertices, numSorted);
	}
	nested::mergeInTheRecentAdditions(vertices, numSorted);

	bg.vertex_count = vertices.size();

	delete bloom;
}

void countDegrees(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg) {
	bg.degrees.resize(bg.vertex_count);

	pair<SimpleIntGraph::Name,SimpleIntGraph::Name> edge;
	Foreach(edge , roe) {
		bg.degrees.at(bg.key_for_vertexName(edge.first))++;
		bg.degrees.at(bg.key_for_vertexName(edge.second))++;
	} 

}

void convertDegreesToOffsets(SimpleIntGraph &bg) {
	vector<SimpleIntGraph::Name > &vs = bg.vertex_mappings;
	bg.offsets.resize(1+bg.vcount());

	V total=0;

	typedef vector<SimpleIntGraph::Name >::iterator PairIterator;
	PairIterator endVertex = vs.begin()+bg.vcount();
	PairIterator i;
	unsigned int counter = 0;
	for (i = vs.begin(); i != endVertex; ++i) { // TODO: Move to my own range. clique.idiro.com doesn't appear to have up-to-date foreach.hpp. And I don't like the C++ Range; maybe should move to D after all.
		V degree = bg.degrees.at(counter);
		bg.offsets.at(counter) = total;
		total+=degree;
		++counter;
	}
	bg.offsets.at(counter) = total;

	bg.edge_count = total;
}

static void populateNameToIDHash(SimpleIntGraph &bg) {
	/* See also key_for_vertexName */
	assert(bg.hash_offsets.empty());
	bg.hash_offsets.resize(size_t(bg.vertex_count * 1.50), 0);
	for(V i=0; i<bg.vertex_count; i++) {
		unsigned int h = hash_integer<unsigned int>(bg.name_of_one_node(i), bg.hash_offsets.size());
		for(unsigned int j = 0; j<5 && h+j < bg.hash_offsets.size(); j++) { // TODO: put this code into Graph.hpp and have proper hashLookup and hashInsert functions.
			if(bg.hash_offsets.at(h+j)==0) {
				bg.hash_offsets.at(h+j) = i;
				break;
			} 
		}
	}
}
static void populateNameToIDHash(SimpleStringGraph &bg) {
	/* See also key_for_vertexName */
	assert(bg.hash_offsets.empty());
	bg.hash_offsets.resize(size_t(bg.vertex_count * 1.50), 0);
	for(V i=0; i<bg.vertex_count; i++) {
		unsigned int h = hash_integer<unsigned int>(bg.name_of_one_node(i), bg.hash_offsets.size());
		for(unsigned int j = 0; j<5 && h+j < bg.hash_offsets.size(); j++) { // TODO: put this code into Graph.hpp and have proper hashLookup and hashInsert functions.
			if(bg.hash_offsets.at(h+j)==0) {
				bg.hash_offsets.at(h+j) = i;
				break;
			} 
		}
	}
}


static void addNeighbour(VertexIDType edge_source, VertexIDType edge_target, VertexIDType _defaultVertexID, V &_edges_added, SimpleIntGraph &_bg) {
	// TODO: throw if not-simple graph

	if( /* if supposed to be simple */ edge_source == edge_target) {
		Die("Loop detected. Quitting. %d <> %d", _bg.name_of_one_node(edge_source), _bg.name_of_one_node(edge_target));
	}

	_edges_added++;
	/*
	if( flag_load_progress && ((_edges_added * 25) % _bg.ecount()) < 25) {
		Pn("edges %5.1f%% (%ld/%ld)" ,(_edges_added*50.0)/_bg.ecount() ,_edges_added, 2*_bg.ecount());
	}
	*/
	VertexIDType *_from, *_to;
	make_refpair(_from,_to) = _bg.neighbours(edge_source);
	VertexIDType *firstNonEmptySlot = upper_bound(_from, _to, _defaultVertexID);
	// Note, the neighbours aren't fully sorted, just the empty slots are to the left of the full slots. Therefore, the upper_bound should be OK.
	if( /* if supposed to be simple */ firstNonEmptySlot != _to) {
		if (*firstNonEmptySlot == edge_target)
			Die("That edge has already been entered. Only simple graphs are allowed in this version. Quitting. %d <> %d", _bg.name_of_one_node(edge_source), _bg.name_of_one_node(edge_target));
	}
	firstNonEmptySlot--;
	if(*firstNonEmptySlot != _defaultVertexID) Die ("already entered all of these neighbours. This is an unexpected bug");
	*firstNonEmptySlot = edge_target;
	if(_from==firstNonEmptySlot) sort(_from,_to);
}

void loadEdges(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg) {

	vector<VertexIDType > &edge_targets = bg.edge_targets;
	VertexIDType defaultVertexID = -1;
	edge_targets.resize(bg.edge_count, defaultVertexID);

	V edges_added = 0; // just so that we can keep counting for any progress bar we might like..
	pair<SimpleIntGraph::Name, SimpleIntGraph::Name> edge;
	Foreach(edge, roe) {
		SimpleIntGraph::ID lkey = bg.key_for_vertexName(edge.first);
		SimpleIntGraph::ID rkey = bg.key_for_vertexName(edge.second);
		addNeighbour(lkey, rkey, defaultVertexID, edges_added, bg);
		addNeighbour(rkey, lkey, defaultVertexID, edges_added, bg); // TODO: decide how to store loops ... Will I store the second entry? What'll that to do _edges_added?
	}
	// if(flag_load_progress) {
		// PP(edge_targets.size());
		// PP(edges_added);
	// }
}

pair<const char *, const char *> mmapFile(const char *fileName) {
	off_t fileSize;
	int graphFD = open(fileName, O_RDONLY);
	graphFD != -1 || Die("open(%s) failed", fileName);
	//PP(sizeof(char *)); // Must be 8 bytes (i.e. 64 bit) to load up the biggest files.
	{
		struct stat64 buf;
		fstat64(graphFD, &buf) == 0 || Die("fstat64 failed");
		fileSize=buf.st_size;
	}
	const void *map = mmap(NULL , (size_t) fileSize , PROT_READ , MAP_SHARED
//#ifdef PLATFORM_MAC
			//|MAP_NOCACHE // flags
//#endif
		, graphFD , 0 ); 
	map != (void*) -1 || Die("mmap failed");
	close(graphFD);
	const char * const fileBegin = (char*) map;
	const char * const fileEnd = fileBegin+fileSize; // I const the points also, so that I never accidentally change them.
	fileEnd[-1] == '\n' || Die("The file MUST be terminated by a newline");
	return make_pair(fileBegin, fileEnd);
}


const char *readEdge(const char *cur, int &l, int &r) throw (readEdgeInvalidLineInDataException) {
	while(*cur=='#' || *cur=='*') { cur = strchr(cur,'\n'); ++cur; } // This'll fail is the final lines in the file are like this. Gotta reorganize the file reading
		
	char *endptr;

	errno=0;
	l=strtol(cur, &endptr, 10);
	if(errno==EINVAL) { throw(readEdgeInvalidLineInDataException()); }

	while(!isdigit(*endptr)) endptr++; // advance to the start of the second name

	errno=0;
	r=strtol(endptr, &endptr, 10);
	if(errno==EINVAL) { throw(readEdgeInvalidLineInDataException()); }

	endptr = strchr(endptr, '\n'); // skip over to the start of the next line. This is why the file must be terminated by a newline ('\n')
	endptr++;
	return endptr;
}
const char *readEdge(const char *cur, string &l, string &r) throw (readEdgeInvalidLineInDataException) {
	l = r = "";
	while(*cur=='#' || *cur=='*') { cur = strchr(cur,'\n'); ++cur; } // This'll fail is the final lines in the file are like this. Gotta reorganize the file reading
		
	const char *endptr = cur;

	while(*endptr == '_' || *endptr == '-' || isalpha(*endptr) || isdigit(*endptr)) {
		endptr++;
	}
	if(endptr == cur) { throw(readEdgeInvalidLineInDataException()); }
	l.append(cur, endptr - cur);
	//Pn(":%s:\n", l.c_str());
	cur=endptr;

	while(*endptr != '\n' && (*endptr == ',' || *endptr == '|' || isspace(*endptr)))
		endptr++; // advance to the start of the second name
	if(endptr == cur) { throw(readEdgeInvalidLineInDataException()); }
	cur=endptr;

	while(*endptr == '_' || *endptr == '-' || isalpha(*endptr) || isdigit(*endptr)) {
		endptr++;
	}
	if(endptr == cur) { throw(readEdgeInvalidLineInDataException()); }
	r.append(cur, endptr - cur);
	cur=endptr;

	//Pn(":%s:\n", r.c_str());

	endptr = strchr(endptr, '\n'); // skip over to the start of the next line. This is why the file must be terminated by a newline ('\n')
	endptr++;
	return endptr;
}



} // namespace graph_loading
