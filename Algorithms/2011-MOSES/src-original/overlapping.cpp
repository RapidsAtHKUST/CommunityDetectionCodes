#include "overlapping.hpp"
/* TODO:
 *   grow seeds from edges, not nodes.
 *   Why are the results changing?
 *   	Then, speed up delV
 *   ZEntropy shouldn't consider the number of groups, that should be taken out to another function.
 *   That factorial expression would be better
 *   Write down some of the stats, and be sure they're correct.
 *   unordered_map for efficiency more often?
 *   a big vector for the comm_count_per_edge?
 *   keep track of frontier properly in growingSeed
 *   type tags for the multi_index_container, instead of get<1>
 *   random tie-breaking in frontier
 *   update _p_in also.
 *
 * PLAN:
 *   Fully abstract interface to grouping
 *   Track count of types of edges.
 *     Varying p_in and p_out
 *   Each edge to know how many communities it's in.
 *   Calculate global objective function every now and then.
 *   More efficient finding of seeds
 *   Random tie breaking
 *   Stop seed growing at first positive?
 */

/*
 * Sources of randomness:
 *   246: Choice of initial edge seed
 *   249: (arbitrary, not random) Randomized p_in 
 *   459: (arbitrary, not random) Tie breaker in seed expansion
 */

#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <math.h>
#include <string.h>
#include <fstream>
#include <sstream>
#include <float.h>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
//#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/indexed_by.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

#include "iterative.hpp"
#include "options.hpp"
#include "Range.hpp"
#include "grouping.hpp"
#include <set>

using namespace std;
using namespace std::tr1 ;
using namespace grouping;

char option_saveMOSESscores[1000] = "";
long option_seed = 0;
int flag_justCalcObjective = 0;

static void runMutual(void) {
	//system("echo;echo -n \" >> ;${x};${gt};${me};                                 \" ; bash -c '~/Code/Mutual3/mutual3/mutual \"${x}\"{\"${me}\",\"${gt}\"}' 1>&2 ");
}

