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

// CommunityNet.H

// Functions related to the LCE community net model, 
// a growing network model which consists of random attachment and connections to the neighborhood. 
// (From the article A model for social networks (LCE)  2006)
// Parameters:   N randseed Nrandmax [cumulative probabilities of Nrand]  Nwalksmax [cumulative probabilities of Nwalks]   delta w0 seedSize seedType optional:k_ave



/* Algorithm: 
Start with a seed network 

Add a new node until network has grown to desired size:
  Choose on average m_r initial contacts uniformly randomly 
    Choose on average m_s neighbors of the initial contacts as secondary contacts if possible
  Link the new node to the initial and secondary contacts. 

*/ 





/*
 Models included:  
 ---------------

   

   * * Use this one ---> 
  communityNetCustomCdfsFaster (Nov 9 2005)  (available since 2006 Jan 12)

    A version where the distributions for the number of initial and
    secondary connections are given in cumulative form. 

    The function is labeled 'Faster' since it includes an optimization
    trick which is important for longer walks but unnecessary for
    walks of length one: when making secondary connections, the
    algoritm does not check for immediately check for saturation but
    instead first attempts to find an available node, and only if
    unsuccessful, checks whether the neighborhood is saturated. When
    the walks are of length one, it only makes the algorithm roughly
    15-25 percent faster than without the optimization, which makes no
    difference, since time required for network generation is
    negligible compared to e.g. calculating the distributions. Anyway,
    I made 'Faster' available on cvs instead of the previous version,
    since this function is otherwise more cleanly written, and in any
    case the optimization does no harm.

    Usage example in        lcelib/nets/Examples/communityNet.cpp  

   * * *


 CommunityNet (Sept 30 2005)   
   The old version with geometric distributions for initial and secondary 
   connections, resulting in high saturation
  (No usage example available for CommunityNet as of 10 Jan 2006)



 Other:
 -----
 struct CommNetArgs
 readCommNetArgs()
 outputCommNetArgs()             Prints parameters to std::cerr verbosely
 outputCommNetArguments2()       Prints command line to std::cout

*/




/* Tested communityNetCustomCdfsFaster() as follows:  

  - Checked by hand that the network seems to grow correctly for 
    the first ~10 added nodes. 
     ./communityNet    14 238  2 0.95 1   3 0.25 0.5 0.75  1   0 1 4 clique > net.edg 2> net.log

  - Number of initial contacts and attempted number of walks follow the
    given distributions. 
    Running with the option #define DEBUG gets the
    realizations printed to std::cerr - at the very end of net.log when the
    output is redirected with 2> net.log as follows: 
    ./communityNet 10010 2349868 2 0.95 1 3 0.25 0.5 0.75 1 0 1 10 random 2 > net.edg 2> net.log 

  - Number of links correct. Two examples when we know exactly how many to expect (saturation eliminated): 
    1) Random seed with N=10 and average degree 2 --> 20 links. At each time step, a single initial contact and a single neighbor of it. No saturation will happen.
    ./communityNet    10010 2349868  1 1   1 0 1   0 1 10 random 2 > net.edg 2> net.log
    should give 10*2 + 10 000 * 2  = 20 020 links.
    wc net.edg
    20021  60106 227754 net.edg
    OK (one header line + 20020 links) 

    2) 10-clique-seed, 2 initial contacts and two walks from each at every time step.  No saturation will happen.
    ./communityNet    10010 2349868  2 0 1   2 0 0 1   0 1 10 clique  > net.edg 2> net.log
     should give 45 + 10 000 * 6  = 60 045 links.
    wc net.edg
    60046 180183 680621 net.edg
    OK (one header line + 60 045 links) 

   - Clustering spectrum matches analytical derivation

   - Degree distribution follows analytical derivation except for the effect of 
     degree correlations that bends down the tails.

*/



//  Authors: Riitta Toivonen




#ifndef COMMUNITYNET_H
#define COMMUNITYNET_H



