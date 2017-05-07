/*
 *  lf_community_reader.cpp
 *  conv
 *
 *  Created by Erwan Le Martelot on 23/02/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "lf_community_reader.h"
#include "communities.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "registry.h"

namespace mscd {
namespace io {
namespace in {

static bool registered = toolkit::Registry<CommunityReader>::GetInstance().Register(new LFCommunityReader);

bool LFCommunityReader::FromFile(const std::string & filename, ds::Communities & coms) {
	std::ifstream comfile(filename.c_str());
	if (!comfile.is_open()) return false;;
	std::vector<int> ncom;
	std::string line;
	int n, s = 0;
	while (std::getline(comfile, line)) {
		std::istringstream is(line);
		is >> n; // node number
		is >> n; // community
		ncom.push_back(n-1);
		if (s<n) s=n;
	}
	comfile.close();
	coms.Clear();
	coms.Resize(s);
	std::vector< std::vector<long> > & cs = coms.GetCommunityList();
	for (int i=0; i<ncom.size(); ++i) cs[ncom[i]].push_back(i);
	return true;
}

} // namespace in
} // namespace io	
} // namespace mscd
