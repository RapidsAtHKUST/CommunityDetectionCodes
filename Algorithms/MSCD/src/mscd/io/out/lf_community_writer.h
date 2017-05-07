/*
 *  lf_community_writer.h
 *  conv
 *
 *  Created by Erwan Le Martelot on 23/02/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_LF_COMMUNITY_WRITER_H_
#define MSCD_LF_COMMUNITY_WRITER_H_

// Includes
#include "community_writer.h"

namespace mscd {
namespace io {
namespace out {

/** Lancichinetti and Fortunato benchmark community file writer
 */
class LFCommunityWriter: public CommunityWriter {
	
public:
	
	/// Return the extension the writer handles
	virtual std::string GetName() const { return "dat"; }
	
	/// Write to file a series of list of nodes
	virtual bool ToFile(const std::string &, const ds::Communities &) const;
	
};

} // namespace out
} // namespace io
} // namespace mscd

#endif