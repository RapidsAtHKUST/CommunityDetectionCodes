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

// lcelib/nets/models/BBV.H
// 
// Functions related to the BBV model
// (From the article ...)  (fill in the blanks)

/* Algorithm: 

...

Parameters:  ...


// Usage example in    ...

Tested as follows:  ...

*/

// Author: Tapio Heimo


#ifndef BBV_H
#define BBV_H

#include "../../Containers.H"
#include "../../Nets.H"
#include "../../Randgens.H"
#include <cassert>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <ctime>


















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




#endif //~ BBV_H 
