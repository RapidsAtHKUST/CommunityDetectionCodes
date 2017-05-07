/*
 *  pajek_community_reader.h
 *  conv
 *
 *  Created by Erwan Le Martelot on 15/02/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_PAJEK_COMMUNITY_READER_H_
#define MSCD_PAJEK_COMMUNITY_READER_H_

// Includes
#include "community_reader.h"

namespace mscd {
namespace io {
namespace in {

/** Pajek community file reader
 */
class PajekCommunityReader: public CommunityReader {
	
public:
	
	/// Return the extension the reader handles
	virtual std::string GetName() const { return "clu"; }
	
	/// Load a community from the given input file
	virtual bool FromFile(const std::string &, ds::Communities &);
	
};

} // namespace in
} // namespace io
} // namespace mscd

#endif