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

// lcelib/nets/models/HolmeKim.H
// 
// Functions related to the Holme-Kim model, which consists of 
// preferential attachment and triangle formation steps. 
// (From the article P. Holme and B. J. Kim: Phys. Rev. E 65, 026107, 2002).
// Parameters:   N, randseed, m, pt, seedSize, seedType, optionally k_ave for Erdos-Renyi seed

/* Algorithm: 

Start with a seed network 
   (in the article, m0 disconnected nodes were used, but since the
   preferential picking function in Jorkki's library is confused by
   nodes with zero degree, we start with a connected seed
   network. Choose the seed from the category found in
   lcelib/nets/models/seedNet.H.)

Add a new node with m links (m fixed): 
  Choose first target by preferential attachment (PA)
  then, for the other m-1 links, 
  with probability pt
    do a triangle formation step if possible
  else 
     do a PA step again. 
  Connect to the chosen targets. 

Parameters:   N, randseed, m, pt, seedSize, seedType, optionally k_ave for Erdos-Renyi seed

*/


// Usage example in        lcelib/nets/Examples/HolmeKim.cpp  

/* Tested as follows: 

   - Checked by hand that the network seems to grow correctly for 
     the first ~10 added nodes. 
   - Degree distribution a power law with exponent ~-3, as
     reported in the paper 
   - Average  clustering coefficients for a couple of parameter sets 
     match values reported in the paper (read from the figure, error +-0.01): 

When m=3:                          average           average
     mt <==>  pt=mt/(m-1)          clustering        clustering 
                                   expected to be    found to be

     1.2      0.6                  0.32              0.3277   (OK!)
     1.8      0.9                  0.54              0.5353   (OK!)

Data for calculating the degree exponent and average clustering were
averaged over 10 networks of size N=10^6.

File used for testing   lcelib/nets/Examples/HolmeKimTest.cpp  
(Memo for Riitta: test details (not publicly available) in  /u/rtoivone/HolmeKim/test/info.txt )
*/


// Author: Riitta Toivonen
// 2 Jan 2006






#ifndef HOLMEKIM_H
#define HOLMEKIM_H
#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include "SeedNet.H"
#include <cassert>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <ctime>








/* The struct into which command line arguments will be read */
struct HolmeKimArgs {
  size_t netSize;
  int randseed;
  size_t m;
  float pt; 
  size_t seedSize;
  SeedType seedType;
  float k_ave;
};






// readHolmeKimArgs --->
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void readHolmeKimArgs(struct HolmeKimArgs & args, int argc, char** argv) {
  size_t bookmark=1;
  if ( (size_t) argc < (6+1)) {  // 6 arguments required: N, randseed, m, pt, seedSize, seedType
    std::cerr << "\nPlease specify arguments: \n\tN, randseed, m, pt, seedSize, seedType, optionally k_ave for Erdos-Renyi seed\n\n"; exit(1);
  }  
  args.netSize=atoi(argv[bookmark]);  bookmark++;
  args.randseed=atoi(argv[bookmark]);  bookmark++;
  args.m=atoi(argv[bookmark]); bookmark++;
  args.pt=atof(argv[bookmark]); bookmark++;



  /* sometime in the future, the part below could  be separated to readSeedNetArgs */
  args.seedSize=atoi(argv[bookmark]); bookmark++;
  if (args.seedSize>args.netSize) {
    std::cerr << "\n\nSeed size should not exceed network size. \n"; 
    std::cerr << "Currently seed size="<< args.seedSize 
	      << " and network size="<< args.netSize << ".\n";
    exit(1);
  }  
  if (std::string(argv[bookmark]) == std::string("random")) {
    args.seedType=RANDOM;
  }
  else if (std::string(argv[bookmark]) == std::string("clique")) {
    args.seedType=CLIQUE;
  }
  else if (std::string(argv[bookmark]) == std::string("ring")) {
    args.seedType=RING;
  }
  else if (std::string(argv[bookmark]) == std::string("chain")) {
    args.seedType=CHAIN;
  }
  else {
    args.seedType=RANDOM; 
    std::cerr << "Unknown seed type - using Erdos-Renyi network as seed" << std::endl;
  } 
  bookmark++;
  // If using random seed, the average degree is read as input or the default is used:
  args.k_ave=2; /* (if not using random seed, this is just a dummy variable) */
  if (args.seedType == RANDOM) {
    if ( (size_t) argc >= bookmark+1) 
      args.k_ave=atof(argv[bookmark]); // read k_ave if it is given, else use default as below
    else {
      std::cerr << "Average degree not given for the random seed, using k_ave=2" << std::endl << std::endl;  
    }
  }
  bookmark++;
}  
// <--- readHolmeKimArgs
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
























