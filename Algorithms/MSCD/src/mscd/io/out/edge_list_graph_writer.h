/*
 *  edge_list_graph_writer.h
 *  grconv
 *
 *  Created by Erwan Le Martelot on 13/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_EDGE_LIST_GRAPH_WRITER_H_
#define MSCD_EDGE_LIST_GRAPH_WRITER_H_

// Includes
#include "graph_writer.h"

namespace mscd {
namespace io {
namespace out {
			
/** Edge list file writer
 *	Write a graph to an edgelist where each line contains 'source target weight'.
 */
class EdgeListGraphWriter: public GraphWriter {
				
public:
	
	/// Return the extension the writer handles
	virtual std::string GetName() const { return "edges"; }
				
	/// Write the given graph to the given output file
	virtual bool ToFile(const std::string &, const ds::Graph &);
				
};
		
} // namespace out
} // namespace io
} // namespace mscd

#endif