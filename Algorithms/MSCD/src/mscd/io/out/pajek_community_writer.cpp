/*
 *  pajek_community_writer.cpp
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 13/01/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "pajek_community_writer.h"
#include "communities.h"
#include <cstdio>
#include <vector>

/// Register reader
#include "registry.h"

namespace mscd {
namespace io {
namespace out {
			
static bool registered = toolkit::Registry<CommunityWriter>::GetInstance().Register(new PajekCommunityWriter);
			
bool PajekCommunityWriter::ToFile(const std::string & filename, const ds::Communities & coms) const {
	FILE * f = fopen(filename.c_str(), "w");
	if (!f) return false;
	std::map<long,long> commap;
	for (int i=0, j; i<coms.GetNbCommunities(); ++i) {
		std::vector<long> com = coms.GetCommunity(i);
		for (j=0; j<com.size(); ++j) commap[com[j]] = i + 1;
	}
	fprintf(f, "*Vertices %d\n", static_cast<int>(commap.size()));
	for (std::map<long,long>::const_iterator cit = commap.begin(); cit != commap.end(); ++cit)
		fprintf(f, "%ld\n", cit->second);
	return (fclose(f) == 0);
}
			
} // namespace out
} // namespace io
} // namespace mscd