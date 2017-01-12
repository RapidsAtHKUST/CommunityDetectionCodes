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

// lcelib/nets/models/ErdosRenyi.H
// 
// Functions related to the Erdos-Renyi network.
// (P.Erdös, A.Rényi: On Random Graphs, Publicationes Mathematicae 6, 290-297, 1959)


/*
 Functions included:  
 ---------------

 ErdosRenyi(net,netSize,k_ave,generator,optional:w0) 

    A version of the Erdos-Renyi model where the number of nodes N and
    their average degree k_ave are given as input. The number of edges
    is then calculated as N * k_ave and rounded upwards. Edges are
    placed randomly between vertex pairs. If a weight w0 is given, all
    edges will have weight w0. 


 ErdosRenyi2(net,p,generator,optional:w0)

   A version of the Erdos-Renyi model where the probability of an edge
   existing between a vertex pair is given as input If a weight w0 is
   given, all edges will have weight w0.
   Note: If NetType is SymmNet<bool>, any edge weights w0 which differ
   from zero are converted to true without warning.  For example, if
   the variable net is of type SymmNet<bool>, a function call
   ErdosRenyi(net, netSize, k_ave, generator, 0.4) will generate a
   network with unweighted edges, ignoring the '0.4'.

*/




#ifndef ERDOSRENYI_H
#define ERDOSRENYI_H
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




// ErdosRenyi ---->
/*   A version of the Erdos-Renyi model where the number of nodes N and
    their average degree k_ave are given as input. The number of edges
    is then calculated as N * k_ave and rounded upwards. Edges are
    placed randomly between vertex pairs. If a weight w0 is given, all
    edges will have weight w0. Feel free to write an improved version,
    or the version where the probability of an edge existing between a
    vertex pair is given as input :-)

 Note: If NetType is SymmNet<bool>, any edge weights w0 which differ
 from zero are converted to true without warning.  For example, if
 the variable net is of type SymmNet<bool>, a function call
 ErdosRenyi(net, netSize, k_ave, generator, 0.4) will generate a
 network with unweighted edges, ignoring the '0.4'.
*/

template<typename NetType, typename Generator>
void ErdosRenyi(NetType& net, size_t netSize, float k_ave, Generator & generator, typename NetType::EdgeData w0=1) {
  ClearNet(net,netSize); /* make sure there are no edges present to start with */
  const size_t m=(size_t) ceil((float) netSize * k_ave / 2.0 ); /* number of edges to be added */
  size_t i, j; 
  /* add m edges, avoiding loops and multiple edges */
  for (size_t k=0; k<m; ++k) {
    do {
      i=generator.next(netSize);
      j=generator.next(netSize);
      assert(i<netSize && j<netSize);
    }
    while ( (i==j) || net(i)[j]!=typename NetType::EdgeData() ); /* no loops of length 1, no multiple edges */
    assert(i!=j);
    net[i][j]=w0; /* add edge between nodes i and j */
  }
}
// <---- ErdosRenyi





// ErdosRenyi2 ---->
/*   A version of the Erdos-Renyi model where the probability of an
edge existing between a vertex pair is given as input If a weight w0
is given, all edges will have weight w0.

 Note: If NetType is SymmNet<bool>, any edge weights w0 which differ
 from zero are converted to true without warning.  For example, if the
 variable net is of type SymmNet<bool>, a function call
 ErdosRenyi(net, netSize, k_ave, generator, 0.4) will generate a
 network with unweighted edges, ignoring the '0.4'.
*/

template<typename NetType, typename Generator>
void ErdosRenyi2(NetType& net, float p, Generator & generator, typename NetType::EdgeData w0=1) {
  
  size_t N=net.size();
  ClearNet(net,N); /* make sure there are no edges present to start with */
  
  for(size_t i=0; i<N; ++i) {
    for(size_t j=i+1; j<N; ++j) { 
      float toss=generator.nextNormed(); // a random number uniformly from [0,1)
      if( toss  < p) { // create the link with prob p
	net[i][j]=w0; /* add edge between nodes i and j */
	// std::cerr << "generated link " << i << "-" << j << "\n";
      }
    }
  }
}
// <---- ErdosRenyi2


#endif //~ ERDOSRENYI_H

