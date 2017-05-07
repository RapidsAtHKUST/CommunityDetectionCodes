/*
 *  pajek_graph_reader.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 13/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_PAJEK_GRAPH_READER_H_
#define MSCD_PAJEK_GRAPH_READER_H_

// Includes
#include "graph_reader.h"

namespace mscd {
namespace io {
namespace in {
			
/** Pajek file reader
 *	Load a graph from an file using Pajek basic format.
 */
class PajekGraphReader: public GraphReader {
				
public:
				
	/// Return the extension the reader handles
	virtual std::string GetName() const { return "net"; }
				
	/// Load a graph from the given input file
	virtual bool FromFile(const std::string &, ds::Graph &);
				
};
			
} // namespace in
} // namespace io
} // namespace mscd

#endif