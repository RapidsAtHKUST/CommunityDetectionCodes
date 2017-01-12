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

// lcelib/nets/SeedNet.H
// 
// Functions related to generating a seed network for growing network
// models. 

// generateSeedNetwork
// insertSeed
// kutsuu headeria BasicModels.H

// Note: you cannot #include Models.H if you #include SeedNet.H,
// otherwise seed options would be defined twice.
// The files in Models.H will be moved to other files:
//    ClearNet to NetExtras.H
//    CommunityNet to CommNet.H
//    ErdosRenyi to ErdosRenyi.H
//    bbvNet to BBV.H


// Authors: Riitta Toivonen
// 30 Dec 2005




/*

 Functions included 
 ------------------  

 generateSeedNetwork    (added Jan 4 2006, Riitta) 


 Other
 -----
 SeedArgs
 SeedType



Feel free to add whatever useful bits you have that belong to this category.
A reasonable amount of testing is expected.

Suggestions, corrections etc. also welcomed. 

Usage examples for nontrivial functions should be found in 
lcelib/nets/Examples/

 */ 



#ifndef SEEDNET_H
#define SEEDNET_H
#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include "../Dijkstrator.H"
#include <cassert>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>


#include "ErdosRenyi.H" // for ER seed
//#include "SimpleNets.H" // for clique, chain, and ring 




// Seed network options:
// --------------------
enum SeedType {RANDOM, CLIQUE, CHAIN, RING};
  // RANDOM: Erdös-Renyi random network
  // CLIQUE: fully connected network

SeedType seedType = CLIQUE; // Defaults to fully connected seed.





struct SeedArgs {
  size_t netSize;
  size_t seedSize;
  SeedType seedType;
  float k_ave;
};










/* Note: generateSeedNetwork does not use SimpleNets.H for generating
  a clique, chain, or ring, since it was written before SimpleNets.H. */

/* function generateSeedNetwork() */
template<typename NetType, typename Generator>
void generateSeedNetwork(NetType& theNet, struct SeedArgs & args, Generator & generator) {
  
  
#ifdef DEBUG
  std::cerr << "-------------------------------------\n"
            << "Generating a seed network of " << args.seedSize << " nodes.\n";
#endif // DEBUG 

  /* generate the seed network */
  size_t i=0;
  
  if (args.seedType == CLIQUE) {
#ifdef DEBUG
    std::cerr << "Links: \n";
#endif // DEBUG 
    /* Fully connected seed: */
    for (; i<args.seedSize; ++i) {
      for (size_t j=0; j<i; ++j) {
	//        theNet[i][j]=args.w0;
        theNet[i][j]=1;
#ifdef DEBUG
	std::cerr << "\t" << i << " to " << j << "\n";
#endif // DEBUG 
      }
    }
  } // end of seedType == CLIQUE
  else if (args.seedType == RANDOM) {
    
    /* Generate ER seed: */
    NetType seedNet(args.seedSize);
    size_t trials=0;
    size_t maxtrials=100;
    do {
      if (trials++ > maxtrials) {
	std::cerr << "\nCouldn't generate a connected Erdos-Renyi seed network in "
                  << maxtrials << " trials, giving up \nand aborting. Please try using a higher average degree (over 1.2). \n\n\n";
        exit(1);
      }
      //      ErdosRenyi(seedNet,args.seedSize,args.k_ave,generator,args.w0);
      ErdosRenyi(seedNet,args.seedSize,args.k_ave,generator);
      std::cerr << "Generated a seed network, checking for connectivity...  ";
      if ( !ConnectivityCheck(seedNet) )
        std::cerr << "failed.\n";
      else
        std::cerr << "OK!\n";
    }
    while ( !ConnectivityCheck(seedNet) );
                                 
    if  ( ConnectivityCheck(seedNet) )
      std::cerr << "Found a connected seed after " << trials << " trial(s). \n\n";
    
    
    /* Copy seedNet into theNet */
    //    std::cerr << "copying ER seed net into theNet... \n\n";                                                                  
#ifdef DEBUG
    std::cerr << "Links: \n";
#endif // DEBUG                                                                                                                       
    for (; i<args.seedSize; ++i) {
      for (typename NetType::const_edge_iterator j=seedNet(i).begin();
           !j.finished();
           ++j) {
        if (i<*j) {
          theNet[i][*j]=seedNet(i)[*j];
#ifdef DEBUG
	  std::cerr << "\t" << i << " to " << *j << "\n";
#endif // DEBUG                                                                                                                       
        }
      }
    }
  }  // end of seedType == RANDOM
  else if (args.seedType == CHAIN) {
    /* A chain of nodes : */  
    for (size_t i=0; i<args.seedSize-1; ++i) {
      theNet[i][i+1]=1;
      //      theNet[i][i+1]=args.w0;
#ifdef DEBUG // For debugging, print out what we are doing
      std::cerr << "Linked " << "\t" << i << " to " << i+1 << "\n";
#endif // DEBUG 
    }
  }   // end of seedType == CHAIN 
  else if (args.seedType == RING) {
    /* A ring of nodes : */  
    for (size_t i=0; i<args.seedSize-1; ++i) {
      //      theNet[i][i+1]=args.w0;
      theNet[i][i+1]=1;
#ifdef DEBUG // For debugging, print out what we are doing
      std::cerr << "Linked " << "\t" << i << " to " << i+1 << "\n";
#endif // DEBUG 
    }
    //    theNet[theNet.size()-1][0]=args.w0; // the edge that will make the loop
    theNet[theNet.size()-1][0]=1; // the edge that will make the loop
#ifdef DEBUG // For debugging, print out what we are doing
    std::cerr << "Linked " << "\t" << theNet.size()-1 << " to " << 0 << "\n";
#endif // DEBUG 
  } // end of seedType == RING
  else {
    std::cerr << "The seed options are 'clique', 'random', 'chain' and 'ring'. Please use one of them.\n";
    exit(1);
  }
  
}
// <--- generateSeedNetwork
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 








#endif //~ SEEDNET_H
