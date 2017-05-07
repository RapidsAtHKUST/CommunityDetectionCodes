/*
 *  community_reader.h
 *  conv
 *
 *  Created by Erwan Le Martelot on 15/02/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_COMMUNITY_READER_H_
#define MSCD_COMMUNITY_READER_H_

// Includes
#include <string>

namespace mscd {
	
namespace ds{
// Forward declarations
class Communities;
}
	
namespace io {
namespace in {
			
/** Community reader abstract class.
 *	To be implemented by readers of specific formats. 
 */
class CommunityReader {
	
public:
	
    /// Virtual destructor
    virtual ~CommunityReader() {}
    
	/// Return extensions the reader handles
	virtual std::string GetName() const = 0;
	
	/// Read from file
	virtual bool FromFile(const std::string &, ds::Communities &) = 0;
	
};
			
} // namespace in
} // namespace io
} // namespace mscd

#endif