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


// lcelib/nets/Randomizer.H  
// 
// Authors: Jussi Kumpula, Jukka-Pekka Onnela, Riitta Toivonen, Mikko Kivelä



/* 

Functions included: 
------------------

confModelSimple

switchConnections()
switchLinkPairEnds()
randomize() 



The function randomize mixes the edges of the network, keeping the
degree distribution intact.

For more information, see comments above the function randomize().

For a usage example, see lcelib/nets/Examples/randomizer.cpp
*/








#ifndef RANDOMIZER_H
#define RANDOMIZER_H
#include "../Containers.H"
#include "../Randgens.H"
#include "Dijkstrator.H"
#include <cassert>
#include <iostream>
#include <vector>







template <typename NetType, typename Generator>
int confModelSimple(NetType & theNet, Generator & generator, int repeats){
  /* Configuration model for generating simple graphs. Simple graph is a graph
     without self-links and multiple links. 

     In practice, we do not use stubs-based algorithm, but a MCMC method where
     end nodes of pairs of links are exchanged/swapped. The pair of edges is selected
     by selecting each edge uniformly random and repeating this two times. If
     the links are AB and CD, and links AD and AB don't exist, they are created.
     This procedure is then repeated.

     Like any MCMC sampling method, the samples are uniform and independed only
     when the number of repeats reached infinity. In practice, however, this kind
     of methods are widely used by just having the number of repeats very large.

     How many times should you then repeat the swapping step? This depends heavily
     on the network. Minimum requirement is to check that the number of successful swaps
     is such that each edge is moved at least few times. The number of untouched 
     edges is on average: L*(1-(1-2/L)**n), where n is the number of successful 
     swaps and L is the number of links in the network. The probability that swap
     succeeds is usually very high for large sparce networks, but is lowered if
     the network is dense or if the degree distribution is fat.
     
     The correctedness of this method can be seen considering following construction:
     Consder a metagrap, which is a graph of graphs, where each node is such a graph
     that it has the same degree sequence as the graph which is given as an input
     to this algorithm. Now there is a link from graph A to another graph B if A can
     be modified to B by swapping two edges as described above. The connection from
     A to B is further weighted with the probability of this swap, when the algorithm
     is at A. We additionally have self-link, which has the probability of failing in
     the swapping process. Now, clearly, every non-self-link has a probability of
     1/(L**2), where L is the number of links in the graph. Now L is constant so the
     metagraph is undirected. Also each node in the metagraph has the same in and out
     strength of 1. Further, the metagraph is connected. If we now put an random walker 
     in this graph, such that at each step it follows each with the probability attached 
     to it, we get that the stationary distribution for finding the walker in a node is
     same for each node. That is, the sampler gives uniform sample from the set of all
     graphs with the given degree sequence after inifinite number of steps.

     Parameters
     ----------
     theNet : Reference to the network that you want to randomize.
     generator : The random number generator.
     repeats : Number of attempts to rewire two links. 

     Returns
     -------
     The number of successfull rewiring steps.

     Complexity
     ----------
     Processor: O(repeats + number of links)
     Memory: O(number of links), this could be made lower.

     Authors
     -------
     Mikko Kivelä (bolozna@gmail.com)

  */

  // First we populate the list of all edges in the network
  std::vector< std::pair<size_t, size_t> > edgeList;
  for (size_t i=0;i<theNet.size();++i)
    for (typename NetType::edge_iterator k=theNet[i].begin(); !k.finished(); ++k) {
      if(i<*k) edgeList.push_back(std::pair<size_t,size_t>(i,*k));
    }
   

  int successfullRounds=0;
  for (size_t round=0;round<repeats;++round){
    //Get two random edges
    size_t edgeIndex1=generator.next(edgeList.size());
    size_t edgeIndex2=generator.next(edgeList.size());

    //Check that the edges are not same edge
    if (edgeIndex1!=edgeIndex2){
      std::pair<size_t,size_t> edge1=edgeList[edgeIndex1];
      std::pair<size_t,size_t> edge2=edgeList[edgeIndex2];
      //Check that the edges do not share a node
      if ((edge1.first != edge2.first ) && 
	  (edge1.first != edge2.second) && 
	  (edge1.second!= edge2.first ) &&
	  (edge1.second!= edge2.second) ){

	
	if (generator.next(2)==0){
	  size_t temp=edge1.first;	  
	  edge1.first=edge1.second;
	  edge1.second=temp;
	}
	

	// Check that the edges do not exist
	if ((theNet(edge1.first)[edge2.second]==typename NetType::EdgeData())&&   //
	    (theNet(edge1.second)[edge2.first]==typename NetType::EdgeData())   ) {
	  successfullRounds++; // keep track of successfull swaps
	  
	  //Update the list, do not keep ordered
	  edgeList[edgeIndex1].first=edge1.first;
	  edgeList[edgeIndex1].second=edge2.second;
	  edgeList[edgeIndex2].second=edge1.second;

	  //Randomize the changing order, this is not needed if edges are not weighted
	  if (generator.next(2)==0){
	    theNet[edge1.first][edge2.second]=theNet(edge1.first)[edge1.second];
	    theNet[edge1.second][edge2.first]=theNet(edge2.first)[edge2.second];
	  } else{
	    theNet[edge1.first][edge2.second]=theNet(edge2.first)[edge2.second];
	    theNet[edge1.second][edge2.first]=theNet(edge1.first)[edge1.second];
	  }
	  //Remove the old edges
	  theNet[edge1.first][edge1.second]=0;
	  theNet[edge2.first][edge2.second]=0;

	  //Check that the edge list is consistent with the net
	  //for (size_t i=0;i<edgeList.size();++i)
	  //  if(theNet(edgeList[i].first)[edgeList[i].second]==0) std::cerr << edgeList[i].first<<" "<<edgeList[i].second << std::endl;
	
	}
      }
    }
  }
  return successfullRounds;

}