#include <vector>
#include <cassert>
#include <string>
#include <iostream>
#include <sstream>
#include <ctime>
#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include "../NetExtras.H"   
#include "SeedNet.H" 






/* The struct into which command line arguments will be read */
struct CommNetArgs {
  size_t netSize;
  size_t seedSize;
  SeedType seedType;
  float k_ave;
  int randseed;
  size_t Nrandmax;
  std::vector<float> NrandCdf;
  size_t Nwalksmax;
  std::vector<float> NwalksCdf;
  float delta;
  float w0;
  float mr;
  float ms;
};


















void commNetArgumentError(void) {
  std::cerr << "\nPlease specify required arguments: N randseed Nrandmax [cumulative probabilities of Nrand]  Nwalksmax [cumulative probabilities of Nwalks]   delta w0 seedSize seedType optional:k_ave\n\t\n"; exit(1);
}












/*  readCommNetArgs(struct CommNetArgs & args, int argc, char** argv)

The function for reading in command line arguments for community nets.
This function reads in the cumulative distributions - a different one
would be required for reading arguments in the "old style", specifying
mr and ms for the geometric distribution. */

/* Later, this can be elaborated to read in flagged arguments, such as 
 --N=1000000 --Nrandmax=2 --NrandCdf 0.5 1 --Nwalksmax=1 --NwalksCdf 0.3 1 
etc. Can't spend time on that now. 

Currently, it reads values in the following order: 
N Nrandmax [values] Nwalksmax [values] delta w0 seedSize seedType optional:k_ave*/


