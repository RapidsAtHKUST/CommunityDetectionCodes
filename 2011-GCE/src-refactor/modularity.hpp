
#include "graph_representation.hpp"

namespace modularity {
	void calculateModularity(const bloomGraph<int> &g, const string &assignmentsFileName, const string &outputCommunityStatsFileName, const bloomGraph<int> *graphToCompareTo);
/*
 * WARNING. calculateModularity (currently) assumes that the ids in the assignmentsFileName are the unique consecutive indices into the list of sorted vertex names. i.e. they don't map directly to the names in the original graph data file.
 */
}