// outputHolmeKimArgs --->
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
void outputHolmeKimArgs(struct HolmeKimArgs & args) {
  std::cerr << "Parameters given for simulating Holme-Kim network:\n";
  std::cerr << "(arguments required: N, randseed, m, pt, seedSize, seedType)\n\n";
  std::cerr << "Network size:\t\t\t\t\t" << args.netSize << "\n"; 
  std::cerr << "m (number of links of added per time step):\t" << args.m << "\n"; 
  std::cerr << "p (probability of triangle formation step):\t" << (float) args.pt << "\n"; 
  std::cerr << "Seed size:\t\t\t\t\t" << args.seedSize << "\n"; 
  if (args.seedType == RANDOM)
    std::cerr << "Seed type:\t" << "random seed (Erdos-Renyi) with average degree " << args.k_ave << std::endl << "\t\tNote: Any disconnected random seed is discarded, \n\t\tand a new one is generated until a connected \n\t\tnetwork is found.\n" << std::endl;
  else if (args.seedType == CLIQUE) 
    std::cerr << "Seed type:\t" << "fully connected" << std::endl;    
  else if (args.seedType == RING) 
    std::cerr << "Seed type:\t" << "ring" << std::endl;
  else if (args.seedType == CHAIN) 
    std::cerr << "Seed type:\t" << "chain" << std::endl;

  std::cerr << "Random number generator seed (integer): \t" << args.randseed << "\n";
  std::cerr << "\n\n";
} 
// <--- outputHolmeKimArgs
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 












































// HolmeKim ---> 
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

/* function HolmeKim(NetType& theNet, struct HolmeKimArgs & args, Generator & generator) */

