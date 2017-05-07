/*
 *  pajek_community_writer.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 13/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_PAJEK_COMMUNITY_WRITER_H_
#define MSCD_PAJEK_COMMUNITY_WRITER_H_

// Includes
#include "community_writer.h"

namespace mscd {
namespace io {
namespace out {
			
/** Pajek community file writer
 */
class PajekCommunityWriter: public CommunityWriter {
				
public:
				
	/// Return the extension the writer handles
	virtual std::string GetName() const { return "clu"; }
				
	/// Write to file a series of list of nodes
	virtual bool ToFile(const std::string &, const ds::Communities &) const;
				
};
			
} // namespace out
} // namespace io
} // namespace mscd

#endif