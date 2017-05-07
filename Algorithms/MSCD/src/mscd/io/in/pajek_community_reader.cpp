/*
 *  pajek_community_reader.cpp
 *  conv
 *
 *  Created by Erwan Le Martelot on 15/02/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#include "pajek_community_reader.h"
#include "communities.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <map>
#include "registry.h"

namespace mscd {
namespace io {
namespace in {
	
static bool registered = toolkit::Registry<CommunityReader>::GetInstance().Register(new PajekCommunityReader);

bool PajekCommunityReader::FromFile(const std::string & filename, ds::Communities & coms) {
	std::ifstream comfile(filename.c_str());
	if (!comfile.is_open()) return false;
	std::string vert; comfile >> vert;
	if (vert != "*Vertices") return false;
	// Read community for each node
	std::vector<long> ncom;
	std::set<long> coms_id;
	long n; comfile >> n;
	ncom.resize(n);
	for (long i=0; i< ncom.size(); ++i) {
		if (comfile.eof()) {
			comfile.close();
			return false;
		}
		comfile >> n; --n;
		ncom[i] = n;
		coms_id.insert(n);
	}
	comfile.close();
	// Convert to community set format
	coms.Clear();
	coms.Resize(coms_id.size());
	std::vector< std::vector<long> > & cs = coms.GetCommunityList();
	std::map<long,long> idx; n=0;
	for (std::set<long>::iterator it = coms_id.begin(); it != coms_id.end(); ++it)
		idx[*it] = n++;
	for (long i=0; i<ncom.size(); ++i)
		cs[idx[ncom[i]]].push_back(i);
	return true;
}

} // namespace in
} // namespace io	
} // namespace mscd

