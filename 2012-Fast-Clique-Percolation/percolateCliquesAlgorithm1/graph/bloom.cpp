#include "bloom.hpp"
#include <tr1/unordered_set>
#include "../pp.hpp"
namespace graph {
namespace bloom {
// a Bloom filter to speed up VerySimpleGraphInterface :: are_connected(int32_t node_id1, int32_t node_id2)

BloomAreConnected :: BloomAreConnected ( const VerySimpleGraphInterface * vsg ) {
	assert(vsg);
	const int32_t R = vsg->numRels();
	const size_t sz = 10 * R; // was 100, now just 10, so as to run on the Idiro machine :-(
	assert(this->bloom.size()==0);
	this->bloom.resize(sz);
	assert(this->bloom.size()==sz);
	// PP2(R,sz);
	int num_trues = 0;
	int num_clashes = 0;
	for(int r=0; r<R;r++) {
		size_t h = this->hash(vsg, r);
		// PP2(r, h);
		if(!this->bloom.at(h % bloom.size()))
			++ num_trues;
		else
			++ num_clashes;
		this->bloom.at(h % bloom.size()) = true;
	}
	// std :: cerr << "num_trues " << num_trues << std :: endl;
	// std :: cerr << "num_clashes " << num_clashes << std :: endl;
}
size_t BloomAreConnected :: hash( const VerySimpleGraphInterface * vsg, const int32_t rel_id )  const {
	assert(vsg);
	assert(rel_id >= 0 && rel_id < vsg->numRels());
	const std :: pair<int32_t, int32_t> & eps = vsg->EndPoints(rel_id);
	assert(eps.first <= eps.second);
	return this -> hash(eps.first, eps.second);
	// return std :: tr1 :: hash<int>()(eps.first) ^ std :: tr1 :: hash<int>()(-eps.second);
}
size_t BloomAreConnected :: hash( int32_t node_id_1, int32_t node_id_2)  const {
	if(node_id_1 > node_id_2) {
		std :: swap(node_id_1, node_id_2);
	}
	assert(node_id_1 <= node_id_2);
	// PP3  ( std :: tr1 :: hash<int>()(node_id_1) , std :: tr1 :: hash<int>()(-node_id_2), std :: tr1 :: hash<int>()(node_id_1) ^ std :: tr1 :: hash<int>()(-node_id_2) );
	return node_id_1 - 100000 * node_id_2;
}

} // namespace bloom
} // namespace graph

