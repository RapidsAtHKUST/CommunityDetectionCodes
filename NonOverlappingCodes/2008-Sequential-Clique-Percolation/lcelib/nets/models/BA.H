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


// BA.H

// Functions used for constructing the basic Barabasi-Albert scale-free networks by drawing the contacts for a
// new node from the weighed distribution of the nodes.

// Usage example in        lcelib/nets/Examples/BAnetworks.cpp 

// Jussi Kumpula 13.1.2006


#ifndef BA_H
#define BA_H

#include <string>
#include <iostream>
#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include "../NetExtras.H"   
#include "SeedNet.H" 




typedef Set<size_t> nodeSet; // stores the indexes of selected nodes





/* function  BAnet_const_addition(NetType& theNet, size_t initialContactsPerNode, struct SeedArgs & seedArgs, Generator & generator) 

Generates a Barabasi-Albert network. First a small seed network is generated according to seedArgs structure and then
the rest of the nodes are added so that each new node connects to initialContactsPerNode random nodes in the network. The probability to
choose a node is proportional to its weigth. Naturally, initialContactsPerNode has to be an integer which means that average degree
in the network is always an even number. For more general average degree see function BAnet_geometric below.

Note that SymmNet<float, ValueTable, ExplSumTreeTable> is much faster SymmNet<float> for drawing weighed nodes!

*/


template<typename NetType, typename Generator>
void BAnet_const_addition(NetType& theNet, size_t initialContactsPerNode, struct SeedArgs & seedArgs, Generator & generator) {

  size_t netSize = theNet.size();

  if (initialContactsPerNode < 1) {
    std::cerr << "Too small number of initial contacts for each node specified.\n" 
	      << "Network can not be constructed.\n";
    exit(1);
  }

  if (seedArgs.seedSize < initialContactsPerNode) {
    std::cerr << "Seednet size is smaller than the number of connections to add for each new node.\n" 
	      << "Network can not be constructed.\n";
    exit(1);
  }

  ClearNet(theNet,netSize);                       /* make sure there are no edges present to start with */
  generateSeedNetwork(theNet,seedArgs,generator); /*generates a seed network and copies the edges into theNet*/

  for(size_t i=seedArgs.seedSize; i < netSize; ++i) {
     
    /*Selecting initialContactsPerNode random nodes using weighed distribution*/
    nodeSet conns; 
    size_t target;
    while(conns.size() < initialContactsPerNode) {
      target = theNet.weighedRandKey(generator);
      conns.put(target);
    }
    
    /* add links, all connections have weight=1 */
    for(nodeSet::iterator j=conns.begin(); !j.finished(); ++j) theNet[i][*j]=1;
  }
}
















/* function BAnet_geometric(NetType& net, const float k_ave, struct SeedArgs & seedArgs,  Generator & generator) 

Generates a Barabasi-Albert network.  First a small seed network is generated according to seedArgs structure.
The rest of the nodes are added so that for each new node the number of initial contacts is drawn from 
a geometric distribution with expectation value k_ave. The probability to choose a node is proportional to its weigth. 

Note that SymmNet<float, ValueTable, ExplSumTreeTable> is much faster SymmNet<float> for drawing weighed nodes!

*/

template<typename NetType, typename Generator>
void BAnet_geometric(NetType& net, const float k_ave, struct SeedArgs & seedArgs,  Generator & generator) {

  if (k_ave < 0) {
    std::cerr << "Negative average degree given.\n" 
	      << "Network can not be constructed.\n";
    exit(1);
  }

  size_t netSize=net.size();
  const float p_r = 2.0/k_ave;                    /* probabity to add one more node according to geometric distribution */
  ClearNet(net,netSize);                          /* make sure there are no edges present to start with */
  generateSeedNetwork(net,seedArgs,generator); /*generates a seed network and copies the edges into theNet*/

  
  for (size_t i=seedArgs.seedSize; i<netSize; ++i) {
    nodeSet conns;
    /* The loop for constructing the initial neighbours. */
    do {
      size_t newNeighbour;
      if (conns.size()==i) {
	std::cerr  << "Picked all nodes in the network to be initial contacts!" << std::endl;
	break;
      }
      do {
	/* Choose a new initial neighbour. Duplicates do not count,
	 * hence the loop.*/
	newNeighbour=net.weighedRandKey(generator); 
      } while (conns.put(newNeighbour));
    } while (generator.nextNormed() >= p_r);

    for (nodeSet::iterator j=conns.begin(); !j.finished(); ++j) net[i][*j]=1;
  }
}




#endif // BA_H