void readCommNetArgs(struct CommNetArgs & args, int argc, char** argv) {
  size_t bookmark=1;

  if ( (size_t) argc < (bookmark+1)) commNetArgumentError();
  args.netSize=atoi(argv[bookmark]);
#ifdef DEBUG
  std::cerr << "\tRead argument " << bookmark << ": args.netSize = " << args.netSize  << "\n";
#endif // DEBUG 
 bookmark++;

  if ( (size_t) argc < (bookmark+1)) commNetArgumentError();
  args.randseed=atoi(argv[bookmark]);
#ifdef DEBUG
  std::cerr << "\tRead argument " << bookmark << ": args.randseed = " <<  args.randseed<< "\n";
#endif // DEBUG 
 bookmark++;
  if ( (size_t) argc < (bookmark+1)) commNetArgumentError();

  args.Nrandmax=atoi(argv[bookmark]);
#ifdef DEBUG
  std::cerr << "\tRead argument " << bookmark << ": args.Nrandmax = " << args.Nrandmax << "\n";
#endif // DEBUG
 ++bookmark;

  // Make sure the probability distribution for the number of initial contacts is given
  if ( (size_t) argc < bookmark+args.Nrandmax+1) commNetArgumentError();
  for (size_t i=0; i<args.Nrandmax; ++i){
    args.NrandCdf.push_back(atof(argv[bookmark]));
#ifdef DEBUG
    std::cerr << "\tRead argument " << bookmark << ": probability = " << atof(argv[bookmark]) << "\n";
#endif // DEBUG
 ++bookmark;
  } 
  args.Nwalksmax=atoi(argv[bookmark]); 
#ifdef DEBUG
  std::cerr << "\tRead argument " << bookmark << ": args.Nwalksmax = " << args.Nwalksmax << "\n";
#endif // DEBUG
  bookmark++;
  

  // Make sure the probability distribution for the number of walks is given 
  if ( (size_t) argc < bookmark+args.Nwalksmax+2) commNetArgumentError(); 
  for (size_t i=0; i<args.Nwalksmax+1; ++i){
    args.NwalksCdf.push_back(atof(argv[bookmark]));
#ifdef DEBUG
    std::cerr << "\tRead argument " << bookmark << ": probability = " << atof(argv[bookmark]) << "\n";
#endif // DEBUG
 bookmark++;
  } 



  /* Make sure the other required parameters are given */
  if ( (size_t) argc < bookmark+4) commNetArgumentError(); // we still need delta, w0, seedSize, and seedType
  args.delta=atof(argv[bookmark]); 
#ifdef DEBUG
  std::cerr << "\tRead argument " << bookmark << ": delta = " << args.delta << "\n";
#endif // DEBUG
bookmark++;
  args.w0=atof(argv[bookmark]); 
#ifdef DEBUG
  std::cerr << "\tRead argument " << bookmark << ": w0 = " << args.w0 << "\n";
#endif // DEBUG
bookmark++;
  args.seedSize=atoi(argv[bookmark]); 
#ifdef DEBUG
  std::cerr << "\tRead argument " << bookmark << ": seedSize = " << args.seedSize << "\n";
#endif // DEBUG
bookmark++;



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
#ifdef DEBUG
      std::cerr << "\tRead argument " << bookmark << ": seed type '" << (argv[bookmark]) << "'\n";
#endif // DEBUG
  bookmark++;


  // If using random seed, the average degree is read as input or the default is used:
  args.k_ave=2; /* (if not using random seed, this is just a dummy variable) */
  if (args.seedType == RANDOM) {
    if ( (size_t) argc >= bookmark) {
      args.k_ave=atof(argv[bookmark]); // read k_ave if it is given, else use default as below
#ifdef DEBUG
      std::cerr << "\tRead argument " << bookmark << ": k_ave = " << args.k_ave << "\n";
#endif // DEBUG
    }
    else {
      std::cerr << "Average degree not given for the random seed, using k_ave=2" << std::endl << std::endl;  
    }
  }
  bookmark++;



  // ***************** Test the validity of command line arguments:  *****************

  /* Make sure that the given vectors correspond to cumulative distributions:
     ending with 1, and each value no smaller than the previous */
  if ( ! ( args.NrandCdf[args.Nrandmax-1]==1 ) ) {
    std::cerr << "There's something wrong with the distribution NrandCdf.\n";
    std::cerr << "It currently looks like this:\n";
    std::cerr << "Number of init conts:";
    for (size_t i=0; i<args.Nrandmax; ++i)   std::cerr << "\t" << i+1 ; 
    std::cerr << "\nDiscrete pdf:\t";
    std::cerr << "\t"<< args.NrandCdf[0];
    for (size_t i=0; i<args.Nrandmax-1; ++i)  std::cerr << "\t" << args.NrandCdf[i+1]-args.NrandCdf[i];
    std::cerr << "\nCumulative probability:";
    for (size_t i=0; i<args.Nrandmax-1; ++i)  std::cerr << "\t" << args.NrandCdf[i] ;
    std::cerr << "\t"<< args.NrandCdf[args.Nrandmax-1] << "\n\n";
    exit(1);
  }
  /* args.NrandCdf contains the values 1 to Nrandmax, so its length is 
     Nrandmax-1, since indices start at zero. */
  for (size_t i=0; i<args.Nrandmax-1; ++i) {
    /* If there is something wrong with the given distribution, exit */ 
    if ( args.NrandCdf[i] > args.NrandCdf[i+1] ) {
      std::cerr << "There's something wrong with the distribution NrandCdf\n\n";
      std::cerr << "It currently looks like this:\n";
      std::cerr << "Number of init conts:";
      for (size_t i=0; i<args.Nrandmax; ++i)   std::cerr << "\t" << i+1 ; 
      std::cerr << "\nDiscrete pdf:\t";
      std::cerr << "\t"<< args.NrandCdf[0];
      for (size_t i=0; i<args.Nrandmax-1; ++i)  std::cerr << "\t" << args.NrandCdf[i+1]-args.NrandCdf[i];
      std::cerr << "\nCumulative probability:";
      for (size_t i=0; i<args.Nrandmax-1; ++i)  std::cerr << "\t" << args.NrandCdf[i] ;
      std::cerr << "\t"<< args.NrandCdf[args.Nrandmax-1] << "\n\n";
      exit(1);
    }
  }
  
  /* If there is something wrong with the given distribution, exit */ 
  if ( ! ( args.NwalksCdf[args.Nwalksmax]==1 )  ) {
    std::cerr << "There's something wrong with the distribution NwalksCdf\n\n";
    std::cerr << "It currently looks like this:\n";
    std::cerr << "Number of walks:";
    for (size_t i=0; i<args.Nwalksmax+1; ++i)   std::cerr << "\t" << i ; 
    std::cerr << "\nDiscrete pdf:\t";
    std::cerr << "\t"<< args.NwalksCdf[0];
    for (size_t i=0; i<args.Nwalksmax; ++i)  std::cerr << "\t" << args.NwalksCdf[i+1]-args.NwalksCdf[i];
    std::cerr << "\nCumulative prob.:";
    for (size_t i=0; i<args.Nwalksmax; ++i)  std::cerr << "\t" << args.NwalksCdf[i] ;
    std::cerr << "\t"<< args.NwalksCdf[args.Nwalksmax] << "\n\n";
    exit(1);
  }
  for (size_t i=0; i<args.Nwalksmax; ++i) {
    //std::cerr << "\t" << args.NwalksCdf[i];
    //    assert( ! (args.NwalksCdf[i] > args.NwalksCdf[i+1] ) );
    if (args.NwalksCdf[i] > args.NwalksCdf[i+1] ) {
      std::cerr << "There's something wrong with the distribution NwalksCdf\n\n";
      std::cerr << "It currently looks like this:\n";
      std::cerr << "Number of walks:";
      for (size_t i=0; i<args.Nwalksmax+1; ++i)   std::cerr << "\t" << i ; 
      std::cerr << "\nDiscrete pdf:\t";
      std::cerr << "\t"<< args.NwalksCdf[0];
      for (size_t i=0; i<args.Nwalksmax; ++i)  std::cerr << "\t" << args.NwalksCdf[i+1]-args.NwalksCdf[i];
      std::cerr << "\nCumulative prob.:";
      for (size_t i=0; i<args.Nwalksmax; ++i)  std::cerr << "\t" << args.NwalksCdf[i] ;
      std::cerr << "\t"<< args.NwalksCdf[args.Nwalksmax] << "\n\n";
      exit(1);
    }
  }  
  

  // Calculate mr and ms from the given distributions
  args.mr=args.NrandCdf[0]; // Probability of choosing Nrand=1 
  for (size_t i=1; i<args.Nrandmax; ++i) {
    args.mr+=(float) (i+1)* (args.NrandCdf[i]-args.NrandCdf[i-1]);
  }
  //std::cerr << "args.mr = " << args.mr << "\n";  
  args.ms=0; 
  for (size_t i=1; i<args.Nwalksmax+1; ++i) {
    args.ms+=(float) i*(args.NwalksCdf[i]-args.NwalksCdf[i-1]);
  }
  //std::cerr << "args.ms = " << args.ms << "\n";  

 
}; // end of function readCommNetArgs























