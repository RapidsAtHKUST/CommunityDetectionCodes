#include "graph_representation.hpp"

namespace graph_analysis {

/* Declarations */

void basicGraphStats(const SimpleIntGraph &);
#ifdef USE_BOOST
void connectedComponents(const SimpleIntGraph &, const char * option_largestComponent);
#endif


/* Definitions */


} // namespace graph_analysis
