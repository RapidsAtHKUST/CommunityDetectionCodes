using namespace std;
#include "graph_representation.hpp"
#include <iostream>


template <> // TODO: I dunno why I can't do a generic one for key_for_vertexName. It seems I have to specialize it for Int
V bloomGraph<int>::key_for_vertexName(int v) const {
	// Don't use this function until populateVerticesAndCountDegree and populateHash is complete.
	if(!hash_offsets.empty()) {
		unsigned int h = hash_integer<unsigned int>(v, hash_offsets.size());
		for(unsigned int j = 0; j<5 && h+j < hash_offsets.size() ; j++) {
			unsigned int k = hash_offsets.at(h+j);
			assert(k < vertex_mappings.size());
			if(vertex_mappings.at(k) == v) {
				return k;
			}
		}
	}
	const int * lb = &(*lower_bound(vertex_mappings.begin(),vertex_mappings.begin()+vcount(),v));
	const int * begin = &(*vertex_mappings.begin());
	return lb - begin;
}
template <> // TODO: I dunno why I can't do a generic one for key_for_vertexName. It seems I have to specialize it for Int
V bloomGraph<string>::key_for_vertexName(string v) const {
	// Don't use this function until populateVerticesAndCountDegree and populateHash is complete.
	if(!hash_offsets.empty()) {
		unsigned int h = hash_integer<unsigned int>(v, hash_offsets.size());
		for(unsigned int j = 0; j<5 && h+j < hash_offsets.size() ; j++) {
			unsigned int k = hash_offsets.at(h+j);
			assert(k < vertex_mappings.size());
			if(vertex_mappings.at(k) == v) {
				return k;
			}
		}
	}
	const string * lb = &(*lower_bound(vertex_mappings.begin(),vertex_mappings.begin()+vcount(),v));
	const string * begin = &(*vertex_mappings.begin());
	return lb - begin;
}


string VertexNameToString (const int &l) {
	ostringstream s; s << l; return s.str();
}


