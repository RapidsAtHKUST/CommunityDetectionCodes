/*
 *  pajek_graph_reader.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 13/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "pajek_graph_reader.h"
#include "graph.h"
#include <cstdio>
#include <vector>
#include "registry.h"

namespace mscd {
namespace io {
namespace in {
			
static bool registered = toolkit::Registry<GraphReader>::GetInstance().Register(new PajekGraphReader);
			
bool PajekGraphReader::FromFile(const std::string & filename, ds::Graph & g) {
	FILE * f = fopen(filename.c_str(), "r");
	if (!f) return false;
	char buf[1024];
	long src, trg, n;
	float w;
	bool done = false, skip = false;
	do {
		if (!fgets(buf, sizeof buf, f)) done = true;
		else if (buf[0] == '*') {
			if (strncmp(buf,"*Network",8) == 0) {
				skip = true;
				fprintf(stderr, "Pajek reader: skipping *Network section\n");
			}
			else if (strncmp(buf,"*Vertices",9) == 0) {
				if (sscanf(buf+9, "%ld", &n) != 1) {
					done = true;
					fprintf(stderr, "Error: incorrect file format: No number of vertices\n");
				}
				else g.Resize(n);
				skip = true;
			}
			else if (strncmp(buf,"*Arcs",5) == 0) {
				skip = true;
				fprintf(stderr, "Pajek reader: skipping *Arcs section\n");
			}
			else if (strncmp(buf,"*Edges",6) == 0) skip = false;
			else {
				done = true;
				fprintf(stderr, "Error: incorrect file format\n");
			}
		}
		else if (!skip) {
			n = sscanf(buf, "%ld%ld%f", &src, &trg, &w);
			if (n < 2) done = true;
			else g.AddEdge(src,trg,(n>2)?w:1.);
		}
	}
	while (!done);
	return (fclose(f) == 0);
}
			
} // namespace in
} // namespace io	
} // namespace mscd
