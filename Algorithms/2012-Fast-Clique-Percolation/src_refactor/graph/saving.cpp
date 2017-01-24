#include "saving.hpp"
#include "../pp.hpp"

#include <string>

using namespace std;

namespace graph {
namespace saving {

void print_Network_to_screen( const graph :: NetworkInterfaceConvertedToStringWithWeights * net ) {
	const graph :: weights :: EdgeDetailsInterface * const edge_weights = net->get_edge_weights();
	for(int r=0; r< net->numRels(); r++) {
		const pair<int,int> eps = net->get_plain_graph()->EndPoints(r);
		assert(eps.first <= eps.second); // the *relationship* is alway stored as lowest_node_id ---> highest_node_id (or maybe it's a self-loop)
		const string node_1_name = net->node_name_as_string(eps.first);
		const string node_2_name = net->node_name_as_string(eps.second);
		const long double weight_1_to_2 = edge_weights->getl2h(r); // might be zero of course, if there's no edge in this direction
		const long double weight_2_to_1 = edge_weights->geth2l(r); // might be zero of course, if there's no edge in this direction
		if(weight_1_to_2)
			cout << ";\t" << node_1_name << "\t" << node_2_name << "\t" << weight_1_to_2 << endl;
		if(weight_2_to_1)
			cout << ";\t" << node_2_name << "\t" << node_1_name << "\t" << weight_2_to_1 << endl;
	}
}

} // namespace graph
} // namespace saving