/*  void outputCommNetArgs(struct CommNetArgs & args)

Prints command line arguments to standard output for community nets. 

*/

void outputCommNetArgs(struct CommNetArgs & args) {

  // ***************** Print parameters to stderr:  ****
  std::cerr << std::endl << std::endl
	    << "Simulating a social type network with communities" << std::endl
	    << std::endl 
	    << " Parameters: " << std::endl <<
    "N:\t\t" << args.netSize  << std::endl <<
    "m_r:\t\t" << args.mr << std::endl <<
    "m_s:\t\t" <<  args.ms << std::endl <<
    "seed size:\t" << args.seedSize << std::endl;
  
  if (args.seedType == RANDOM)
    std::cerr << "seed type:\t" << "random seed (Erdos-Renyi) with average degree " << args.k_ave << std::endl << "\t\tNote: Any disconnected random seed is discarded, \n\t\tand a new one is generated until a connected \n\t\tnetwork is found.\n" << std::endl;
  else if (args.seedType == CLIQUE) 
    std::cerr << "seed type:\t" << "fully connected" << std::endl;    
  else if (args.seedType == RING) 
    std::cerr << "seed type:\t" << "ring" << std::endl;
  else if (args.seedType == CHAIN) 
    std::cerr << "seed type:\t" << "chain" << std::endl;
  
  
  std::cerr <<  "\nCumulative distribution for number of initial contacts:\n";
  
  std::cerr << "Number of init conts:";
  for (size_t i=0; i<args.Nrandmax; ++i)   std::cerr << "\t" << i+1 ; 
  std::cerr << "\nDiscrete pdf:\t";
  std::cerr << "\t"<< args.NrandCdf[0];
  for (size_t i=0; i<args.Nrandmax-1; ++i)  std::cerr << "\t" << args.NrandCdf[i+1]-args.NrandCdf[i];
  std::cerr << "\nCumulative probability:";
  for (size_t i=0; i<args.Nrandmax-1; ++i)  std::cerr << "\t" << args.NrandCdf[i] ;
  std::cerr << "\t"<< args.NrandCdf[args.Nrandmax-1] << "\n\n";
  
  
  std::cerr <<  " Cumulative distribution for number of walks from each initial contact:\n";  
  std::cerr << "Number of walks:";
  for (size_t i=0; i<args.Nwalksmax+1; ++i)   std::cerr << "\t" << i ; 
  std::cerr << "\nDiscrete pdf:\t";
  std::cerr << "\t"<< args.NwalksCdf[0];
  for (size_t i=0; i<args.Nwalksmax; ++i)  std::cerr << "\t" << args.NwalksCdf[i+1]-args.NwalksCdf[i];
  std::cerr << "\nCumulative prob.:";
  for (size_t i=0; i<args.Nwalksmax; ++i)  std::cerr << "\t" << args.NwalksCdf[i] ;
  std::cerr << "\t"<< args.NwalksCdf[args.Nwalksmax] << "\n\n";
  
  std::cerr <<  "Seed used for random number generator:\t" << args.randseed << std::endl << std::endl << std::endl;
  
}  // end of function outputCommNetArgs









