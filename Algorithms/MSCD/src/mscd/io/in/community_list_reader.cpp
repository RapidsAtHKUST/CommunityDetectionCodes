/*
*  community_list_reader.cpp
*  conv
*
*  Created by Erwan Le Martelot on 15/02/2012.
*  Copyright 2012 __MyCompanyName__. All rights reserved.
*
*/

#include "community_list_reader.h"
#include "communities.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "registry.h"

namespace mscd {
namespace io {
namespace in {

static bool registered = toolkit::Registry<CommunityReader>::GetInstance().Register(new CommunityListReader);

bool CommunityListReader::FromFile(const std::string & filename, ds::Communities & coms) {
	std::ifstream comfile(filename.c_str());
	if (!comfile.is_open()) return false;
	coms.Clear();
	std::vector<long> c;
	std::string line;
	long n;
	while (std::getline(comfile, line)) {
		std::istringstream is(line);
		c.clear();
		while (!is.eof()) {
			is >> n;
			c.push_back(n-1);
		}
		if (!c.empty()) coms.CreateCommunity().swap(c);
	}
	comfile.close();
	return true;
}

} // namespace in
} // namespace io	
} // namespace mscd
