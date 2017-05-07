/*
 *  mscd_algorithm.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 28/11/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_ALGORITHM_H_
#define MSCD_ALGORITHM_H_

// Includes
#include "config.h"
#include <cstdio>
#include <string>
#include <sstream>
#include <vector>
#include "communities.h"
#include "community_writer.h"

// Error threshold for checking Q correctness
#ifdef __DEBUG__
#define ERROR_Q_DIFF 0.00001
#endif

// Minimum value dQ must have to be considered
#define DQ_ZERO 0.00001

// Debug printing macro (only prints in debug mode)
#ifdef __DEBUG__
#define PrintDb(...) fprintf(stdout, __VA_ARGS__)
#else
#define PrintDb(...)
#endif

namespace mscd {

// Forward declarations
namespace ds{
class Graph;
}	

namespace alg {
	
/** Multi-scale community detection algorithm abstract interface.
 *	To be implemented for specific algorithms. 
 */
class MSCDAlgorithm {
	
public:

	/// Default constructor
	MSCDAlgorithm() : verbose(false), community_writer(NULL) {}
	
	/// Destructor
	virtual ~MSCDAlgorithm() {}
	
	/// Return the name of the algorithm
	virtual std::string GetName() const = 0;
	
	/// Run the algorithm
	virtual bool Run(const ds::Graph &,
					 const std::vector<double> &,
					 std::vector<ds::Communities> &,
					 std::vector<double> &,
					 const ds::Communities & = ds::Communities()) = 0;
	
	/// Set extra parameters
	virtual bool SetExtraParams(const std::vector<double> &) { return false; }
	
	/// Set verbose mode
	inline void SetVerbose(const bool value) { this->verbose = value; }
	
	/// True if verbose mode is on
	inline bool IsVerbose() const { return this->verbose; }
	
	/// Turn on/off the dump of communities (a dumped community is written to disk to free the RAM)
	inline void SetCommunityWriter(io::out::CommunityWriter * writer) {
		this->community_writer = writer;
	}
	
	/// True if communities are dumped on the disk
	inline const io::out::CommunityWriter * GetCommunityWriter() const { return this->community_writer; }
	
protected:
	
	/// Verbose mode
	bool verbose;
	
	/// Optional community writer to dump communities to disk
	io::out::CommunityWriter * community_writer;
	
	/// Store the given community into the given community placing the data either in memory or on disk
	inline void storeCommunities(ds::Communities & src, ds::Communities & dst) const {
        if (this->community_writer == NULL) {
			dst.GetCommunityList() = src.GetCommunityList();
			return;
		}
		std::stringstream ss;
		ss << "c_" << src.GetCommunityList().size() << "_" << reinterpret_cast<long>(&dst) << "." << this->community_writer->GetName();
		if (!this->community_writer->ToFile(ss.str(), src)) {
			fprintf(stderr, "Error: Cannot dump communities -> Keeping them in memory\n");
			dst.GetCommunityList() = src.GetCommunityList();
		}
		else {
			dst.Clear();
			dst.SetComFile(ss.str());
		}
	}
	
};

} // namespace alg
} // namespace mscd

#endif
