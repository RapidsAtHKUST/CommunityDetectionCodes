/*
 *  edge_list_graph_writer.cpp
 *  grconv
 *
 *  Created by Erwan Le Martelot on 13/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "edge_list_graph_writer.h"
#include "graph.h"
#include <cstdio>
#include <vector>
#include <cmath>
#include "registry.h"

namespace mscd {
namespace io {
namespace out {
			
static bool registered = toolkit::Registry<GraphWriter>::GetInstance().Register(new EdgeListGraphWriter);
			
bool EdgeListGraphWriter::ToFile(const std::string & filename, const ds::Graph & g) {
	FILE * f = fopen(filename.c_str(), "w");
	if (!f) return false;
	const ds::Graph::adjlist & al = g.GetAdjList();
	for (long i=0, j; i<al.size(); ++i)
		for (j=0; j<al[i].size(); ++j)
			if (std::floor(al[i][j].second) == al[i][j].second)
				fprintf(f, "%ld %ld %ld\n", i+1, al[i][j].first+1, static_cast<long>(al[i][j].second));
			else
				fprintf(f, "%ld %ld %f\n", i+1, al[i][j].first+1, al[i][j].second);
	return (fclose(f) == 0);
}
			
} // namespace out
} // namespace io	
} // namespace mscd