void outputCommNetArguments2(int argc, char** argv) {

  /**** For double-checking, print all parameters on the first line
        of the data file - to std::cerr. This can be ignored by Matlab.  ****/ 
  std::cout << "* * * * * Parameters in order: N Nrand [Nrand values] Nwalks [Nwalks+1 values] delta w0 seedsize seedtype randseed k_ave. This file was generated by the call: ";
  for (int i=0; i<argc; ++i) 
    std::cout << " " << argv[i]; // Print out all params..
  std::cout << " * * * * * \n"; // ...and end with newline.   
}









































/* function communityNetCustomCdfsFaster(NetType& theNet, struct CommNetArgs & args, Generator & generator) */

template<typename NetType, typename Generator>
void communityNetCustomCdfsFaster(NetType& theNet, struct CommNetArgs & args, Generator & generator) {


  /* A map from initial contacts to their neighbors */
  typedef Map<size_t, Set<size_t> > ConnMap;

  ClearNet(theNet,args.netSize);   /* make sure there are no edges present to start with */

  
  struct SeedArgs seedArgs;
  seedArgs.netSize=args.netSize; 
  seedArgs.seedSize=args.seedSize; 
  seedArgs.seedType=args.seedType; 
  seedArgs.k_ave=args.k_ave; 
  generateSeedNetwork(theNet,seedArgs,generator); /*generates a seed network and copies the edges into theNet*/

  
#ifdef DEBUG 
  std::vector<size_t> DistributionTestNrand;
  for (size_t k=0; k<args.Nrandmax; ++k)  DistributionTestNrand.push_back(0);
  std::vector<size_t> DistributionTestNwalks;
  for (size_t k=0; k<=args.Nwalksmax; ++k)  DistributionTestNwalks.push_back(0);
#endif // DEBUG	
  
  /* Growth loop. Adding node i. */
  for (size_t i=args.seedSize; i<args.netSize; ++i) {
    
#ifdef DEBUG
    std::cerr << "***********************************************************************\n";
    std::cerr << "\t\tAdding new node " << i << " to the network.\n\n";
#endif // DEBUG 


    Set<size_t> allChosen; // All nodes chosen at this time step 
    ConnMap chosenNeighs;  // The initial contacts chosen at this time
			   // step, and the neighbors we walked to
			   // from each initial contact

    
    /* First decide how many initial contacts to choose. */
    float toss=generator.nextNormed(); // a random number uniformly from [0,1) 
    size_t Nrand=0; while ( (args.NrandCdf[Nrand] <= toss) && (Nrand < args.NrandCdf.size() ) ) { Nrand++; }
    Nrand++; 
#ifdef DEBUG    
    std::cerr << "Going to pick nrand = " << Nrand << " initial contacts.\n"; 
    DistributionTestNrand[Nrand-1]++;
#endif // DEBUG	
    
    /* Then pick them. */
    for (size_t m=0; m<Nrand; ++m) {
      size_t randNode;
      if (allChosen.size()==i) {std::cerr << "Picked all nodes in the network. This is not good."; exit(1); }
      
      size_t tries=0;
      size_t maxtries=200;
      do {
	/* Choose a new random node, avoiding duplicates. */
	randNode=generator.next(i);  // an integer value from 0 to i-1
	tries++;
	if (tries>maxtries) {
	  std::cerr << "Couldn't find a new random node not already touched on this time step in "<<maxtries<< " tries. Giving up. ";
	  exit(1);
	}
      } while (allChosen.contains(randNode)); 
      allChosen.put(randNode);   
      chosenNeighs[randNode];
    }  // end of loop for picking random nodes
    
#ifdef DEBUG
    std::cerr << "\tChose random nodes (node id:s): "; 
    for ( Set<size_t>::iterator j=allChosen.begin(); !j.finished(); ++j)
      std::cerr << *j << "  ";    
    std::cerr << "\n";
#endif // DEBUG	
    
    
    
    /* For each initial contact, do some walks according to NwalksCdf */      
    /* First decide how many walks to do from this initial contact. */
    for (ConnMap::iterator j=chosenNeighs.begin(); !j.finished(); ++j) { //*j is the initial contact  
      bool saturated=false;  // first assume that this initial contact is not saturated; check later
      toss=generator.nextNormed(); size_t Nwalks=0;
      while ( (args.NwalksCdf[Nwalks] < toss) && (Nwalks < args.NwalksCdf.size() ) ) { Nwalks++; }
#ifdef DEBUG
      std::cerr << "Decided to do " << Nwalks << " walks from init cont " << *j << ".\n";
      std::cerr << "Init cont " << *j << " has " << theNet(*j).size() << " neighbors.\n";
      DistributionTestNwalks[Nwalks]++;
#endif // DEBUG	
      for (size_t r=0; r<Nwalks; ++r) {
	size_t walkEnd;
	size_t tries=0;
	size_t maxtries=10;
	if (! theNet(*j).size()==0 ) { // if there are any neighbors (in a disconnected network, there might not be)
	  do {
	    /* A single step of (weighted) random walk: */
	    walkEnd=theNet(*j).weighedRandKey(generator);
	    tries++; 
	    if (tries > maxtries) { // If already tried a few times without success, check if the neighborhood is saturated; if it is, move on, else, reset the counter and go on trying.
#ifdef DEBUG
	      std::cerr << "Looked for new neighbors " << maxtries << " times. Checking for saturation.\n";
#endif // DEBUG	
	      Set<size_t> availableNeighs;
	      size_t counter=0;
	      for  (typename NetType::const_edge_iterator k=theNet(*j).begin(); !k.finished(); ++k) {
		if ( allChosen.contains(*k) )  /* if this neighbor was already picked during this time step */
		  counter+=1; 
		else
		  availableNeighs.put(*k); /* refine later: if maxtries has already been reached once (use a flag), pick from availableNeighs and not just randomly. */
	      }
#ifdef DEBUG
	      std::cerr << "Number of neighbors " << *j << " has: " << theNet(*j).size() << ".\n";
	      std::cerr << "Of these, already picked: " << counter << ".\n";
	      std::cerr << "Available neighbors: " << availableNeighs.size() << ".\n";
	      std::cerr << "These sum up, right?\n";
#endif // DEBUG	
	      if (counter==theNet(*j).size() ) { /* If ...*/
		saturated=true;
#ifdef DEBUG
		std::cerr << "Neighborhood of initial contact " << *j << " was saturated.\n";
		std::cerr << "Did only " << r << " walk(s).\n";
#endif // DEBUG	
		break;
	      }
	      else 
		tries=0; // reset counter and try some more
#ifdef DEBUG
	      std::cerr << "Decided to go on looking for more neighbors.\n";
#endif // DEBUG	
	    }
	  } while (allChosen.contains(walkEnd) && saturated==false);
	  if (saturated==true)  break;
	  chosenNeighs[*j].put(walkEnd);
	  allChosen.put(walkEnd);   
#ifdef DEBUG
	  std::cerr << "\tChose neighbor " << walkEnd << ". ";
	  std::cerr << "Done " << r+1 << " walk(s) so far.\n";
#endif // DEBUG	
	}
      }
    } // end of loop for doing walks from  each random node
    
    
    
    /* Make the connections */
#ifdef DEBUG
    std::cerr << "-----------------\n";  
    std::cerr << "Making the links.\n"; 
#endif // DEBUG	

    for (ConnMap::iterator j=chosenNeighs.begin(); !j.finished(); ++j) {
      theNet[i][*j]=args.w0;
#ifdef DEBUG
      std::cerr << "Linking the new node " << i << " to init cont "<< *j << ".\n";
#endif // DEBUG	
      
      for (Set<size_t>::iterator k=chosenNeighs[*j].begin(); !k.finished(); ++k) {
	theNet[i][*k]=args.w0;
#ifdef DEBUG
	std::cerr << "\tLinking the new node " << i << " to neighbor " << *k << " of " << *j << ".\n"; 
#endif // DEBUG	

	theNet[*j][*k]+=args.delta;
#ifdef DEBUG

	if (args.delta>0)
	  std::cerr << "\t\tStrengthening the link " << *j << " to " << *k << " by "<< args.delta << ".\n"; 
#endif // DEBUG	
      }
    }  
    
#ifdef EDGECLUSTERING
    size_t t=i+1; // node indices start at 0 at time 1
    if ( ( (float) t/1000  -  t/1000 )== 0 )  {
      outputEdgeClustering(theNet,args);
      std::cerr << "Outputting edge clustering for t=" << t << ".\n"; //REMOVE
    }
#endif // EDGECLUSTERING    

  } // end of loop for adding node i to the network

#ifdef DEBUG
    std::cerr << "------------------------------------------\n";  
    std::cerr << "Distribution of number of initial contacts:\n"; 
    for (size_t i=0; i<DistributionTestNrand.size(); ++i) 
      std::cerr << "\t" <<  i ; 
      std::cerr << "\n" ; 
    for (size_t i=0; i<DistributionTestNrand.size(); ++i) 
      std::cerr << "\t" <<  DistributionTestNrand[i] ; 
    std::cerr << "\t times\n\n";
    std::cerr << "-------------------------------\n";  
    std::cerr << "Distribution of number of walks:\n"; 
    for (size_t i=0; i<DistributionTestNwalks.size(); ++i) 
      std::cerr << "\t" <<  i ; 
      std::cerr << "\n" ; 
    for (size_t i=0; i<DistributionTestNwalks.size(); ++i) 
      std::cerr << "\t" <<  DistributionTestNwalks[i] ; 
    std::cerr << "\t times\n\n";
#endif // DEBUG	

  
} // end of loop for generating the network