/* function switchConnections */

template <typename NetType,  typename Generator>
void switchConnections(NetType & theNet, Generator & generator, size_t netSize) {

  // float should be replaced by the real type of the weights
  typedef Map<size_t, float > WeighedNodes;


  size_t i,j;

  // choose the nodes which switch the connections
  do {
    i=generator.next(netSize);
    j=generator.next(netSize);
  } while (i==j);
  assert(i!=j);


  WeighedNodes tmp_nodes;

  // copy edges from i to temporary storage
  for (typename NetType::edge_iterator k=theNet[i].begin(); !k.finished(); ++k) {
    tmp_nodes[*k]=theNet(i)[*k]; // stores the destination number and edge weight
    k.value() = 0; // remove edge i - > k
  }

  // copy connections from j -> i
  for (typename NetType::edge_iterator k=theNet[j].begin(); !k.finished(); ++k) {
    if (*k != i) { // do not link to itself
      theNet[i][*k] = theNet(j)[*k]; // copy edge
      k.value() = 0; // remove old edge
    }
  }



  // copy connections from temporary storage -> j
  for (WeighedNodes::iterator k=tmp_nodes.begin(); !k.finished(); ++k) {
    if ( *k != j) {
      theNet[j][*k] = tmp_nodes[*k]; 
    }
  }
  
 }
// <--- switchConnections
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -               




















/* function switchLinkPairEnds */

template <typename NetType,  typename Generator>
size_t switchLinkPairEnds(NetType & theNet, Generator & generator, size_t netSize, size_t limit) {

  assert(limit <= netSize && 0<limit);
  size_t i,j, m, n, tries=0;
  bool expectConnected;
  do {
    ++tries;
    do { // choose edges i<->m and j<->n
      i=generator.next(netSize);
      j=generator.next(netSize);
      m=theNet(i).randKey(generator);
      n=theNet(j).randKey(generator);
    } while ( i==j || m == n || m==j || n==i                           // makes sure the switch is possible
	      ||  theNet(i)[n]!=typename NetType::EdgeData()   // there must not be connection between i<->n
	      ||  theNet(j)[m]!=typename NetType::EdgeData()); 

    
    if (theNet(i).size() == 1 && theNet(n).size() == 1 ||
	theNet(j).size() == 1 && theNet(m).size() == 1 ) { // its certain that the net breaks down
      expectConnected = false;
    }
    else {
      
      
      // make the switch
      theNet[i][n]=theNet(i)[m]; // add edge
      theNet[j][m]=theNet(j)[n]; 
      theNet[i][m]=0; // remove old
      theNet[j][n]=0;
      expectConnected = true;
      
      Dijkstrator<NetType> paths1(theNet,i); // start dijkstrator from i
      Dijkstrator<NetType> paths2(theNet,j); // start dijkstrator from j
      
      size_t  steps = 0;
      while ( !paths1.finished() && !paths2.finished() && steps < limit) { // find the smaller connected set 
	++paths1;
	++paths2;
	++steps;
      }
      
      
      expectConnected = true;
      if ( paths1.finished() || paths2.finished() ) {  // if a small disconnected subnet is found
	expectConnected = false;
      }
      if (  !expectConnected ) {
	// reverse the switch
	theNet[i][m]=theNet(i)[n]; // add edge
	theNet[j][n]=theNet(j)[m]; 
	theNet[i][n]=0; // remove old
	theNet[j][m]=0;
      }
    }
  }
  while (!expectConnected);

  return tries;
}

