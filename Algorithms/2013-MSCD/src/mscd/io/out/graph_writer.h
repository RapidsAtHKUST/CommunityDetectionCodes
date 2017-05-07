/*
 *  graph_writer.h
 *  grconv
 *
 *  Created by Erwan Le Martelot on 13/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_GRAPH_WRITER_H_
#define MSCD_GRAPH_WRITER_H_

// Includes
#include <string>

namespace mscd {
	
namespace ds{
	// Forward declarations
	class Graph;
}
	
namespace io {
namespace out {
			
/** Graph writer abstract class.
 *	To be implemented by writers of specific formats. 
 */
class GraphWriter {
				
public:
			
	/// Virtual destructor
    virtual ~GraphWriter() {}
    
	/// Return extensions the writer handles
	virtual std::string GetName() const = 0;
				
	/// Write to file
	virtual bool ToFile(const std::string &, const ds::Graph &) = 0;				
				
};
			
} // namespace out
} // namespace io
} // namespace mscd

#endif