#include "network.hpp"
namespace graph {

template <class NodeNameT>
NetworkInterface<NodeNameT> :: NetworkInterface(const bool directed, const bool weighted) {
	if(directed) {
		if(weighted) {
			edge_weights.reset(new graph :: weights :: EdgeDetails< graph :: weights :: DirectedLDoubleWeights >());
		} else {
			edge_weights.reset(new graph :: weights :: EdgeDetails< graph :: weights :: DirectedNoWeights >());
		}
	} else {
		if(weighted) {
			edge_weights.reset(new graph :: weights :: EdgeDetails< graph :: weights :: WeightNoDir >());
		} else {
			edge_weights.reset(new graph :: weights :: EdgeDetails< graph :: weights :: NoDetails >());
		}
	}
}
template <class NodeNameT>
NetworkInterface<NodeNameT> :: ~ NetworkInterface() throw() {
}
NetworkInterfaceConvertedToString :: ~ NetworkInterfaceConvertedToString() {
}

template NetworkInterface<NodeNameIsInt64>  :: NetworkInterface(const bool directed, const bool weighted);
template NetworkInterface<NodeNameIsString> :: NetworkInterface(const bool directed, const bool weighted);
template NetworkInterface<NodeNameIsInt64>  :: ~NetworkInterface();
template NetworkInterface<NodeNameIsString> :: ~NetworkInterface();
} // namespace graph
