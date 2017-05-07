/*
 *  graph_reader.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 28/11/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_GRAPH_READER_H_
#define MSCD_GRAPH_READER_H_

// Includes
#include <string>

namespace mscd {
	
namespace ds{
// Forward declarations
class Graph;
}

namespace io {
namespace in {

/** Graph reader abstract class.
 *	To be implemented by readers of specific formats. 
 */
class GraphReader {
	
public:

    /// Constructor
    GraphReader() : verbose(true) {}
    
    /// Virtual destructor
    virtual ~GraphReader() {}
    
	/// Return extensions the reader handles
	virtual std::string GetName() const = 0;
	
	/// Read from file
	virtual bool FromFile(const std::string &, ds::Graph &) = 0;

    /// Set verbose mode
	inline void SetVerbose(const bool value) { this->verbose = value; }
	
	/// True if verbose mode is on
	inline bool IsVerbose() const { return this->verbose; }
    
protected:
	
	/// Verbose mode
	bool verbose;
    
};

} // namespace in
} // namespace io
} // namespace mscd

#endif