template<typename NetType, typename Generator>
void HolmeKim(NetType& theNet, struct HolmeKimArgs & args, Generator & generator) {  
  
  ClearNet(theNet,args.netSize);   /* make sure there are no edges present to start with */
  
  struct SeedArgs seedArgs;
  seedArgs.netSize=args.netSize; 
  seedArgs.seedSize=args.seedSize; 
  seedArgs.seedType=args.seedType; 
  seedArgs.k_ave=args.k_ave; 
  generateSeedNetwork(theNet,seedArgs,generator); /*generates a seed network and copies the edges into theNet*/
  
  
  
  std::vector<size_t> PAVector;
  
  /* Initialize PAVector, which will be used to choose nodes linearly with respect to their degree, to include the seed network */
  for (size_t k=0; k<args.seedSize; ++k) {
    for (size_t j=0; j<theNet(k).size(); ++j)
      PAVector.push_back(k);
  }
#ifdef DEBUG  
  std::cerr << "PAVector contains the elements: ";
  for (size_t q=0; q<PAVector.size(); ++q) {
    assert(q<PAVector.size());
    std::cerr << " " <<  PAVector.at(q) ; 
  }
  std::cerr << "\n\n"; 
#endif // DEBUG 
  
  size_t tosscounter=0;
  size_t TFtosscounter=0;
      



  //********************************************************************
  /* Growth loop. Adding node i. */
    std::cerr << "Growing the network... ";

  for (size_t i=args.seedSize; i<args.netSize; ++i) {
#ifdef DEBUG
    std::cerr << "***********************************************************************\n";
    std::cerr << "\t\tAdding new node " << i << " to the network.\n\n";
#endif // DEBUG 
    

    Set<size_t> targets; // Targets to link to 
    size_t PANode,TFNode;
    // Pick the first target preferentially
    do {
      PANode=PAVector[generator.next(PAVector.size())]; 
    } while (targets.contains(PANode));
    targets.put(PANode); 

#ifdef DEBUG
    std::cerr<<"Chose initial PA target " << PANode <<"\n\n"; 
#endif // DEBUG    
    

    
    //********************************************************************





for (size_t j=0; j<(args.m-1); ++j) {
  float toss=generator.nextNormed(); // a random number uniformly from [0,1) 
  tosscounter++; // tossed altogether this many times
  
  //-------------------------------------------------------------------------------
 // With probability pt, choose a neighbor of previously picked PANode if possible 
  if (toss<args.pt) { 
    TFtosscounter++; // tossed a TF step this many times
    assert (! theNet(PANode).size()==0 );  // make sure PANode has neighbors (not disconnected)
    
#ifdef DEBUG
    std::cerr<<"\tStarting to do a TF step" << "\n"; 
    std::cerr<<"\tChecking if any  unpicked neighbor exists... ";
#endif // DEBUG  
    
    // It is slow to go through all the neighbors like this, but the
    // code is easier to read and maintain than if it is optimized for
    // speed.
    std::vector<size_t> availableNeighs;
    for  (typename NetType::const_edge_iterator k=theNet(PANode).begin(); !k.finished(); ++k) {
      if ( !targets.contains(*k) )  /* if this neighbor was not already picked during this time step */
	availableNeighs.push_back(*k);
    } 
    assert( availableNeighs.size() <= theNet(PANode).size() ); 
#ifdef DEBUG
    std::cerr<<"\t" << availableNeighs.size()  << " neighbors available.\n"; 
#endif // DEBUG  

    // if yes, choose one of them
    if (availableNeighs.size()>0) {
      TFNode=availableNeighs[generator.next(availableNeighs.size())]; 
      assert(theNet(PANode).contains(TFNode)); // make sure that TFNode is a neighbor of PANode
      targets.put(TFNode);
#ifdef DEBUG
      std::cerr<<"\tChose TF target " << TFNode <<"\n\n"; 
#endif // DEBUG   
    }
    else { 	// Pick a target preferentially
      do {
	PANode=PAVector[generator.next(PAVector.size())]; 
      } while (targets.contains(PANode));	
      targets.put(PANode); 
#ifdef DEBUG
      std::cerr<<"Chose PA target " << PANode <<" since TF step was unsuccessful.\n"; 
#endif // DEBUG  
    }
  }   // end of choice "try to do a TF step" 
  
  
  
  //-------------------------------------------------------------------------------
  // With the complementary probability, do a PA step.     
  if (toss>=args.pt) {  
    // Pick a target preferentially
    do {
      PANode=PAVector[generator.next(PAVector.size())]; 
    } while (targets.contains(PANode));	
    targets.put(PANode); 
#ifdef DEBUG
    std::cerr<<"Chose PA target (with probability 1-p) " << PANode <<"\n\n"; 
#endif // DEBUG  
  }
}

// By now, we should have picked all m targets.
 assert( targets.size() == args.m);
 
 
 //********************************************************************
 /* Link the new node to chosen targets  */
    for (Set<size_t>::iterator k=targets.begin(); !k.finished(); ++k) {
      theNet[i][*k]=1; 
      PAVector.push_back(i); // Add the link's "i-end" to PAVector
      PAVector.push_back(*k); // Add the link's target end to PAVector
#ifdef DEBUG
      std::cerr << "Linking the new node " << i << " to node "<< *k << ".\n";
#endif // DEBUG      
    }  // end of loop for making the links

    
#ifndef NDEBUG
    //    std::cerr << "Checking that the number of items in PAVector equals degree sum.\n"; //REMOVE
    /* sum up the degrees in the network */
    size_t degreesum=0;
    for (size_t j=0; j<i+1; ++j) { // i+1 because we have already added node i
      degreesum+=theNet(j).size(); 
    } 
    /* number of items in PAVector should be the same */ 
    assert(degreesum==PAVector.size());//REMOVE
#endif // NDEBUG


  } // end of loop for adding each new node i to the network



 //-------------------------------------------------------------------
 //-------------------------------------------------------------------
  std::cerr << "Generated Holme-Kim network of size N = " << theNet.size() << "\n\n";
  
  // Now the network should be of desired size: 
  assert (theNet.size() == args.netSize);
    
  // And we should have thrown the dice m-1 times for each added node:
  assert (tosscounter == (args.m-1)*(args.netSize-args.seedSize) );

  // And we should have tried a TF step with probability args.pt every
  // time we tossed (this can't be asserted, really - check manually)
  //#ifdef DEBUG
  std::cerr << "Threw the dice " << tosscounter  <<  " times (should be (m-1)*(netSize-seedSize) = " 
	    << (args.m-1)*(args.netSize-args.seedSize) << "), \nof which " 
	    << TFtosscounter << " times got a value smaller than pt and thus tried to do a TF step - \n" 
	    << (float) TFtosscounter / (float) tosscounter  
	    << " fraction of the time. The parameter pt given as input was \n" << args.pt 
	    << ". These should match.\n\n";
  //#endif // DEBUG 


}
// <--- HolmeKim 
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 














#endif //~ HOLMEKIM_H 
