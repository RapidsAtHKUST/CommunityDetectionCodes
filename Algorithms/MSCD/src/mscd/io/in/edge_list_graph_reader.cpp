/*
 *  edge_list_graph.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 12/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "edge_list_graph_reader.h"
#include "graph.h"
#include <cstdio>
#include <vector>
#include <map>
#include "registry.h"

namespace mscd {
namespace io {
namespace in {
			
static bool registered = toolkit::Registry<GraphReader>::GetInstance().Register(new EdgeListGraphReader);

/// Read edges from a file and loads them into a map, then reindex the nodes to add in the graph
bool EdgeListGraphReader::FromFile(const std::string & filename, ds::Graph & g) {
	FILE * f = fopen(filename.c_str(), "r");
	if (!f) return false;
	char buf[1024];
	long src, trg, n;
	float w;
	std::map<long,std::vector<std::pair<long,float> > > adjmap;
	bool done = false;
	long nb_edges = 0;
	do {
		if (!fgets(buf, sizeof buf, f)) done = true;
		else if (buf[0] != '#') {
			n = sscanf(buf, "%ld%ld%f", &src, &trg, &w);
			if (n < 2) done = true;
			else {
				// Create node entry in map if necessary
				if (adjmap.find(src) == adjmap.end())
					adjmap[src] = std::vector<std::pair<long,float> >();
				if (adjmap.find(trg) == adjmap.end())
					adjmap[trg] = std::vector<std::pair<long,float> >();
				// Add edge to map
				adjmap[src].push_back(std::make_pair(trg,(n>2)?w:1.));
				++nb_edges;
			}
		}
	}
	while (!done);
	// Find index or each node
	std::map<long,long> nodeidx;
	std::map<long,std::vector<std::pair<long,float> > >::const_iterator it;
	for (it = adjmap.begin(), n=1; it != adjmap.end(); ++it, ++n)
		nodeidx[it->first] = n;
	// Resize graoh to its number of nodes
	g.Resize(adjmap.size());
	// Add final edges
	long nb_errors = 0, nb_sl = 0;
	for (it = adjmap.begin(); it != adjmap.end(); ++it) {
		for (n=0; n<it->second.size(); ++n) {
			if (it->first == it->second[n].first) ++nb_sl;
			if (!g.AddEdge(nodeidx[it->first],nodeidx[it->second[n].first],it->second[n].second)) {
				//fprintf(stderr, "Cannot add edge %d->%d (%f)\n", it->first, it->second[n].first, it->second[n].second);
				++nb_errors;
			}
		}
	}
    if (this->verbose) {
        fprintf(stdout, "Loaded %ld edges\n", nb_edges);
        fprintf(stdout, "Encountered %ld errors (or duplicates) adding edges\n", nb_errors);
        fprintf(stdout, "Graph: %ld edges including %ld self-loops. Total weight = %f\n", g.GetNbEdges(), nb_sl, g.GetTotalWeight());
	}
    return (fclose(f) == 0);
}
	
} // namespace in
} // namespace io	
} // namespace mscd
