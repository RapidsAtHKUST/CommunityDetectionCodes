/*
 *  community_list_writer.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 19/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "community_list_writer.h"
#include "communities.h"
#include <cstdio>
#include <vector>

/// Register reader
#include "registry.h"

namespace mscd {
namespace io {
namespace out {
			
static bool registered = toolkit::Registry<CommunityWriter>::GetInstance().Register(new CommunityListWriter);

bool CommunityListWriter::ToFile(const std::string & filename, const ds::Communities & coms) const {
	FILE * f = fopen(filename.c_str(), "w");
	if (!f) return false;
	for (long i=0, j; i<coms.GetNbCommunities(); ++i) {
		const std::vector<long> & com = coms.GetCommunity(i);
		for (j=0; j<com.size(); ++j)
			if (j<com.size()-1) fprintf(f, "%ld ", com[j]+1);
			else fprintf(f, "%ld", com[j]+1);
		fprintf(f, "\n");
	}
	return (fclose(f) == 0);
}

} // namespace out
} // namespace io
} // namespace mscd