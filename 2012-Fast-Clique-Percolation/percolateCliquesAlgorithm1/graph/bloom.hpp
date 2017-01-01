#include <vector>
#include "graph.hpp"
namespace graph {
namespace bloom {
// a Bloom filter to speed up VerySimpleGraphInterface :: are_connected(int32_t node_id_1, int32_t node_id_2)

class BloomAreConnected;

class BloomAreConnected {
public:
	std :: vector<bool> bloom;
	BloomAreConnected ( const graph :: VerySimpleGraphInterface * vsg );
	size_t hash( const graph :: VerySimpleGraphInterface * vsg, const int32_t rel_id ) const ;
	size_t hash( int32_t node_id_1, int32_t node_id_2 ) const ;
	bool test( const int32_t node_id_1, const int32_t node_id_2 ) const {
		return this->bloom.at( this->hash(node_id_1, node_id_2) % bloom.size() );
	}
};

} // namespace bloom
} // namespace graph

