/*
 *  mscd_rn.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 06/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_RN_H_
#define MSCD_RN_H_

// Includes
#include "mscd_algorithm.h"

namespace mscd {
namespace alg {
	
/** Multi-scale community detection using Ronhovde and Nussinov's method.
 * This implementation ignores edges weight and requires no self-loop.
 */
class MSCD_RN: public MSCDAlgorithm {
	
public:
	
	/// Return the name of the algorithm
	virtual std::string GetName() const { return "RN"; }
	
	/// Run the algorithm
	virtual bool Run(const ds::Graph &,
					 const std::vector<double> &,
					 std::vector<ds::Communities> &,
					 std::vector<double> &,
					 const ds::Communities &);
	
private:
	
	/// Compute the criterion value
	inline preal computeQ(const std::vector<preal> & twi,
						 std::vector<std::vector<long> > & coms,
						 const double p) const {
		double Q = 0.;
		for (long i=0; i<twi.size(); ++i)
			Q += twi[i] - p*(static_cast<double>(coms[i].size()*(coms[i].size()-1))*0.5 - twi[i]);
		return Q;
	}

#ifdef __DEBUG__
	double DbCheckQ(const ds::Graph &, const std::vector<long> &, const double) const;
#endif

};
	
} // namespace alg
} // namespace mscd

#endif
