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

// OldModels.H
// Network models - obsolete file


/* 

 The network model functions take in a reference to a network 
 of fixed size, whose edges they modify. The type of seed network 
 used for growing network models is specified with the enumerated 
 variables RANDOM (referring to an Erdös-Renyi random network) 
 and CLIQUE (fully connected network). 

 Each function also takes in a reference to a random number generator 
 of type RandNumGen (from /proj/finance/lcelib/Randgens.H). 

 (This information may or may not change in the future as more
 network models  are included and more features added. It held 
 true at Aug 30 2005.)




 Models included:  
 ---------------
 CommunityNet (Aug 19 2005)
      A simple social type network 
      with seed options: fully connected or random seed

  ErdosRenyi (Aug 30 2005)
      Erdös-Rényi random graph


  bbvNet (Sep 30 2005)
      The BBV network model




 Other related functions:
 -----------------------
 ClearNet
      A function to clear all edges of a network. 
      The number of nodes does not change. 
 


 Authors: Riitta Toivonen, Jörkki Hyvönen

 */



#ifndef OLDMODELS_H
#define OLDMODELS_H
#include "../Containers.H"
#include "../Nets.H"
#include "../Randgens.H"
#include "Dijkstrator.H"

#include <cassert>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <ctime>







// Seed network options:
// --------------------
enum SeedType {RANDOM, CLIQUE, CHAIN, RING}; // RANDOM: Erdös-Renyi random network
                                // CLIQUE: fully connected network
SeedType seedType = CLIQUE; // Defaults to fully connected seed.











// Auxiliary functions:
// -------------------


// ClearNet ---->

template <typename NetType>
void ClearNet(NetType& theNet, size_t netSize) {
  /* Go through each node */
  for (size_t i=0; i<netSize; ++i) {
    /* and remove the links to its neighbors */
    for (typename NetType::edge_iterator j=theNet[i].begin(); 
	 !j.finished();  
	 ++j) {
      j.value()=0;  // don't use 'theNet[i][*j]=0;' to avoid messing up the iterator
    }
  }
}
//  <---- ClearNet




// ConnectivityCheck ---->
/*

The function ConnectivityCheck uses the Dijkstra algorithm 
to determine whether or not a path exists between the first 
node in the network and all others - in other words, whether 
or not the network is connected. In fact, it finds all shortest 
paths from this node, which takes an equivalent time to calculate.

If the network is connected, all nodes are reachable from 
any single node. We start the search with the first node 
in the network, since it always exists for a nonempty network. 

To carry out the Dijkstra algorithm, we use the Dijkstrator 
from /proj/finance/lcelib/nets/Dijkstrator.H.

*/

template<typename NetType>
bool ConnectivityCheck(NetType & theNet) {
  
  /* Find all shortest paths from node 0 */
  Dijkstrator<NetType> paths(theNet,0); 
  for (; !paths.finished(); ++paths) {
  }
 
  /* Check whether all other nodes were reachable */
  assert( paths.getFoundSet().size() <= theNet.size() );
  if (paths.getFoundSet().size() == theNet.size() )
    return true;
  else
    return false;
}
//  <---- ConnectivityCheck










// Network models:
// --------------


// ErdosRenyi ---->
// Note: If NetType is SymmNet<bool>, any edge weights w0 which differ 
// from zero are converted to true without warning.  For example, if 
// the variable theNet is of type SymmNet<bool>, a function call 
// ErdosRenyi(theNet, netSize, k_ave, generator, 0.4)
// will generate a network with unweighted edges, ignoring the '0.4'. 


template<typename NetType, typename Generator>
void ErdosRenyi(NetType& theNet, size_t netSize, float k_ave, Generator & generator, typename NetType::EdgeData w0=1) {
  ClearNet(theNet,netSize); /* make sure there are no edges present to start with */
  const size_t m=(size_t) ceil((float) netSize * k_ave); /* number of edges to be added */
  size_t i, j; 
  /* add m edges, avoiding loops and multiple edges */
  for (size_t k=0; k<m; ++k) {
    do {
      i=generator.next(netSize);
      j=generator.next(netSize);
      assert(i<netSize && j<netSize);
    }
    while ( (i==j) || theNet(i)[j]!=typename NetType::EdgeData() ); /* no loops of length 1, no multiple edges */
    assert(i!=j);
    theNet[i][j]=w0; /* add edge between nodes i and j */
  }
}
// <---- ErdosRenyi






// CommunityNet  ---->
// If not using SeedType RANDOM, the variable k_ave will not be used,
// and you can call CommunityNet without k_ave in the argument list.


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



// bbvNet --->

template<typename NetType, typename Generator>
void bbvNet(NetType& net, size_t netSize, size_t N_0, size_t m, float w_0, float delta, Generator & generator) {
  ClearNet(net,netSize); /* make sure there are no edges present to start with */

  /*initial edges*/
  for(size_t i=0; i < N_0; i++) {
    for(size_t j=0; j < i; j++) {
      net[i][j]=w_0;
    }
  }

  for(size_t i=N_0; i < netSize; i++) {
    
    /*Selecting m random nodes*/
    Set<size_t> conns;
    while(conns.size() < m) {
      conns.put(net.weighedRandSlot(generator));
    }
    
    /*Constructing the map of all neighbours and their neighbours*/
    Map<size_t, Map<size_t, float> > bigMap;
    for(Set<size_t>::iterator j=conns.begin(); !j.finished(); ++j) {
      bigMap[*j] = net(*j);
    }

    /*Changing the weights*/
    for(Set<size_t>::iterator j=conns.begin(); !j.finished(); ++j) {
      const float c = delta / net(*j).weight();
      net[i][*j] = w_0;

      for(Map<size_t, float>::iterator k=bigMap[*j].begin(); !k.finished(); ++k) {
	  net[*j][*k] =+ bigMap[*j][*k] * c;
      }
    }

  }
}

// <--- bbvNet



#endif //~ OLDMODELS_H
