#include "graph.hpp"
namespace graph {

neighbouring_node_id_iterator neighbouring_node_id_iterator :: end_marker() const {
	const neighbouring_node_id_iterator ender;
	return ender;
}

} // namespace graph