// <--- switchLinkPairEnds
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -               














/* function randomize() */
/* mixes the edges of the network, keeping the degree distribution intact */
 
template<typename NetType, typename Generator>
void randomize(NetType & net, Generator & generator, size_t rounds, size_t limit) {


  /* 
   To rearrange the edges of the network while keeping the degree
   distribution intact, we repeatedly pick two edges and switch their
   end nodes. 

   One round consists of making L switches, where L is the number of
   links in the network (a switch means swapping the end nodes of two
   edges). The number of rounds is given as input. 

   It is important to keep the network connected. Since checking the
   connectivity of the whole network is very time consuming, we use an
   adaptive procedure. It is based on the fact that a component that
   becomes disconnected in a switch is generally very small. 

   After switching two edges, we run Dijkstrator from each of them to
   see whether a small component broke apart from the network.  The
   parameter 'limit' determines to how many nodes we will try to find
   a path to from the starting node. If a disconnection is detected,
   the switch is immediately reversed, and we try a different switch.

   After L switches, we make a full connectivity check for the
   network.  If the network is found to be disconnected, we retrieve a
   backup of the network and redo the round. The parameter 'limit' is
   increased to make disconnection less likely on the next round. If
   the network is found to be connected, the parameter 'limit' has a
   small probability of decreasing (this probability is hard-coded). 
   The connectivity checking is thereby optimized. 

   Tests for answering the question 'How many rounds of scrambling should we do?'
   can be found in
      nets/Examples/testingRandomizer.info 
      nets/Examples/testingRandomizer.eps

   Starting with a network with strong community structure, we mixed
   it with randomizer different numbers of times, and detected all
   communities from the networks.  When community structure is 
   eliminated, we may assume that the network is properly scrambled.

   10 rounds should be adequate, 100 rounds plentiful. For the
   'limit' parameter, 15 is a good starting point. 

   Adaptive algorithm design and coding by JP and Jussi, composition
   of function, comments and tests by Riitta.
  */
 

 
  const size_t numLinks =  numberOfEdges(net); 
 
  float triesPerSwitch[rounds];  // Average number of tries before a single switch is accepted
  size_t tries;                                                                      
  
  size_t netSize=net.size();
  NetType backupNet(netSize); 
  bool disconnectionFound = false; 
  

  /* We will do altogether 'rounds*numLinks' switches. After 'numLinks' switches, 
     we check whether the network is still connected. If not, retrieve the backup and 
     start again. */

  std::cerr << "\nrandomize: Starting to randomize the network,\n" 
	    << "keeping the degree distribution intact.\n";  
  
  for (size_t i=0; i<rounds; ++i) { 
    
    copyNet(net,backupNet); // Take backup. If the network becomes disconnected,
	 		      // we will return to the previous backup 
    bool switchAgain; 


 
    do { 
      tries = 0;  
      for (size_t j=0; j<numLinks; ++j) {  
	tries += switchLinkPairEnds(net, generator, netSize, limit);
      }
      
      
      triesPerSwitch[i] = (float) tries/(float)numLinks; 
      // average number of tries before a single switch is accepted 
      
      
      /* If the net has become disconnected, adapt the parameters so that
	 it would be less likely to happen on the next round */
      if ( !ConnectivityCheck(net) ) { // if net is disconnected 
	std::cerr << "Disconnected, using backup. " 
		<< i+1 <<"/" << rounds << " Limit was: " << limit  <<"\n";
	copyNet(backupNet,net);
	switchAgain = true;
	limit += 5;    // increase the limit...
	disconnectionFound = true;
      }
      else {   // if net is connected 
	std::cerr << "Net OK " <<  i+1 <<"/" << rounds << " Limit was: " << limit <<"\n";
	switchAgain = false;
      // accept measured distributions and print them   
	if (disconnectionFound) {
	  if (generator.next(1.0) < 0.1 && limit > 1) {
	    limit -= 1;                             // decrease limit...
	  } }
	else { if (limit > 1) limit -= 1; }         // decrease limit... 
      }
    } while( switchAgain );

 

    
  }
   
  /*  std::cout << "%";
      for (size_t i=0; i<rounds; ++i) {
      std::cout << triesPerSwitch[i] << " ";
      }
      std::cout << "\n";
  */

  std::cerr << "Randomization finished.\n\n";

}



// <--- randomize
//  - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -               













#endif //~ RANDOMIZER_H