namespace overlapping {

bool flag_save_group_id_in_output = true;
template <class N> static void overlappingT(bloomGraph<N> &g);
void overlapping(SimpleIntGraph &g) { overlappingT(g); }
void overlapping(SimpleStringGraph &g) { overlappingT(g); }
void addSeed(Grouping &ging, const set<V> &nodes, bool randomized_p_in);
static pair<long double, set<V> > growThisEdge(Grouping &ging, V edgeNumber, const long double &boost, bool randomized_p_in);
static void update_p_out(Grouping &ging);
static void estimate_p_in_and_p_out(Grouping &ging);
long double MOSES_objective(const Grouping &ging);

void addSeed(Grouping &ging, const set<V> &nodes, bool randomized_p_in) {
	assert(nodes.size()>0);
	Group *g = ging.newG(randomized_p_in);
	ForeachContainer(V v, nodes) {
		ging.addV(g, v);
	}
}


void seedGroupingWithPartition(Grouping &ging, const iterative::Partition &p);
template <class N> static void useOneNodeSeeds(Grouping &ging, bloomGraph<N> &g, bool randomize_p_in);
template <class N> static void groupStats(const Grouping &ging, bloomGraph<N> &g);
template <class N> static void save(Grouping &ging, bloomGraph<N> &g);
template <class N> static void louvainStyle(Grouping &ging, bloomGraph<N> &g);
static void tryMerges(Grouping &ging);
static void tryDeletions(Grouping &ging, bool SaveScores = false);
static bool tryAndApplyThisOne(Grouping &ging, V e, bool randomized_p_in);

template <class N> static void overlappingT(bloomGraph<N> &g) {
	printf("env: ALLOW_BOOST000 %s\n", getenv("ALLOW_BOOST000"));
	printf("env: Lookahead %s\n", getenv("Lookahead"));
	printf("env: MaxCommSize %s\n", getenv("MaxCommSize"));
	printf("env: OUTER_ITERS %s\n", getenv("OUTER_ITERS"));
	printf("env: LOUVAIN_ITERS %s\n", getenv("LOUVAIN_ITERS"));
	printf("NoBroken\n");
	srand48(option_seed); // default seed. Will be changeable by command line arg.
/*
	iterative::Partition p(g);
	iterative::findPartition(p, g);
	assert(option_planted[0]==0);
	strcpy(option_planted, option_overlapping);
	strcat(option_planted, "_partition");
	savePartition(g, p, -1);

	long double p_in = (long double) p.m_i / p.a;
	long double p_out;
	{
		int64 b = p.N * (p.N-1L) / 2L;
		int64 m_e = p.m-p.m_i;
		p_out = (long double) (m_e) / (b - p.a);
	}
	PP(p_in);
	PP(p_out);
	//Grouping ging(g, p_in, p_out);
	//seedGroupingWithPartition(ging, p);
*/

	Grouping ging(g, option_p_in, option_p_out);
	estimate_p_in_and_p_out(ging);
	ging.value_of_objective_with_no_communities = MOSES_objective(ging);
	//PP(ging._p_in);
	//PP(ging._p_out);

	if(option_loadOverlapping[0]) { // load  preexisting grouping
		Pn("Preloading grouping '%s'", option_loadOverlapping);
		ifstream inFile(option_loadOverlapping);
		//for (int i=0; i<5; ++i)
		int lineNo=0;
		while(inFile.peek()!=EOF)
		{
			lineNo ++;
			string line;
			getline(inFile, line);
			if(line.length()==0) break;
			Group *grp = ging.newG();
			istringstream linestream(line);
			//PP(line);// 
			if(linestream.peek()=='"') {
				char ch;
				linestream >> ch;
				while(linestream.peek() != EOF) {
					linestream >> ch;
					if(ch=='"')
						break;
				}
				if(linestream.peek()!='\t') Die("grouping file should have a tab after any \" \". line %d {%d '%c'}", lineNo, (int) ch, ch);
			}
			while(linestream >> ws, linestream.peek() != EOF) {
				N n;
				linestream >> n;
				V v = g.key_for_vertexName(n);
				assert(v>=0);
				assert(v<g.vcount());
				ging.addV(grp, v);
			}
		}
		save(ging, g);
	}

	estimate_p_in_and_p_out(ging);
	groupStats(ging, g);
	MOSES_objective(ging);
	if(flag_justCalcObjective)
		exit(0);
	

	const int max_OuterIters = atoi(getenv("OUTER_ITERS") ? : "20");
	PP(max_OuterIters);
	for (int k=0; k<max_OuterIters; k++) {
		MOSES_objective(ging);
		const size_t num_groups_before = ging.groups.size();
		ostringstream s;
		s << "Whole outer iter " << k << "/" << max_OuterIters;
		Timer timer(s.str());
		Pn("\ngrow seeds %d/%d", k, max_OuterIters);
		bool randomize_p_in;
		if(k < max_OuterIters / 2) {
			randomize_p_in = true;
			Pn("Random p_i for each edge that we try");
			estimate_p_in_and_p_out(ging); // this is just to estimate p_out
			useOneNodeSeeds(ging, g, true);
			estimate_p_in_and_p_out(ging);
			tryDeletions(ging);
		} else {
			randomize_p_in = false;
			useOneNodeSeeds(ging, g, false);
			tryDeletions(ging);
		}
		groupStats(ging, g);
		save(ging, g);
		estimate_p_in_and_p_out(ging);
		//tryDeletions(ging);
		//save(ging, g);
		const size_t num_groups_after = ging.groups.size();
		if(!randomize_p_in && /*num_groups_after > 1000 && */ 0.99L * num_groups_after <= num_groups_before) {
			Pn("breaking after %d growing passes, as VERY FEW more have been found among the most recent subset", k+1);
			break;
		}
	}

		int louvainStyle_iter=0;
		const int max_louvainStyleIters = atoi(getenv("LOUVAIN_ITERS") ? : "10");
		PP(max_louvainStyleIters);
		while(louvainStyle_iter != max_louvainStyleIters) {
			MOSES_objective(ging);
			Timer timer("Whole Louvain iter");
			Pn("\nLouvain-style iteration %d/%d", louvainStyle_iter++, max_louvainStyleIters);
			louvainStyle(ging, g);
			tryDeletions(ging, louvainStyle_iter==max_louvainStyleIters);
			estimate_p_in_and_p_out(ging);
			groupStats(ging, g);
			save(ging, g);
		}

		if(0) {
			tryMerges(ging);
			save(ging, g);
			groupStats(ging, g);
		}

	Pn("\n\nFINAL Grouping");
	groupStats(ging, g);
	estimate_p_in_and_p_out(ging);
	MOSES_objective(ging);
}
static bool tryAndApplyThisOne(Grouping &ging, V e, bool randomized_p_in) {
			static long double boost = 0.0L;
			if(!getenv("ALLOW_BOOST000"))
				boost=0.0L; // reset it back to zero unless ALLOW_BOOST000 is defined in the environment
			pair<long double, set<V> > bestSeed = growThisEdge(ging, e, boost, randomized_p_in);
			if(bestSeed.first + boost> 0.0L && bestSeed.second.size()>0 ) {
				addSeed(ging, bestSeed.second, randomized_p_in);
				boost /= 2.0; // if(boost<0.0L) boost=0.0L;
				return true;
				//Pn("Applied best 1-seed. Now returning. %zd nodes (+%Lg). Now there are %zd communities", bestSeed.second.size(), bestSeed.first, ging.groups.size());
				// if(bestSeed.second.size() < 20) ForeachContainer (V v, bestSeed.second) { cout << "|" << g.name(v); } P("\n");
			}
			boost += 0.1;
			return false;
}
template <class N> static void useOneNodeSeeds(Grouping &ging, bloomGraph<N> &g, bool randomize_p_in) {
	Timer timer(__FUNCTION__);
	const int numTries = g.ecount()/5;
	P("  \n Now just use one-EDGE seeds at a time. Try %d edges\n", numTries);
	// groupStats(ging, g);
	for(int x=0; x<numTries; ++x) {
		if(x && numTries>5 && x%(numTries/5)==0) {
			PP(x);
			PP(ging.groups.size());
		}
		// choose an edge at random, but prefer to use it iff it's sharedCommunities score is low.
		V e = V(drand48() * (2*ging._g.ecount()));
		assert(e >= 0);
		assert(e < 2*ging._g.ecount());
		if(randomize_p_in) {
			ging._p_in = 0.01L + 0.98L*drand48();
			assert(ging._p_in < 1.0L);
		}
		tryAndApplyThisOne(ging, e, randomize_p_in);
#if 0
		int sharedCommunities = ging.comm_count_per_edge(ging._g.neighbours(0).first[e], &(ging._g.neighbours(0).first[e])); // a little time in here
		//PP(sharedCommunities);
		if(  (double(rand())/RAND_MAX)
				<  powl(0.5, sharedCommunities))
		{
			//Pn(" X %d", sharedCommunities);
			tryAndApplyThisOne(ging, e);
		} else {
			//Pn("   %d", sharedCommunities);
		}
#endif
	}
}
template <class N> static void groupStats(const Grouping &ging, bloomGraph<N> &g) {
	map<size_t, int> group_sizes_of_the_randomized;
	map<size_t, int> group_sizes;
	int64 totalAssignments = 0; // to help calculate average communities per node.
	ForeachContainer(Group *group, ging.groups) {
		DYINGWORDS(group->vs.size()>0) {
			PP(group->vs.size());
		}
		group_sizes[group->vs.size()]++;
		totalAssignments += group->vs.size();
		if(group->_randomized_p_in)
			group_sizes_of_the_randomized[group->vs.size()]++;
	}

	//Perror("    %zd\n", ging.groups.size());
	Pn("#groups=%zd. %zd nodes, out of %d, are in at least one community. avgs grps/node=%g", ging.groups.size(), ging.vgroups_size(), g.vcount(), (double) totalAssignments / g.vcount());

	pair<size_t, int> group_size;
	size_t max_group_size = group_sizes.size()==0 ? 0 : group_sizes.rbegin()->first;
	int entries_per_row = 15;
	int number_of_rows = (max_group_size / entries_per_row) + 1;
	for(int r = 0; r<number_of_rows ; r++) {
		for(size_t c = r; c <= max_group_size; c+=number_of_rows) {
			if(group_sizes[c]>0)
				P("%6d{%3zd}", group_sizes[c], c);
			else
				P("           ");
		}
		P("\n");
	}
{ // now, just the randomized ones
	size_t max_group_size = group_sizes_of_the_randomized.size()==0 ? 0 : group_sizes_of_the_randomized.rbegin()->first;
	int entries_per_row = 15;
	int number_of_rows = (max_group_size / entries_per_row) + 1;
	for(int r = 0; r<number_of_rows ; r++) {
		for(size_t c = r; c <= max_group_size; c+=number_of_rows) {
			if(group_sizes_of_the_randomized[c]>0)
				P("%6d{%3zd}", group_sizes_of_the_randomized[c], c);
			else
				P("           ");
		}
		P("\n");
	}
}
#if 0
	set<V> lonelyNodesInLargestGroup;
	Group *largestGroup = NULL;
	ForeachContainer(Group *group, ging.groups) {
		if(group->vs.size() == max_group_size) {
			ForeachContainer(V v, group->vs) {
				P("(%zd)", ging.vgroups(v).size());
				if(1 == ging.vgroups(v).size() && 0 == rand()%2)
					lonelyNodesInLargestGroup.insert(v);
			}
			P("\n");
			largestGroup = group;
			break;
		}
	}
#endif

	{
		int print_count = 0;
		for(map<int, V>::const_iterator it = ging.global_edge_counts.begin(); it!=ging.global_edge_counts.end(); ++it) {
			P("%6d(%3d)", (int) it->second ,(int) it->first);
			print_count++;
			if(print_count%15==0)
				P("\n");
		}
		P("\n");
		//if(0) update_p_out(ging);
		//PP(ging._p_in);
		//PP(ging._p_out);
	}
}

void seedGroupingWithPartition(Grouping &ging, const iterative::Partition &p) {
	for(V c=0; c<p.g.vcount(); ++c) {
		if(p.p[c].c == c) { // we're at the head node
			//Pn("  order %d", p.p[c].order);
			Group *grp = ging.newG();
			V v = c;
			do {
				assert(c == p.p[v].c);
				ging.addV(grp, v);
				//Pn ("%d is in %d", v ,c);
				v = p.p[v].next;
			} while (v != c);
		}
	}
}

template <class It>
static size_t count_intersection(It it1b, It it1e, It it2b, It it2e) {
	vector< typename It::value_type > is;
	set_intersection(it1b, it1e, it2b, it2e, back_inserter(is));
	return is.size();
}
template <class Container>
static size_t count_intersection(const Container &container1, const Container &container2) {
	return count_intersection(
		container1.begin()
		, container1.end()
		, container2.begin()
		, container2.end()
	);
}

struct DeltaSeed {
	const V _v; // for this node
	const int _group_size_smaller; // ...and this group (which v is NOT currently in)
	const Grouping &_ging;
		// what'd be the change in entropy (edge+Z+Q) from joining it?
	long double _deltadeltaEdgeEntropy;
	int count_edges_back_into_this_group;
	long double deltadeltaPairEntropy() const {
		return _deltadeltaEdgeEntropy + log2l(1.0L - _ging._p_in) * (_group_size_smaller - count_edges_back_into_this_group);
	}
	explicit DeltaSeed(V v, int group_size_smaller, Grouping &ging) : _v(v), _group_size_smaller(group_size_smaller), _ging(ging), _deltadeltaEdgeEntropy(0.0L), count_edges_back_into_this_group(0) {
		//assert(_grp.vs.count(v)==0);
	}
	void addEdge2(V n, const V* edgeVN_ptr) { // n is connected to _v 
		assert(*edgeVN_ptr == n);
		this->addEdge(n, _ging.comm_count_per_edge(n, edgeVN_ptr));
	}
	void addEdge(V  , int sharedCommunities) { // n is connected to _v // TODO: might be quicker to pass in the count of sharedCommunities too
		//assert(_ging._g.are_connected(_v, n)); // TODO: remove these assertions
		//assert(_grp.vs.count(n) == 1);
		//assert(_grp.vs.count(_v) == 0);
		count_edges_back_into_this_group ++;
		_deltadeltaEdgeEntropy += log2l(1.0L - (1.0L-_ging._p_out)*powl(1.0L - _ging._p_in, 1+sharedCommunities))
		                        - log2l(1.0L - (1.0L-_ging._p_out)*powl(1.0L - _ging._p_in, sharedCommunities));
	}
	void redoEdge(V  , int previous_sharedCommunities) { // n is connected to _v // TODO: might be quicker to pass in the count of sharedCommunities too
		_deltadeltaEdgeEntropy -=(log2l(1.0L - (1.0L-_ging._p_out)*powl(1.0L - _ging._p_in, 1+previous_sharedCommunities))
		                        - log2l(1.0L - (1.0L-_ging._p_out)*powl(1.0L - _ging._p_in,   previous_sharedCommunities)));
		_deltadeltaEdgeEntropy += log2l(1.0L - (1.0L-_ging._p_out)*powl(1.0L - _ging._p_in, 2+previous_sharedCommunities))
		                        - log2l(1.0L - (1.0L-_ging._p_out)*powl(1.0L - _ging._p_in, 1+previous_sharedCommunities));
	}
	long double _deltaZentropy() const {
		const size_t N = _ging._g.vcount();
		const size_t x = _group_size_smaller;
		const size_t x2 = 1+x;

			return( x2 * log2l(x2)             + (N-x2)  *  log2l(N-x2)
		          -x *  log2l(x)             - (N-x )  *  log2l(N-x )             );
/*
		 (x2 * (log2l(x2)-log2l(N))  + (N-x2)  * (log2l(N-x2)-log2l(N))  + log2l(1+ging.groups.size()) - log2l(N))
		-(x  * (log2l(x) -log2l(N))  + (N-x )  * (log2l(N-x )-log2l(N))  + log2l(1+ging.groups.size()) - log2l(N))
	=  (x2 * (log2l(x2)-log2l(N))  + (N-x2)  * (log2l(N-x2)-log2l(N))  )
		-(x  * (log2l(x) -log2l(N))  + (N-x )  * (log2l(N-x )-log2l(N))  )
	=  (x2 * (log2l(x2)         )  + (N-x2)  * (log2l(N-x2)         )  )
		-(x  * (log2l(x)          )  + (N-x )  * (log2l(N-x )         )  )
	=   x2 *  log2l(x2)            + (N-x2)  *  log2l(N-x2)
		  -x *  log2l(x)             - (N-x )  *  log2l(N-x )
*/

	}
	long double _deltaTotalentropy() const {
		return this->deltadeltaPairEntropy() + this->_deltaZentropy();
	}
};

struct FrontierNode {
	FrontierNode(long double &score, V v) : _score(score), _v(v) {}
	long double _score;
	V _v;
	struct Incrementer {
		long double _x;
		Incrementer(long double &x) : _x(x) {}
		void operator() (FrontierNode &fn) const { fn._score += _x; }
	};
};

using namespace boost::multi_index;
struct VertexTag {};
struct Frontier : private multi_index_container < // TODO: Some sort of binary tree sometime?
		FrontierNode,
		indexed_by<
			ordered_non_unique< member<FrontierNode,long double,&FrontierNode::_score>, greater<long double> >,
			hashed_unique< tag<VertexTag>, member<FrontierNode,V,&FrontierNode::_v> >
		>
	> {
	// vertices, and their scores.
	// easy removal of the highest-score vertices.
  // easy increase of score of arbitrary members, adding them if they don't exist already.
public:
	static long double __attribute__ ((noinline)) calcddEE(const Grouping &ging, int sharedCommunities) {
		return                   log2l(1.0L - (1.0L-ging._p_out) * powl(1.0L - ging._p_in, 1+sharedCommunities))
		                      -  log2l(1.0L - (1.0L-ging._p_out) * powl(1.0L - ging._p_in, sharedCommunities))
		                      -  log2l(1.0L - ging._p_in)
													+  1e-20L * drand48() // random tie breaking
													;
	}
	void addNode(const Grouping &ging, V to, const V *edgeFT_ptr) {
		// to is being added to the frontier, BUT it may already be in the frontier.
		// from is in the seed.
		//assert(*edgeFT_ptr == to);
		int sharedCommunities = ging.comm_count_per_edge(to, edgeFT_ptr); // a little time in here
		long double deltadeltaEdgeEntropy = calcddEE(ging, sharedCommunities /*, to*/); // a little time in here

		Frontier::nth_index<1>::type::iterator addOrModifyThis = this->get<1>().find(to);
		if(addOrModifyThis==this->get<1>().end()) { // TODO: faster if search for to is done just once?
			this->insert(FrontierNode(deltadeltaEdgeEntropy, to));
		} else {
			this->get<1>().modify(addOrModifyThis, FrontierNode::Incrementer(deltadeltaEdgeEntropy));
		}
	}
	void erase_best_node() {
		Frontier::iterator best_node = this->get<0>().begin();
		this->erase(best_node);
	}
	int erase_this_node(V to) {
		return this->get<1>().erase(to);
	}
	long double best_node_score() const {
		Frontier::iterator best_node = this->get<0>().begin();
		return best_node -> _score;
	}
	V best_node_v() const {
		Frontier::iterator best_node = this->get<0>().begin();
		return best_node -> _v;
	}
	bool Empty() const {
		return this->empty();
	}
};

static long double logNchoose(int64 N, int64 n_c) {
	if(n_c==N)
		return 0;
	static vector<long double> logNchoose_vector;
	// static int64 usedN;
	assert(n_c>0);
	assert(n_c<=N);
	if (logNchoose_vector.size()==0) {
		Timer t("logNchoose Initialization");
		logNchoose_vector.resize(N+1);
		// usedN = N;

		long double lN = 0.0L;
		for(int64 x1=1; x1<=N; x1++) {
			if(x1>1) {
				int64 i = x1-1;
				lN += log2l(i) - log2l(N-i);
			}
			logNchoose_vector.at(x1) = lN;
		}
	}
	assert(logNchoose_vector.at(0) == 0);
	// DYINGWORDS(logNchoose_vector.at(N) == 0) { // will never be exactly zero, but it should be, in theory
	assert(logNchoose_vector.size()>0);
	// assert(usedN == N);
	assert( size_t(N+1) == logNchoose_vector.size());
	assert( size_t(n_c) < logNchoose_vector.size());
	return logNchoose_vector.at(n_c);
}

pair<long double, set<V> > growingSeed(Grouping &ging, int lookahead
		, set<V> &seed
		, pair<long double, set<V> > bestSoFar
		, long double seedEdgeEntropy
		, int seed_size
		, Frontier &frontier
		, int edges_in_seed
		, const long double &boost // to allow some negative communities to persist. The deletion phase will fix them later. This is to ensure that we have the best chance of filling the graph up quickly.
		, bool randomized_p_in
	)
// Find the expansion among the frontier that best improves the score. Then recurse to it.
// Stop growing if dead end is reached (empty frontier) or the seed isn't increasing and we already have at least 5 nodes.
// Return the best set of nodes
// TODO: Profile, then make this damn efficient
{
		assert((int) seed.size() == seed_size);
		if(frontier.Empty())
			return bestSoFar;
		const V best_v = frontier.best_node_v();
#if 0
		{
			IteratorRange<const V *> ns(ging._g.neighbours(best_v));
			Foreach(V n, ns) {
				if(1==seed.count(n)) { // This neighbour is already in the seed. That means the count of edges within the seed is about to be increased. Should update randomized p_in in light of this.
					++edges_in_seed;
				}
			}
			int64 pairsInSeed = seed_size * (1+seed_size) / 2;
			long double new_p_in = 1.0L * (2*edges_in_seed+1) / (2 * pairsInSeed + 2) ;
			if(randomized_p_in) ging._p_in = new_p_in;

/*
			cout << edges_in_seed << "/" << (1+seed_size)
				<< "\t" << 100.0 * edges_in_seed / (seed_size * (1+seed_size) / 2)
				// << "\t" << ging._p_in << "\t" << randomized_p_in
				<< "\t" << new_p_in
				<< endl; 
*/
			assert(edges_in_seed <= (seed_size * (1+seed_size) / 2)); 
		}
#endif
		const long double newseedEdgeEntropy = seedEdgeEntropy + frontier.best_node_score() + log2l(1.0L-ging._p_in) * seed_size;

		const int N = ging._g.vcount();
		//const int x = seed_size;
		const int x1= 1+seed_size;
		//long double seed_totalDeltaEntropy    = seedEdgeEntropy    + x  * (log2l(x)-log2l(N))  + (N-x)  * (log2l(N-x)-log2l(N))  + log2l(1+ging.groups.size())/*equivalent groupings*/ - log2l(N)/*encoding of size of the group*/;
		UNUSED       int64 q = ging.groups.size(); if (q==0) q=1;
		UNUSED const int64 q_= 1 + q;
		const long double logNchoosen = logNchoose(N,x1);
		const long double newseed_totalDeltaEntropy = newseedEdgeEntropy
				//+ x1 * (log2l(x1)-log2l(N)) + (N-x1) * (log2l(N-x1)-log2l(N))
				+ logNchoosen
				- log2l(N+1)/*encoding of size of the group*/
				+ log2l(1+ging.groups.size())/*equivalent groupings*/
				//+ (getenv("UseBroken") ? ( (  q_ * (log2l(q_) - log2l(exp(1))) + log2l(N+1) - log2l(N+1-q_)  ) - (  q  * (log2l(q ) - log2l(exp(1))) + log2l(N+1) - log2l(N+1-q )  ) ) : 0)
			;

		if(bestSoFar.first < newseed_totalDeltaEntropy) {
			bestSoFar.first = newseed_totalDeltaEntropy;
			bestSoFar.second = seed;
			bestSoFar.second.insert(best_v);
		}

		if( (size_t) seed_size >= lookahead +bestSoFar.second.size() ) // lookahead
			return bestSoFar;
		const size_t max_commsize = atoi(getenv("MaxCommSize") ? : "10000000");
		if( (size_t) seed_size >= max_commsize ) { // max comm size
			bestSoFar.first = -10000000; return bestSoFar;
		}
		//if( bestSoFar.first > 0.0L) // once positive, return immediately!!!!!!!!!!!!!
			//return bestSoFar;
		if( (size_t) seed_size > bestSoFar.second.size() && bestSoFar.first + boost > 0.0L) // once positive, return immediately if it drops.
			return bestSoFar;


		//if(bestSoFar.first > 0.0L)
			//return bestSoFar; // This isn't working so well. Too many communities (I think), and no faster (for Oklahoma at least)

		frontier.erase_best_node();
		IteratorRange<const V *> ns(ging._g.neighbours(best_v));
		const V *edgeVN_offset = ging._g.neighbours(best_v).first;
		Foreach(V n, ns) {
			assert( *edgeVN_offset == n);
			//assert(ging._g.neighbours(0).first[ging._comm_count_per_edge2[edgeVN_offset].other_index] == best_v);
			if(0==seed.count(n))
				frontier.addNode(ging, n
					, edgeVN_offset
					);
			++edgeVN_offset;
		}
		seed.insert(best_v);
		return growingSeed(ging
			, lookahead
			, seed
			, bestSoFar
			, newseedEdgeEntropy 
			, 1 + seed_size
			, frontier
			, edges_in_seed
			, boost
			, randomized_p_in
		);
}

struct ThrowingIterator {
	struct Dereferenced {};
	V & operator *() { throw Dereferenced(); }
	void operator ++() { throw Dereferenced(); }
};
bool emptyIntersection(const pair<const V*, const V*> &l, const pair<const V*, const V*> &r) {
		try {
			set_intersection(
			 	l.first
				,l.second
				,r.first
				,r.second
				,ThrowingIterator()
			);
		} catch (ThrowingIterator::Dereferenced &) {
			return false;
		}
		return true; //inter.empty();
} 

static pair<long double, set<V> > growThisEdge(Grouping &ging, const V edgeNumber, const long double &boost, bool randomized_p_in) {
	assert(edgeNumber < 2*ging._g.ecount());
	V r = ging._g.neighbours(0).first[edgeNumber];
	V l = ging._g.neighbours(0).first[ging.comm_count_per_edge2[edgeNumber].other_index];

	// there must be a triangle available
	if(emptyIntersection(ging._g.neighbours(l),ging._g.neighbours(r)))
		return make_pair(-1.0L, set<V>());

	Frontier frontier;
	{
		IteratorRange<const V *> ns(ging._g.neighbours(l));
		const V *edgeIN_ptr = ging._g.neighbours(l).first;
		Foreach(V n, ns) {
			frontier.addNode(ging, n, edgeIN_ptr);
			++edgeIN_ptr;
		}
	}
	//PP(frontier.size());
	const int erased = frontier.erase_this_node(r);
	DYINGWORDS(erased==1) {
		PP(erased);
		PP(l);
		PP(r);
		//PP(frontier.size());
		PP(ging._g.degree(l));
		PP(ging._g.degree(r));
		IteratorRange<const V *> ns(ging._g.neighbours(l));
		Foreach(V n, ns) { PP(n); }
	}
	assert(erased==1);
	{
		IteratorRange<const V *> ns(ging._g.neighbours(r));
		const V *edgeIN_ptr = ging._g.neighbours(r).first;
		Foreach(V n, ns) {
			if(n!=l)
				frontier.addNode(ging, n, edgeIN_ptr);
			++edgeIN_ptr;
		}
	}

	set<V> seed;
	seed.insert(l);
	seed.insert(r);

	int sharedCommunities = ging.comm_count_per_edge(r, &(ging._g.neighbours(0).first[edgeNumber])); // a little time in here

	pair<long double, set<V> > grownSeed = growingSeed(ging
			, atoi(getenv("Lookahead") ? : "2")
			, seed
			, make_pair(-10000000.0L, seed) // This score is too low, but then we don't expect a singleton community to come out best anyway! Only positive scores are used.
			,   log2l(1.0L - (1.0L-ging._p_out) * powl(1.0L - ging._p_in, 1+sharedCommunities))
		    - log2l(1.0L - (1.0L-ging._p_out) * powl(1.0L - ging._p_in, sharedCommunities))
			, 2
			, frontier
			, 1
			, boost
			, randomized_p_in
		);
	return grownSeed;
}

template <class N> static void save(Grouping &ging, bloomGraph<N> &g) {
	ofstream saveFile(option_overlapping);
	ForeachContainer(Group *grp, ging.groups) {
		bool firstLine = true;
		if(flag_save_group_id_in_output)
			saveFile << '\"' << grp->_id << "\"\t";
		ForeachContainer(V v, grp->vs) {
			saveFile << (firstLine?"":" ") << g.name(v);
			firstLine = false;
		}
		saveFile << endl;
	}
	runMutual();
}

struct hashGroup {
	int operator() (Group *grp) const {
		return grp->_id;
	}
};
typedef boost::unordered_map<Group *, DeltaSeed, hashGroup> SeedDeltasT;

template <class N> static void louvainStyle(Grouping &ging, bloomGraph<N> &g) {
	Timer t(__FUNCTION__);
	// find a node, isolate it from its communities, Add back one at a time if appropriate.
	const bool DEBUG_louvainStyle = 0;
	for(V v=0; v<g.vcount(); v++) {
		if(0) update_p_out(ging);
		// if(v%(g.vcount()/20)==0) PP(v);
if(DEBUG_louvainStyle)
		groupStats(ging, g);
if(DEBUG_louvainStyle)
		cout << "removing node in these many groups: " << ging.vgroups(v).size() << endl;

		ging.isolate(v);

		SeedDeltasT _seedDeltas;
		IteratorRange<const V *> ns(ging._g.neighbours(v));
		{
			const V * edgeVN_ptr = ging._g.neighbours(v).first;
			Foreach(V n, ns) {
				int sharedCommunities = ging.comm_count_per_edge(n,edgeVN_ptr);
				// TODO: Could prepare the results for addEdge of v<>n
				ForeachContainer(Group *grp, ging.vgroups(n)) {
					_seedDeltas.insert(make_pair(grp, DeltaSeed(v, grp->vs.size(), ging))).first->second.addEdge(n, sharedCommunities);
					//_seedDeltas.find(grp)->second.addEdge(n, sharedCommunities);
				}
				++edgeVN_ptr;
			}
		}

		for(int addedBack = 0; _seedDeltas.size()>0 ; addedBack++) {
	
			// for each neighbouring group, calculate the delta-entropy of expanding back in here.
			pair<long double, Group *> bestGroup(-LDBL_MAX, (Group*) NULL);
			int num_positive = 0;
			for(SeedDeltasT::iterator i = _seedDeltas.begin(); i!=_seedDeltas.end(); ) {
				if(i->second._deltaTotalentropy()<=0.0L) {
					i = _seedDeltas.erase(i);
					continue;
				} else {
					long double delta2 = i->second._deltaTotalentropy();
					// TODO: Count the positive scores. No point proceeding if there aren't any more positive scores, as they can only decrease
					if(bestGroup.first < delta2)
						bestGroup = make_pair(delta2, i->first);
					if(delta2>0.0L)
						++num_positive;
				}
				++i;
			}
			if(bestGroup.first > 0.0L) {
				assert(num_positive>=1);
				ging.addV(bestGroup.second, v);
				if(num_positive==1) { // if just one was positive, then there's no point continuing, as the rest will only lose more score.
					break;
				}

				_seedDeltas.erase(bestGroup.second);

				// the other potential groups on the end of this edge need to have their addEdge undone
				const V * edgeVN_ptr = ging._g.neighbours(v).first;
				IteratorRange<const V*> ns(ging._g.neighbours(v));
				Foreach(V n, ns) {
					assert(*edgeVN_ptr == n);
					if(bestGroup.second->vs.count(n)) {
						int previous_sharedCommunities = ging.comm_count_per_edge(n, edgeVN_ptr) - 1;
						ForeachContainer(Group *grp, ging.vgroups(n)) {
							SeedDeltasT::iterator grpInSeed =_seedDeltas.find(grp);
							if(grpInSeed != _seedDeltas.end())
							{
								const long double before = grpInSeed->second._deltaTotalentropy();
								grpInSeed->second.redoEdge(n, previous_sharedCommunities);
								const long double after = grpInSeed->second._deltaTotalentropy();
								if(after > before) {
									Perror("%s:%d _deltaTotalentropy %Lg -> %Lg\n", __FILE__, __LINE__, before, after);
								}
							}
						}
					}
					++edgeVN_ptr;
				}
			}
			else
				break;
		}
	}
}

static void update_p_out(Grouping &ging) {
		long double new_p_out = 2.0L * ging.global_edge_counts[0] / ging._g.vcount() / (ging._g.vcount()-1) ;
		if(new_p_out > 0.1L)
			ging._p_out = 0.1L;
		else
			ging._p_out = new_p_out;
}

struct PairGroupHash {
	int operator() (const pair<Group*,Group*> &pg) const {
		return pg.first->_id + pg.second->_id;
	}
};
static void tryMerges(Grouping &ging) {
	Timer timer(__FUNCTION__);
	typedef boost::unordered_map< pair<Group*,Group*> , long double, PairGroupHash> MergesT;
	MergesT proposed_merges;
	size_t counter=0;
	for(V e = 0; e <
			/*100000 */
			2*ging._g.ecount()
		; e++) {
		//if(e % (2*ging._g.ecount()/10) ==0) { PP(e); }
		V e2 = ging.comm_count_per_edge2[e].other_index;
		V l = ging._g.neighbours(0).first[e2];
		V r = ging._g.neighbours(0).first[e];
		if(r<l) continue; // no point considering each edge twice
		V sharedCommunities = ging.comm_count_per_edge2[e].shared_comms;
		assert(sharedCommunities == ging.comm_count_per_edge2[e2].shared_comms);
		//Pn("%d\t%d", l, r);
		counter++;
		const set<Group *> &lgrps = ging.vgroups(l);
		const set<Group *> &rgrps = ging.vgroups(r);
		//PP(lgrps.size());
		//PP(rgrps.size());
		vector<Group *> lonly, ronly;
		set_difference(lgrps.begin(),lgrps.end(),rgrps.begin(),rgrps.end(),back_inserter(lonly));
		set_difference(rgrps.begin(),rgrps.end(),lgrps.begin(),lgrps.end(),back_inserter(ronly));
		//Pn("# unmatched %zd,%zd", lonly.size(), ronly.size());
		ForeachContainer(Group *lg, lonly) {
			ForeachContainer(Group *rg, ronly) {
				long double qi = 1.0L - ging._p_in;
				long double qo = 1.0L - ging._p_out;
				//const int64 oldQz = ging.groups.size();
				Group * lg1 = lg;
				Group * rg1 = rg;
				if(lg1 > rg1) swap(lg1, rg1);
				MergesT::key_type key = make_pair(lg1, rg1);
				MergesT::iterator pm = proposed_merges.find(key);
				if(pm == proposed_merges.end()) {
					const int64 s1 = lg1->vs.size();
					const int64 s2 = rg1->vs.size();
					vector<V> Union;
					set_union(lg1->vs.begin(),lg1->vs.end(),rg1->vs.begin(),rg1->vs.end(),back_inserter(Union));
					const int64 N = ging._g.vcount();
					pm = proposed_merges.insert(make_pair(key,
						log2l(qi)*0.5L* (long double)(    Union.size() * (Union.size()-1) - s1*(s1-1) - s2*(s2-1) )
							 // + (  (oldQz)  *log2l(oldQz-1) + log2l(exp(1)) - log2l(oldQz-2-N)       )
							 // - (  (oldQz+1)*log2l(oldQz  )                 - log2l(oldQz-1-N)       )
						                 + (s1+s2) * (log2l(s1+s2) /*- log2l(N)*/)
						                 - (s1   ) * (log2l(s1   ) /*- log2l(N)*/)
						                 - (s2   ) * (log2l(s2   ) /*- log2l(N)*/)
						                 + log2l(N) // one fewer community whose pi has to be encoded
						)).first;
				}
				pm -> second+=
		                         log2l(1.0L - qo * powl(qi, 1+sharedCommunities))
		                      -  log2l(1.0L - qo * powl(qi, sharedCommunities))
		                      -  log2l(qi) // for a given member of the map, each edge will be found exactly once. So here we cancel the affect of assuming it was disconnected
		                     ;
			}
		}

	}
	for(V e = 0; e <
			/*100000 */
			2*ging._g.ecount()
		; e++) {
		if(e % (2*ging._g.ecount()/10) ==0) {
			PP(e);
		}
		V e2 = ging.comm_count_per_edge2[e].other_index;
		V l = ging._g.neighbours(0).first[e2];
		V r = ging._g.neighbours(0).first[e];
		if(r<l) continue; // no point considering each edge twice
		V sharedCommunities = ging.comm_count_per_edge2[e].shared_comms;
		assert(sharedCommunities == ging.comm_count_per_edge2[e2].shared_comms);
		//Pn("%d\t%d", l, r);
		counter++;
		const set<Group *> &lgrps = ging.vgroups(l);
		const set<Group *> &rgrps = ging.vgroups(r);
		//PP(lgrps.size());
		//PP(rgrps.size());
		vector<Group *> inter;
		set_intersection(lgrps.begin(),lgrps.end(),rgrps.begin(),rgrps.end(),back_inserter(inter));
		//Pn("# unmatched %zd,%zd", lonly.size(), ronly.size());
		ForeachContainer(Group *lg, inter) {
			ForeachContainer(Group *rg, inter) {
				if(lg < rg) { // no point proposing a merge between a group and itself
				long double qi = 1.0L - ging._p_in;
				long double qo = 1.0L - ging._p_out;
				Group * lg1 = lg;
				Group * rg1 = rg;
				if(lg1 > rg1) swap(lg1, rg1);
				MergesT::key_type key = make_pair(lg1, rg1);
				MergesT::iterator pm = proposed_merges.find(key);
				if(pm != proposed_merges.end())
					pm -> second+=
		                         log2l(1.0L - qo * powl(qi, sharedCommunities-1))
		                      -  log2l(1.0L - qo * powl(qi, sharedCommunities))
		                      +  log2l(qi) // for a given member of the map, each edge will be found exactly once. So here we cancel the affect of assuming it was disconnected
		                     ;
				}
			}
		}

	}

	int64 merges_accepted = 0;
	int64 merges_applied = 0;
	for(MergesT::const_iterator pm = proposed_merges.begin(); pm != proposed_merges.end(); ++pm) {
		const long double score = pm->second;
		//const int64 N = ging._g.vcount();
		boost::unordered_set<Group *> already_merged;
		if(score >0.0) {
			//PP(scoreEdges);
			//PP(scoreZ);
			merges_accepted++;
			MergesT::key_type merge_these = pm->first;
			if(already_merged.count(merge_these.first)==0 && already_merged.count(merge_these.second)==0) {
				Group * l = merge_these.first;
				Group * r = merge_these.second;
				const set<V> these_nodes(l->vs); // copy them, so as to iterate properly over them.
				//P(" "); PP(ging.groups.size());
				ForeachContainer(V v, these_nodes) {
					if(r->vs.count(v)==0) {
						ging.addV(r,v);
					}
					assert(r->vs.count(v)==1);
					//ging.delV(l,v);
				}
				//PP(ging.groups.size());
				already_merged.insert(merge_these.first);
				already_merged.insert(merge_these.second);
				merges_applied++;
			}
		}
		//if(score > -25.0) { printf("merge: %-11.2Lf\n", score); }
	}
	PP(proposed_merges.size());
	PP(merges_accepted);
	PP(merges_applied);
}
static void tryDeletions(Grouping &ging, bool SaveScores /*= true*/) { // delete groups which aren't making a positive contribution any more.
	Timer timer(__FUNCTION__);
	typedef boost::unordered_map< Group* , long double, hashGroup> DeletionsT;
	DeletionsT proposed_deletions;
	const int64 N = ging._g.vcount();
	ForeachContainer(Group *grp, ging.groups) { // preseed with all groups, because we want the groups even that don't have an edge in them!
				const int64 sz = grp->vs.size();
				      //int64 q = ging.groups.size() -1; if (q<=0) q=1;
				//const int64 q_= 1 + q;
				const long double logNchoosen = logNchoose(N,sz);
				proposed_deletions.insert(make_pair(grp,
					log2l(1.0L - ging._p_in)*(sz*(sz-1)/2)
              + logNchoosen
							- log2l(N+1)/*encoding of size of the group*/
							+ log2l(1+ging.groups.size())/*equivalent groupings*/
							//+ (getenv("UseBroken") ? ( (  q_ * (log2l(q_) - log2l(exp(1))) + log2l(N+1) - log2l(N+1-q_)  ) - (  q  * (log2l(q ) - log2l(exp(1))) + log2l(N+1) - log2l(N+1-q )  ) ) : 0)
					));
	}
	for(V e = 0; e <
			/*100000 */
			2*ging._g.ecount()
		; e++) {
/*
		if(e % (2*ging._g.ecount()/10) ==0) {
			PP(e);
		}
*/
		V e2 = ging.comm_count_per_edge2[e].other_index;
		V sharedCommunities = ging.comm_count_per_edge2[e].shared_comms;
		assert(sharedCommunities == ging.comm_count_per_edge2[e2].shared_comms);
		V l = ging._g.neighbours(0).first[e2];
		V r = ging._g.neighbours(0).first[e];
		if(r<l) continue; // no point considering each edge twice

		const set<Group *> &lgrps = ging.vgroups(l);
		const set<Group *> &rgrps = ging.vgroups(r);
		vector<Group *> sharedComms;
		set_intersection(lgrps.begin(),lgrps.end(),rgrps.begin(),rgrps.end(),back_inserter(sharedComms));
		assert((size_t)sharedCommunities == sharedComms.size());

		ForeachContainer(Group *grp, sharedComms) {
			DeletionsT::iterator pm = proposed_deletions.find(grp);
			assert(pm != proposed_deletions.end());
			pm -> second+=
		                        log2l(1.0L - (1.0L-ging._p_out) * powl(1.0L - ging._p_in, sharedCommunities))
		                     -  log2l(1.0L - (1.0L-ging._p_out) * powl(1.0L - ging._p_in, sharedCommunities-1))
		                     -  log2l(1.0L - ging._p_in)
		                    ;
		}

	}
	assert(proposed_deletions.size() <= ging.groups.size()); // maybe some communities didn't have an edge in them

	V deletions_accepted = 0;
	map<V, int> deletions_sizes;
	PP(ging.groups.size());
	for(DeletionsT::const_iterator pm = proposed_deletions.begin(); pm != proposed_deletions.end(); ++pm) {
		const long double score = pm->second;
		//const int64 N = ging._g.vcount();
		if(score < 0.0) {
			//PP(scoreEdges);
			//PP(scoreZ);
			deletions_accepted++;
			deletions_sizes[pm->first->vs.size()]++;
			{ // delete the group
				set<V> vs = pm->first->vs; // COPY the vertices in
				ForeachContainer(V v, vs) {
					ging.delV(pm->first, v);
				}
				// By now, pm->first will be an invalid pointer, as it will be been delete'd
			}
		}
	}
	P("deletions_accepted: %d\t", deletions_accepted);
	pair<V, int> delete_size;
	ForeachContainer(delete_size, deletions_sizes) {
		P("%d{%d} ", delete_size.second, delete_size.first);
	}
	P("\n");
	PP(ging.groups.size());

	//if(SaveScores && option_saveMOSESscores[0]) Pn("NOT Saving the delta-scores for each comm");
	if(SaveScores && option_saveMOSESscores[0]) {
		Pn("Saving the MOSES delta-score for each community as per the --saveMOSESscores option");
		ofstream saveFile(option_saveMOSESscores);
		ForeachContainer(Group *grp, ging.groups) { // preseed with all groups, because we want the groups even that don't have an edge in them!
			saveFile << proposed_deletions[grp] << '\t' << grp->vs.size() << endl;
		}
	}
}

long double P_x_given_z(const Grouping &ging, long double p_o, long double p_i, int sigma_shared_Xis1) {
	const int64 N = ging._g.vcount();
	const int64 m = ging._g.ecount() / 2L;
	long double logP_XgivenZ = 0.0;
	logP_XgivenZ += log2l(1.0L - p_o) * (N * (N-1) / 2 - m);
	logP_XgivenZ += log2l(1.0L - p_i) * (ging._sigma_shared - sigma_shared_Xis1);
	typedef pair <int,V> edge_countT;
	ForeachContainer(const edge_countT &edge_count, ging.global_edge_counts) {
		const int64 s = edge_count.first;
		const int64 m_s = edge_count.second;
		logP_XgivenZ += log2l(1.0L - (1.0L - p_o)*powl(1.0L - p_i, s)) * m_s;
	}
	return logP_XgivenZ;
}
long double MOSES_objective(const Grouping &ging) {
	Timer t(__FUNCTION__);
	// Three components
		// P(x|z)
		// Qz!
		// product of binomial/N+1
		int64 sigma_shared_Xis1 = 0;
		pair <int,V> edge_count;
		ForeachContainer(edge_count, ging.global_edge_counts) {
			sigma_shared_Xis1 += edge_count.first * edge_count.second;
		}
		long double Pxz = P_x_given_z(ging, ging._p_out, ging._p_in, sigma_shared_Xis1);

		long double Pz = 0.0;
		for (size_t i = 1; i<=ging.groups.size(); i++) {
			Pz += log2l(i); //+ log2l(1+ging.groups.size())/*equivalent groupings*/
			//P(Pz);
		}
		// PP(Pz);
		int64 N = ging._g.vcount();
		ForeachContainer(const Group *grp, ging.groups) {
			long double logNchoosen = logNchoose(N,grp->vs.size());
			DYINGWORDS(logNchoosen <= 0.0) {
				PP(logNchoosen);
			}
			assert(logNchoosen <= 0.0);
			Pz +=		logNchoosen - log2l(N+1) ; 
		}
		// PP(Pxz);
		// PP(Pz);
		// PP(Pxz + Pz);
		// PP(ging.value_of_objective_with_no_communities);
		if(ging.value_of_objective_with_no_communities==1.0)
			Pn("Compression:\t%Lf\t%Lg\t%Lg", 1.0L
				,Pz
				,Pxz
				);
		else
			Pn("Compression:\t%Lf\t%Lg\t%Lg", (Pxz + Pz) / ging.value_of_objective_with_no_communities
				,Pz
				,Pxz
				);
		return Pxz + Pz;
}

static void estimate_p_in_and_p_out(Grouping &ging) {
	Timer t(__FUNCTION__);
	//PP(ging._sigma_shared);
/*
	int64 _sigma_shared2 = 0;
	ForeachContainer(const Group *grp, ging.groups) {
		_sigma_shared2 += int64(grp->vs.size()) * int64(grp->vs.size()-1);
	}
	PP(_sigma_shared2/2);
	assert(_sigma_shared2 = 2 * ging._sigma_shared);
*/
	const int64 N = ging._g.vcount();
	const int64 m = ging._g.ecount() / 2L;
	int64 sigma_shared_Xis1 = 0;
	pair <int,V> edge_count;
	ForeachContainer(edge_count, ging.global_edge_counts) {
		sigma_shared_Xis1 += edge_count.first * edge_count.second;
	}
	//PP(sigma_shared_Xis1);

	map<long double, pair<long double, long double>, greater<long double>  > ALLlogP_XgivenZ;
	for(long double p_i = 0.0L; (p_i+=0.001L) < 1.0L; ) {
	for(long double p_o = 1e-11L; (p_o*=1.1L) < 1.0L; ) {
		long double logP_XgivenZ = 0.0;
		logP_XgivenZ += log2l(1.0L - p_o) * (N * (N-1) / 2 - m);
		logP_XgivenZ += log2l(1.0L - p_i) * (ging._sigma_shared - sigma_shared_Xis1);
		ForeachContainer(edge_count, ging.global_edge_counts) {
			const int64 s = edge_count.first;
			const int64 m_s = edge_count.second;
			logP_XgivenZ += log2l(1.0L - (1.0L - p_o)*powl(1.0L - p_i, s)) * m_s;
		}
		assert(logP_XgivenZ == P_x_given_z(ging ,p_o ,p_i ,sigma_shared_Xis1));
		ALLlogP_XgivenZ[logP_XgivenZ] = make_pair(p_i, p_o);
	}
	}

	pair<long double, pair<long double, long double> > best;
	ForeachContainer(best, ALLlogP_XgivenZ) {
		Pn("BEST: %Lg,%Lg -> %9.0Lf  ", best.second.first, best.second.second, best.first);
		ging._p_in =  best.second.first;
		ging._p_out=  best.second.second;
		break;
	}
}

} // namespace overlapping
