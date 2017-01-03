#ifndef _GRAPH_HPP_
#define _GRAPH_HPP_


#include <vector>
#include "aaron_utils.hpp"


typedef pair<int, int> EdgeType; // TODO: Remove this hardcoding
typedef int VertexIDType; // TODO: Remove this hardcoding
// Define types of vertex names as stored in the file.

template <class VertexNameType>
struct bloomGraph;

string VertexNameToString (const int &l);

template<typename H>
H hash_integer( // This is used in populateHash and key_for_vertexName for help speed up the latter.
		unsigned int name /*The type of Int::l*/
		, H maxHash) {
	return (unsigned int) (name * 2654435761U) // a simple idea from Knuth apparently, based on the golden ratio. http://www.concentric.net/~Ttwang/tech/inthash.htm
			% maxHash;
}

template <class VertexNameType>
struct bloomGraph {
public:
//#define offset_to_edges second
	typedef ::EdgeType EdgeType;
	typedef unsigned int ID;
	typedef VertexNameType Name;

	bloomGraph(void) : vertex_count(0), edge_count(0) {}

	long unsigned int vcount(void) const { return vertex_count ; }
	long unsigned int ecount(void) const { return edge_count / 2; } // This is one place where we continue to assume undirected graph.
	inline long unsigned int degree(long int v) const { return degrees.at(v); }
	VertexNameType name_of_one_node(long int v) const { return vertex_mappings.at(v); }
	string name_of_one_node_asString(long int v) const { return VertexNameToString(name_of_one_node(v)); }
	long int key_for_vertexName(VertexNameType v) const;
	bool are_connected(const EdgeType &e) const { 
		if (degree(e.first) < degree(e.second))
			return binary_search( neighbours(e.first).first ,neighbours(e.first).second ,e.second);
		else
			return binary_search( neighbours(e.second).first ,neighbours(e.second).second ,e.first);
	}
	pair<const VertexIDType*,const VertexIDType*> neighbours(VertexIDType i) const { // This version promises not to modify the edges
		return make_pair(
			&(edge_targets[offsets.at(i)])
			,  (&(edge_targets[offsets.at(i+1)]))
		);
	}
	pair<VertexIDType*,VertexIDType*> neighbours(VertexIDType i) { 
		return make_pair(
			&(edge_targets[offsets.at(i)])
			,  (&(edge_targets[offsets.at(i+1)]))
		);
	}
public: // TODO: Make this private again
	vector<VertexNameType > vertex_mappings; // This will probably have one extra element as a sentinel. Do NOT rely on its size() or end() members. Use the range instead.
	vector<unsigned int> degrees;
	vector<unsigned int> offsets;
	vector<VertexIDType> edge_targets;
public:
	long int vertex_count;
	long int edge_count;
	vector<unsigned int> hash_offsets; // This will help speed up key_for_vertexName
	// friend void loadBloomGraphMMAP<VertexNameType>(bloomGraph &bg, const char *fileName);
	// friend void populateVerticesAndCountDegree<VertexNameType>  (const char *fileBegin, const char* fileEnd, bloomGraph &bg);
	// friend void convertDegreesToOffsets<VertexNameType>  (bloomGraph &bg);
	// friend void loadEdges<VertexNameType>  (const char *fileBegin, const char* fileEnd, bloomGraph &bg);
	// friend void populateHash<VertexNameType>(bloomGraph<VertexNameType> &bg);
};


string VertexNameToString (const int&);

typedef int V;
typedef bloomGraph<int> SimpleIntGraph; // A simple (no loops or multiple edges), unweighted, undirected immutable graph. The vertex ids and names are ints, hence 2 billion max.


#endif
