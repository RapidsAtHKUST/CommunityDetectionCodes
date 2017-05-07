/*
 *  lf_community_reader.h
 *  conv
 *
 *  Created by Erwan Le Martelot on 23/02/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_LF_COMMUNITY_READER_H_
#define MSCD_LF_COMMUNITY_READER_H_

// Includes
#include "community_reader.h"

namespace mscd {
namespace io {
namespace in {
	
/** Lancichinetti and Fortunato benchmark community file reader
 */
class LFCommunityReader: public CommunityReader {
	
public:
	
	/// Return the extension the reader handles
	virtual std::string GetName() const { return "dat"; }
	
	/// Load a community from the given input file
	virtual bool FromFile(const std::string &, ds::Communities &);
	
};
	
} // namespace in
} // namespace io
} // namespace mscd

#endif