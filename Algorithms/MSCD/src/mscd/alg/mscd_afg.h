/*
 *  mscd_afg.h
 *  MSCD
 *
 *  Created by Erwan Le Martelot on 05/12/2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_AFG_H_
#define MSCD_AFG_H_

// Includes
#include "mscd_algorithm.h"

namespace mscd {
namespace alg {
	
/** Multi-scale community detection using Arenas, Fernandez and Gomez's method.
 * The parameter taken is the number of self loops that will be added.
 * In the calculations each self-loop will be counted twice.
 */
class MSCD_AFG: public MSCDAlgorithm {
	
public:
	
	/// Return the name of the algorithm
	virtual std::string GetName() const { return "AFG"; }
	
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
						   const double i2m) const {
		double Q = 0.;
		for (long j=0; j<tw.size(); ++j)
			Q += twi[j] - tw[j]*tw[j]*i2m;
		return Q*i2m;
	}

#ifdef __DEBUG__
	double DbCheckQ(const ds::Graph &, const std::vector<long> &, const double) const;
#endif
		
};
		
} // namespace alg
} // namespace mscd

#endif
