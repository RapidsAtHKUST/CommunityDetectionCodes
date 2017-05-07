/*
 *  edge_list_graph_reader.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 28/11/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_EDGE_LIST_GRAPH_READER_H_
#define MSCD_EDGE_LIST_GRAPH_READER_H_

// Includes
#include "graph_reader.h"

namespace mscd {
namespace io {
namespace in {
			
/** Edge list file reader
 *	Load a graph from an edgelist where each line contains 'source target weight'.
 *	The weight can be omitted in which case it is set to 1.
 *	Comments can be given using a '#' symbol as the first character of the line.
 */
class EdgeListGraphReader: public GraphReader {
	
public:
	
	/// Return the extension the reader handles
	virtual std::string GetName() const { return "edges"; }
	
	/// Load a graph from the given input file
	virtual bool FromFile(const std::string &, ds::Graph &);

};

} // namespace in
} // namespace io
} // namespace mscd

#endif