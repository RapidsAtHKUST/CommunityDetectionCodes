/*
lcelib, A high performance library for complex networks research
Copyright (C) 2011  Aalto University

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

// Distributions.H

/*
 Functions included:  
 -------------------
 outputDegrees (Nov 1 2005)
 outputDistributions1 (Nov 9 2005) 
 outputDistributions2 (Nov 2005)

 (for details, see comments above each function)

 Authors: Riitta Toivonen



outputDistributions1 and 2 write out n(k), c(k) and k_nn(k)
binned with respect to degree k, in two different formats. 

Hopefully someone will write more functions for calculating 
different distributions. Antti's binning code should come in handy. 
                                                                                                              
Feel free to add whatever useful bits you have that 'belong to this category'!
(a reasonable amount of testing expected) 

Suggestions, corrections etc. also welcomed.

Usage examples for nontrivial functions should be found in
lcelib/nets/Examples/

*/



#ifndef DISTRIBUTIONS_H
#define DISTRIBUTIONS_H


#include <vector>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include "../Containers.H"
#include "../Nets.H"
#include "../Randgens.H"












/* function outputDegrees(NetType& theNet, size_t kmax) */

template<typename NetType>
void outputDegrees(NetType& theNet, size_t kmax) {
  
  std::vector<size_t> n_k;        // number of nodes of degree k   
  for(size_t i=0; i<kmax+1; ++i)  n_k.push_back(0);
  
  // Go through each node in the network: 
  size_t lostcounter=0;  // Count the number of nodes lost because their degree exceeded kmax
  size_t true_kmax=0;
  for (size_t i=0; i<theNet.size(); ++i) {
    size_t curr_deg=theNet(i).size(); // Read the degree of node i
    // We need max_deg+1 places, since we don't use the '0:th' vector element
    if (n_k.size()<(curr_deg+1) ) {  // If the container is too small... 
      n_k.resize(curr_deg+1,0); // ...enlarge it with zeroes
    }
    if (curr_deg>kmax) {
      if (curr_deg>true_kmax) 
	true_kmax=curr_deg;
      lostcounter++;
    }
    n_k[curr_deg]+=1; 
  }
  if (true_kmax>kmax) {
    std::cerr << "\n\nNote: There were nodes of degree up to  " << true_kmax << ", larger than the given kmax = " << kmax << ".\nThey will be lost in the outputted data.\n";
    std::cerr << "\n * * Lost information on " << lostcounter << " nodes because their degree exceeded  kmax = " << kmax << ".\n";  
  }
  /* The container has one extra element, since we do not use the zeroth element. 
     Hence, iterate from 1 to n_k.size()-1. Notice: any degree larger than kmax 
     is not printed out here. It is the user's responsibility to check that this 
     does not cause too much trouble. */
  for (size_t i=1; i<kmax+1; ++i)  std::cout <<  n_k[i]  << " ";
  std::cout << "\n";
} 
// <--- outputDegrees
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

















/*  void outputDistributions1(NetType& theNet, size_t kmax)
       Print out the distributions: 
         On one line, n(k)
         On the next line, csum(k)=trisumx2(k)/k/(k-1) 
         On the third line, knnsum(k)=nndegsum(k)/k 
       Each iteration prints out three rows to stdout.  
       The length of each line is kmax.
*/


