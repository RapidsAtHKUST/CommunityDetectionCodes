/*
 *  communities.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 19/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_COMMUNITIES_H_
#define MSCD_COMMUNITIES_H_

// Includes
#include "config.h"
#include <vector>
#include <string>
#include <sstream>
#include <assert.h>

namespace mscd {
namespace ds {	

/** Communities class.
 *	Each community is list of nodes it contains. 
 */
class Communities {
	
public:
	
	/// Default constructor
	Communities() : comfile("") {}
	
	/// Constructor with size initialisation of the number of communities
	Communities(const long nb_coms) : community_list(nb_coms), comfile("") {}
	
	/// Create a new community
	inline std::vector<long> & CreateCommunity() {
		this->community_list.push_back(std::vector<long>());
		return this->community_list.back();
	}
	
	/// Return the list of communities
	inline const std::vector<std::vector<long> > & GetCommunityList() const {
		return this->community_list;
	}
	
	/// Return the list of communities
	inline std::vector<std::vector<long> > & GetCommunityList() {
		return this->community_list;
	}
	
	/// Return the requested community (0-based index)
	inline const std::vector<long> & GetCommunity(const long index) const {
		assert((index>=0) && (index<this->community_list.size()));
		return this->community_list[index];
	}
	
	/// Return the requested community (0-based index)
	inline std::vector<long> & GetCommunity(const long index) {
		assert((index>=0) && (index<this->community_list.size()));
		return this->community_list[index];
	}
	
	/// Return the number of communities
	inline long GetNbCommunities() const {
		return this->community_list.size();
	}
	
	/// Clear all data
	inline void Clear() {
		this->community_list.clear();
		this->comfile = "";
	}
	
	/// Resize the community list
	inline void Resize(const long value) {
		this->community_list.resize(value);
	}
	
	/// Swap the data from the given communities to self
	inline void Swap(Communities & c) {
		this->community_list.swap(c.community_list);
	}
	
	/// True if the community contains no data (no loaded data)
	inline bool IsEmpty() const { return this->community_list.empty(); }
	
	/// Return the community file when dumped
	inline const std::string & GetComFile() const { return this->comfile; }
	
	/// Set the community file name
	inline void SetComFile(const std::string & file) { this->comfile = file; }
	
private:
	
	/// List of communities
	std::vector<std::vector<long> > community_list;
	
	/// Community file on disk when memory is dumped (communities can be reloaded using community readers)
	std::string comfile;
	
};

} // namespace ds
} // namespace mscd

#endif

