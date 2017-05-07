/*
 *  community_writer.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 19/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_COMMUNITY_WRITER_H_
#define MSCD_COMMUNITY_WRITER_H_

// Includes
#include <string>

namespace mscd {
	
namespace ds {
// Forward declarations
class Communities;	
}
	
namespace io {
namespace out {

/** Community writer abstract class.
 *	To be implemented by writers in specific formats. 
 */
class CommunityWriter {
	
public:
    
    /// Virtual destructor
    virtual ~CommunityWriter() {}
	
	/// Return extensions the writer handles
	virtual std::string GetName() const = 0;
	
	/// Write to file
	virtual bool ToFile(const std::string &, const ds::Communities &) const = 0;
	
};

} // namespace out
} // namespace io
} // namespace mscd

#endif