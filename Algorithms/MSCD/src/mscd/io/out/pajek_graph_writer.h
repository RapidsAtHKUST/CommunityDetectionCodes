/*
 *  pajek_graph_writer.h
 *  grconv
 *
 *  Created by Erwan Le Martelot on 13/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_PAJEK_GRAPH_WRITER_H_
#define MSCD_PAJEK_GRAPH_WRITER_H_

// Includes
#include "graph_writer.h"

namespace mscd {
namespace io {
namespace out {
			
/** Pajek file writer
 *	Write a graph in the Pajek file format.
 */
class PajekGraphWriter: public GraphWriter {
				
public:
				
	/// Return the extension the writer handles
	virtual std::string GetName() const { return "net"; }
				
	/// Write the given graph to the given output file
	virtual bool ToFile(const std::string &, const ds::Graph &);
				
};
			
} // namespace out
} // namespace io
} // namespace mscd

#endif