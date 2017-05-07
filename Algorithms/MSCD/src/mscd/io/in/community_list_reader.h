/*
 *  community_list_reader.h
 *  conv
 *
 *  Created by Erwan Le Martelot on 15/02/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_COMMUNITY_LIST_READER_H_
#define MSCD_COMMUNITY_LIST_READER_H_

// Includes
#include "community_reader.h"

namespace mscd {
namespace io {
namespace in {

/** Community list file reader
 *	Load a set of communities from an community file where each line i contains
 *	the list of nodes belonging to community i.
 */
class CommunityListReader: public CommunityReader {
	
public:
	
	/// Return the extension the reader handles
	virtual std::string GetName() const { return "coms"; }
	
	/// Load a community from the given input file
	virtual bool FromFile(const std::string &, ds::Communities &);
	
};

} // namespace in
} // namespace io
} // namespace mscd

#endif