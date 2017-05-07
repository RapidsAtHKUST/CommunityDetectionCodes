/*
 *  lf_community_writer.cpp
 *  conv
 *
 *  Created by Erwan Le Martelot on 23/02/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "lf_community_writer.h"
#include "communities.h"
#include <cstdio>
#include <vector>

/// Register reader
#include "registry.h"

namespace mscd {
namespace io {
namespace out {

static bool registered = toolkit::Registry<CommunityWriter>::GetInstance().Register(new LFCommunityWriter);

bool LFCommunityWriter::ToFile(const std::string & filename, const ds::Communities & coms) const {
	const std::vector<std::vector<long> > & cs = coms.GetCommunityList();
	long s = 0;
	for (long i=0, j; i<cs.size(); ++i)
		for (j=0; i<cs[i].size(); ++j)
			if (cs[i][j] >= s) s = cs[i][j]+1;
	std::vector<long> ncom(s);
	for (long i=0, j; i<cs.size(); ++i)
		for (j=0; i<cs[i].size(); ++j)
			ncom[cs[i][j]] = i;
	FILE * f = fopen(filename.c_str(), "w");
	if (!f) return false;
	for (long i=0; i<ncom.size(); ++i)
		fprintf(f, "%ld %ld\n", i+1, ncom[i]+1);
	return (fclose(f) == 0);
}

} // namespace out
} // namespace io
} // namespace mscd