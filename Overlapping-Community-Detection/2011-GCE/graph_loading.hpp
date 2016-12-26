
#include "graph_representation.hpp"
#include "Range.hpp"

namespace graph_loading {

void loadBloomGraphMMAP(SimpleIntGraph &bg, const char *fileName);
void loadSimpleIntGraphFromFile(SimpleIntGraph &bg, const char *fileName);

} // namespace graph_loading
