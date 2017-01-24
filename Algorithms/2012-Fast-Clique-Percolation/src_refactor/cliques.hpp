#ifndef _CLIQUES_HPP_
#define _CLIQUES_HPP_


#include "graph/network.hpp"

typedef const graph :: VerySimpleGraphInterface * SimpleIntGraph;

namespace cliques {

void cliquesToStdout          (const graph :: NetworkInterfaceConvertedToString * net, unsigned int minimumSize); // You're not allowed to ask for the 2-cliques
void cliquesToVector(std :: vector< std :: vector<int64_t> > &all_cliques_by_orig_name, graph :: NetworkInterfaceConvertedToString *net, unsigned int minimumSize /* = 3*/ );

} // namespace cliques


#endif
