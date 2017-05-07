/*
 *  community_list_writer.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 19/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_COMMUNITY_LIST_WRITER_H_
#define MSCD_COMMUNITY_LIST_WRITER_H_

// Includes
#include "community_writer.h"

namespace mscd {
namespace io {
namespace out {
			
/** Community list file writer
 */
class CommunityListWriter: public CommunityWriter {
	
public:
	
	/// Return the extension the writer handles
	virtual std::string GetName() const { return "coms"; }
	
	/// Write to file a series of list of nodes
	virtual bool ToFile(const std::string &, const ds::Communities &) const;
	
};

} // namespace out
} // namespace io
} // namespace mscd

#endif