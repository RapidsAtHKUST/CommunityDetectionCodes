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

// lcelib/nets/models/VDEB.H



/*
 Functions included:  
 ---------------

 VDEB(net,u,generator) 

A growing variant of the Davidsen, Ebel, Bornholdt 2002 model for social networks
(Davidsen, J., Ebel, H., and Bornholdt, S., 2002.
Emergence of a Small World from Local Interactions: Modeling
Acquaintance Networks. Phys. Rev. Lett. 88 (12) 128701-1.)

proposed by A. Vazquez, 2003 
(Growing networks with local rules: Preferential attachment, clustering
hierarchy, and degree correlations. Phys. Rev. E 67, 056104)

In this variant, at each time step 
  1) with prob. u a link is formed between two non-neighboring nodes 
     which share at least one common neighbor (in Vazquez's words,  
     a "potential edge" is converted into an edge)
  2) with prob. 1-u, a new node is added to the network, joining it to a random node.

  I keep track of potential edges by storing them in a set (Jorkki's hash set).
  Whenever a new edge is added to the network, I add any new potential edges to this set.
  When a potential edge is converted to an edge, I remove it from the list. 



*/




#ifndef VDEB_H
#define VDEB_H
#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include "../Dijkstrator.H"
#include <cassert>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include "../NetExtras.H"  // required for clearNet()




// VDEB ---->


template<typename NetType, typename Generator>
void VDEB(NetType& net, const float u, Generator & generator) {
  ClearNet(net,net.size()); /* make sure there are no edges present to start with */

  // NetType oldNet(net.size());
  
   /* container (hash set) to hold potential edges (and some more) */
  Set< std::pair<size_t,size_t> > potentialEdges; 
  // std::pair<size_t,size_t> thisEdge(4, 5); 
  
  /* to begin with, we have one node in the network (0) and no edges */
  size_t nextNodeIndex=1;

  std::pair<size_t,size_t> thisPair;

  while(nextNodeIndex < net.size() ) {
    /* either add a new node, connecting it to a random node */
    float toss=generator.nextNormed();
    if ( toss > u ) { // with prob. 1-u
      //std::cerr << "\nTossed 1-u, adding a new node ... \n";
      size_t randomNode=generator.next(nextNodeIndex); // a random number from 0 to nextNodeIndex-1
      /* add the new potential edges: from the new node to all neighbors of the random node */
      for  (typename NetType::const_edge_iterator k=net(randomNode).begin(); !k.finished(); ++k) {
	thisPair=std::make_pair(*k,nextNodeIndex); potentialEdges.put(thisPair);
	//std::cerr << "Added potential edge " << *k << "-" << nextNodeIndex <<"\n";
      }
      net[nextNodeIndex][randomNode]=1;
      //std::cerr << "Added node " << nextNodeIndex << ", linking it to node " << randomNode<< "\n";
      nextNodeIndex++;
    }
    else { // with prob. u      /* or convert a potential edge to an edge */      
      //std::cerr << "\nTossed u, trying to convert a potential edge to an edge ... \n";
      std::pair<size_t,size_t> e;
      bool notFound=true;
      do {
	if ( potentialEdges.size() ==0 ) { 
	  //std::cerr << "Breaking out because there are no edges.\n"; 
	  break;}
	e = potentialEdges.randKey(generator); 
	if ( net[e.first][e.second]==0 ) {notFound=false; } // if the edge really wasn't in the network yet, 
	// we found what we were looking for (but don't add it in the network just yet)
	else { potentialEdges.remove(e); } // otherwise, remove it from potentialEdges
      }  while ( notFound );
      /* Now add the new potential edges: from each end node of the new edge to all neighbors of 
	 the other end node, if the link isn't there yet */
      for  (typename NetType::const_edge_iterator k=net(e.second).begin(); !k.finished(); ++k) {
	if (net[e.first][*k] == 0) {   // if the link isn't there yet 
	  // make sure the pairs are ordered smallest first
	  if (e.first < *k) {thisPair=std::make_pair(e.first,*k); } 
	  else {thisPair=std::make_pair(*k,e.first); } 
	  potentialEdges.put(thisPair);
	  //std::cerr << "- Added potential edge " << thisPair.first << "-" << thisPair.second <<"\n";
	}
      }
      for  (typename NetType::const_edge_iterator k=net(e.first).begin(); !k.finished(); ++k) {
	if (net[e.second][*k] == 0) {   // if the link isn't there yet 
	  // make sure the pairs are ordered smallest first 
	  if (e.second < *k) {thisPair=std::make_pair(e.second,*k); } 
	  else {thisPair=std::make_pair(*k,e.second); } 
	  potentialEdges.put(thisPair);
	  //std::cerr << "- Added potential edge " << thisPair.first << "-" << thisPair.second <<"\n";
	}
      }
      /* And then convert the potential edge to a real edge */
      if (notFound==false) {  // if we did manage to find a suitable potential edge
	net[e.first][e.second]=1; 
	thisPair=std::make_pair(e.first,e.second);  
	potentialEdges.remove(thisPair); 
	//std::cerr << "Converted the  potential edge " << e.first << "-" <<e.second << " to an edge. \n";   
      }
    }
  } // loop for adding all N nodes to the network 
}
// <---- VDEB




#endif //~ VDEB_H

