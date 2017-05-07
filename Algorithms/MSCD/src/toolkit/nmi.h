/*
 *  nmi.h
 *  analyse
 *
 *  Created by Erwan Le Martelot on 15/02/2012.
 *  Copyright 2012 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef MSCD_NMI_H_
#define MSCD_NMI_H_

// Includes
#include <cmath>
#include <vector>
#include <map>
#include <utility>
#include <limits>
#include "communities.h"

namespace toolkit {

/// Compute the NMI of the given community sets
double nmi(const mscd::ds::Communities & c1, const mscd::ds::Communities & c2, const long nb_nodes) {
	const std::vector< std::vector<long> > & coms1 = c1.GetCommunityList(),
										  & coms2 = c2.GetCommunityList();
	// Convert to node community list
	std::vector<long> nc1(nb_nodes, 0), nc2(nb_nodes, 0);
	for (long i=0, j; i<coms1.size(); ++i)
		for (j=0; j<coms1[i].size(); ++j)
			nc1[coms1[i][j]] = i;
	for (long i=0, j; i<coms2.size(); ++i)
		for (j=0; j<coms2[i].size(); ++j)
			nc2[coms2[i][j]] = i;
	// Build the confusion matrix using a map and compute the row and column sums
	std::map<std::pair<long,long>,long> cm;
	std::map<std::pair<long,long>,long>::iterator it;
	std::pair<long,long> p;
	std::vector<float> sumci(coms1.size(), 0.), sumcj(coms2.size(), 0.);
	for (long i=0; i<nb_nodes; ++i) {
		p.first = nc1[i];
		p.second = nc2[i];
		it = cm.find(p);
		if (it == cm.end()) cm[p] = 1;
		else ++it->second;
		++sumci[nc1[i]];
		++sumcj[nc2[i]];
    }
    // NMI terms computing
    double term1 = 0., term2 = 0., term3 = 0.;
	for (it = cm.begin(); it != cm.end(); ++it)
		term1 += it->second * log( (it->second*nb_nodes) / (sumci[it->first.first]*sumcj[it->first.second]) );
	for (long i=0; i<sumci.size(); ++i) term2 += sumci[i]*log(sumci[i]/nb_nodes);
    for (long i=0; i<sumcj.size(); ++i) term3 += sumcj[i]*log(sumcj[i]/nb_nodes);
	
    // Return NMI
    return (-2.*term1)/(term2+term3);
}

/// Count the number of common elements in two sorted lists
inline long intersect_size(const std::vector<long> & c1, const std::vector<long> & c2) {
	long counter = 0;
	long i=0, j=0;
	while ((i<c1.size()) && (j<c2.size())) {
		while ((i<c1.size()) && (c1[i] < c2[j])) ++i;
		if (i == c1.size()) break;
		while ((j<c2.size()) && (c2[j] < c1[i])) ++j;
		if (j == c2.size()) break;
		if (c1[i++] == c2[j]) { ++j; ++counter; }
	}
	return counter;
}
	
/// Compute the GNMI of the given community sets
double gnmi(const mscd::ds::Communities & c1, const mscd::ds::Communities & c2, const long nb_nodes) {
	const std::vector< std::vector<long> > & coms1 = c1.GetCommunityList(),
										  & coms2 = c2.GetCommunityList();
	double p;
	// H(X)
	std::vector<double> HX(coms1.size());
    for (long c=0; c<coms1.size(); ++c) {
        p = static_cast<double>(coms1[c].size())/nb_nodes;
		if ((p == 0.) || (p == 1.)) HX[c] = 0.;
		else HX[c] = -p*log(p) - (1.-p)*log(1.-p);
	}
    
    // H(Y)
	std::vector<double> HY(coms2.size());
    for (long c=0; c<coms2.size(); ++c) {
        p = static_cast<double>(coms2[c].size())/nb_nodes;
		if ((p == 0.) || (p == 1.)) HY[c] = 0.;
		else HY[c] = -p*log(p) - (1.-p)*log(1.-p);
	}
	
    // H(Xk|Yl)
	std::vector<long> nbtestc1(coms1.size(),0), nbtestc2(coms2.size(),0);
	std::vector<double> minHXY(coms1.size(), std::numeric_limits<double>::max()),
						minHYX(coms2.size(), std::numeric_limits<double>::max());
	double p11, p10, p01, p00, HXY, HYX, h11, h10, h01, h00;
	long l12, lc1, lc2;
    for (long c1=0, c2; c1<coms1.size(); ++c1) {
		lc1 = coms1[c1].size();
		for (c2=0; c2<coms2.size(); ++c2) {
			lc2 = coms2[c2].size();
			l12 = intersect_size(coms1[c1],coms2[c2]);
			p11 = static_cast<double>(l12)/nb_nodes;
			p10 = static_cast<double>(lc1 - l12)/nb_nodes;
			p01 = static_cast<double>(lc2 - l12)/nb_nodes;
			p00 = static_cast<double>(nb_nodes - (lc1 + lc2 - l12))/nb_nodes;
			if (p11 > 0.) h11 = - p11 * log(p11);
			else h11 = 0.;
			if (p10 > 0.) h10 = - p10 * log(p10);
			else h10 = 0.;
			if (p01 > 0.) h01 = - p01 * log(p01);
			else h01 = 0.;
			if (p00 > 0.) h00 = - p00 * log(p00);
			else h00 = 0.;
			HXY = (h11 + h10 + h01 + h00) - HY[c2];
			HYX = (h11 + h10 + h01 + h00) - HX[c1];
			if (h11 + h00 > h01 + h10) {
				++nbtestc1[c1];
				if (HXY < minHXY[c1]) minHXY[c1] = HXY;
				++nbtestc2[c2];
				if (HYX < minHYX[c2]) minHYX[c2] = HYX;
			}
		}
	}
    
    // H(Xk|Y) norm summed
    double HXYn = 0.;
    for (long c=0; c<coms1.size(); ++c) {
        if ((nbtestc1[c] > 0) && (HX[c] > 0.)) HXYn += minHXY[c]/HX[c];
        else ++HXYn;
	}
    HXYn /= coms1.size();
    
    // H(Yk|X) norm summed
	double HYXn = 0.;
    for (long c=0; c<coms2.size(); ++c) {
        if ((nbtestc2[c] > 0) && (HY[c] > 0.)) HYXn += minHYX[c]/HY[c];
        else ++HYXn;
	}
    HYXn /= coms2.size();
    
	// Return NMI
    return 1. - (HXYn + HYXn)/2.;
}

/// Compute the NMI or GNMI of the given community sets
inline double NMI(const mscd::ds::Communities & c1, const mscd::ds::Communities & c2, const long nb_nodes, bool gen = false) {
	if (!gen) return nmi(c1, c2, nb_nodes);
	return gnmi(c1, c2, nb_nodes);
}

} // namespace toolkit

#endif