template<typename NetType>
void outputDistributions1(NetType& theNet, size_t kmax) {
  /* Containers to hold degrees, number of triangles and nearest neighbor degree sums.*/

  std::vector<size_t> n_k;        // number of nodes of degree k 
  std::vector<float> trisumx2_k;  // twice the number of triangles around nodes of degree k 
  std::vector<float> nndegsum_k;  // degree sum of nearest neighbors of nodes of degree k 

  // Initialize the vectors manually. It seems to be necessary to avoid absurd results. Don't know why.
  for(size_t i=0; i<kmax+1; ++i)  n_k.push_back(0);
  for(size_t i=0; i<kmax+1; ++i)  trisumx2_k.push_back(0);
  for(size_t i=0; i<kmax+1; ++i)  nndegsum_k.push_back(0);
  

  /* Degrees, clustering and nearest neighbor degree (in the same loop for efficiency) */

  // Go through each node in the network:
  for (size_t i=0; i<theNet.size(); ++i) {
    size_t trianglesum=0;
    size_t degreesum=0;
    
    // Go through the neighbors of node i:
    for (typename NetType::const_edge_iterator j=theNet(i).begin();
	 !j.finished();
	 ++j) {
      degreesum+=theNet(*j).size();       // ... sum up their degrees... 
      // ...and check if some of their neighbors are also neighbors of node i.    
      // If yes, increase the triangle counter. 
      for (typename NetType::const_edge_iterator k=theNet(*j).begin();
	   !k.finished();
	   ++k) {
	if (theNet(*k)[i] != 0) {
	  trianglesum++;
	  //    trianglesum+=theNet(*k)[i];  // for the weighted version 
	}
      }
    }
    
    size_t curr_deg=theNet(i).size(); // Read the degree of node i  
   
    // We need max_deg+1 places, since we don't use the '0:th' vector element    
    if (n_k.size()<(curr_deg+1) )   // If the container is too small... 
      n_k.resize(curr_deg+1,0); // ...enlarge it with zeroes 
    
    if (trisumx2_k.size()<(curr_deg+1)) // If the container is too small...  
      trisumx2_k.resize(curr_deg+1,0); // ...enlarge it with zeroes  
    
    if (nndegsum_k.size()<(curr_deg+1)) // If the container is too small... 
      nndegsum_k.resize(curr_deg+1,0); // ...enlarge it with zeroes  
    
    n_k[curr_deg]+=1;
    trisumx2_k[curr_deg]+=(float) trianglesum;
    nndegsum_k[curr_deg]+=(float) degreesum;
  }
  
  
  /* The container has one extra element, since we do not use the zeroth element.  
     Hence, iterate from 1 to n_k.size()-1. 
     
     We assume all vectors are of the same length - and they should be:   
    if there is a node with degree k, it has a clustering coefficient and a nn-degree
     (except clustering is not defined for k=1, and it becomes NaN, not a number). */
  
  for (size_t i=1; i<kmax+1; ++i)  std::cout << " " <<  n_k[i]  ;
  std::cout << "\n";
  
  //    for (size_t i=1; i<kmax; ++i)  std::cout <<  ((float) trisumx2_k[i]) / i / (i-1) / n_k[i]   << " ";                         
  for (size_t i=1; i<kmax+1; ++i)  std::cout  << " " <<   trisumx2_k[i] / i / (i-1)  ;
  std::cout << "\n";
  
  for (size_t i=1; i<kmax+1; ++i)  std::cout   << " " <<   nndegsum_k[i] / i;
  std::cout << "\n";
  
  /* Sum up the distributions from each iteration in Matlab and divide by niter. */
  //  nk should be summed over all iterations, divided by niter and normalized for
  //  plotting the degree distribution. 
  //  csum=trisumx2/i/(i-1) should be summed over all iterations and divided by nksum. 
  //  knnsum=nndegsum/i should be summed over all iterations and divided by nksum. 
  //  Plot with respect to k.
}
// <--- outputDistributions1
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 











/*  void outputDistributions2(NetType& theNet)
       Print out the distributions: 
         In the first column, degrees of each node 
         In the next column, trix2/k/(k-1) for each node 
         In the third column, knn=nndegsum/k for each node 
      Each column will have N elements. 
*/


template<typename NetType>
void outputDistributions2(NetType& theNet) {
  
  
  /* Degrees, clustering and nearest neighbor degree (in the same loop for efficiency) */
  // Go through each node in the network:
  for (size_t i=0; i<theNet.size(); ++i) {
    size_t trix2=0;     // twice the number of triangles around the node 
    size_t nndegsum=0;  // degree sum of nearest neighbors of the node 
    
    // Go through the neighbors of node i:
    for (typename NetType::const_edge_iterator j=theNet(i).begin();
	 !j.finished();
	 ++j) {
      nndegsum+=theNet(*j).size();       // ... sum up their degrees... 
      // ...and check if some of their neighbors are also neighbors of node i.    
      // If yes, increase the triangle counter. 
      for (typename NetType::const_edge_iterator k=theNet(*j).begin();
	   !k.finished();
	   ++k) {
	if (theNet(*k)[i] != 0) {
	  trix2++;
	  //    trix2+=theNet(*k)[i];  // for the weighted version 
	}
      }
    } 
    /* output degree, clustering and average nearest neigh degree for each node */    
    size_t curr_deg=theNet(i).size(); // Read the degree of node i  
    std::cout << curr_deg << "\t" << ((float) trix2) / curr_deg / (curr_deg - 1) 
	      << "\t" <<  ((float) nndegsum ) / curr_deg << "\n";
  } // end of loop for going through each node in the network
  

  /* Write values for each network directly below the previous. 
     Use bindata to bin the values for plotting with respect to k.  */
}
// <--- outputDistributions2
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 












#endif // DISTRIBUTIONS_H
