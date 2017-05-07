/*
 *  mscd_rb.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 28/11/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_RB_H_
#define MSCD_RB_H_

// Includes
#include "mscd_algorithm.h"

namespace mscd {
namespace alg {
	
/** Multi-scale community detection using Reichardt and Bornholdt's method.
 */
class MSCD_RB: public MSCDAlgorithm {
	
public:
	
	/// Return the name of the algorithm
	virtual std::string GetName() const { return "RB"; }
	
	/// Run the algorithm
	virtual bool Run(const ds::Graph &,
					 const std::vector<double> &,
					 std::vector<ds::Communities> &,
					 std::vector<double> &,
					 const ds::Communities &);

private:
	
	/// Compute the criterion value
	inline double computeQ(const std::vector<preal> & tw,
						   const std::vector<preal> & twi,
						   const double i2m,
						   const double p) const {
		double Q = 0.;
		for (long j=0; j<tw.size(); ++j)
			Q += twi[j] - p*tw[j]*tw[j]*i2m;
		return Q*i2m;
	}
	
#ifdef __DEBUG__
	double DbCheckQ(const ds::Graph &, const std::vector<long> &, const double) const;
#endif
		
};
	
} // namespace alg
} // namespace mscd
	
#endif