// <--- communityNetCustomCdfsFaster
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
























// CommunityNet  ---->

template<typename NetType, typename Generator>
void CommunityNet(NetType& theNet,size_t netSize, float p_r, float p_t, float delta, float w_0, size_t seedSize, SeedType seedType, Generator & generator, float k_ave=0) {
  

  /* A map from nodes in R to nodes in T. */
  typedef Map<size_t, Set<size_t> > ConnMap;
  

  ClearNet(theNet,netSize); /* make sure there are no edges present to start with */

  /* generate the seed network */
  
  size_t i=0;

  if (seedType == CLIQUE) {
    /* Fully connected seed: */     
    for (; i<seedSize; ++i) {
      for (size_t j=0; j<i; ++j) {
	theNet[i][j]=w_0;
      }
    }
  }
  
  else if (seedType == RANDOM) {

    /* Generate random seed: */     
    NetType seedNet(seedSize); 
    size_t trials=0; 
    size_t maxtrials=20;
    do {
      if (trials++ > maxtrials) {
	std::cerr << "\nCouldn't generate a connected Erdos-Renyi seed network in "
		  << maxtrials << " trials, giving up \nand aborting. Please try using a higher average degree (over 1.2). \n\n\n";
	assert(false);
      }
      ErdosRenyi(seedNet,seedSize,k_ave,generator,w_0);
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
    //    std::cerr << "ER copying seed net into theNet... \n\n"; 
    for (; i<seedSize; ++i) {
      for (typename NetType::const_edge_iterator j=seedNet(i).begin(); 
	   !j.finished(); 
	   ++j) {
	//	std::cerr << "Copying edge " << i << "-" << *j 
	//  << " from the seed network.\n\n";
	theNet[i][*j]=seedNet(i)[*j];
      }
    }
  }
  
  else  assert(false);  // No other seed options available.



  /* Growth loop. i is the node to be inserted.*/
  
  for (; i<netSize; ++i) {
    ConnMap RtoT;
    /* The loop for constructing the initial neighbours. */
    do {
      size_t newNeighbour;
      if (RtoT.size()==i) {
	std::cerr << "Bummer! You did not think of that, did you" << std::endl 
		  << "Picked all nodes in the network to be initial contacts!" << std::endl;
	break;
      }
      do {
	/* Choose a new initial neighbour. Duplicates do not count,
	 * hence the loop.*/
	newNeighbour=generator.next(i);
      } while (RtoT.contains(newNeighbour));
      /* We init the member in the map. The stdlib-style behaviour 
       * seems rather useful in some cases, after all.
       */
      RtoT[newNeighbour];
    } while (generator.nextNormed() >= p_r);

    /* The loop for neighbours' neighbours*/
    
    while (generator.nextNormed() >= p_t) {
      /* A random node from R */
      size_t neighbour=RtoT.randKey(generator);
      /* If some node's neighbourhood gets "full", we break. */
      if (theNet(neighbour).size()==RtoT[neighbour].size()) break;
      /* A single step of (weighed) random walk: */
      size_t neighsNeigh=theNet(neighbour).weighedRandKey(generator);
      /* Store the result. */
      RtoT[neighbour].put(neighsNeigh);
    }
   
    for (ConnMap::iterator j=RtoT.begin(); !j.finished(); ++j) {
      theNet[i][*j]=w_0;
      for (Set<size_t>::iterator k=RtoT[*j].begin(); !k.finished(); ++k) {
	theNet[i][*k]=w_0;
	theNet[i][*j]+=delta;
	theNet[*j][*k]+=delta;
      }
    }
  }
 
}
// <--- CommunityNet
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 











#endif // COMMUNITYNET_H
