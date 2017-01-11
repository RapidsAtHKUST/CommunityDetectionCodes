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

#include <cassert>
#ifndef UNWEIGHTED_DIJK
#define UNWEIGHTED_DIJK
#include "../hashes.H"

/**
 * Unweighed Dijkstra's algorithm, aiming towards O(E)-complexity.
 * 
 * A class iteratively calculating shortest routes from a given
 * node to other ones reachable. Uses simple iterator syntax.
 * The sets are supposed to be hashes. They are so bloody general
 * that there should be no need to use anything else. 
 * No heap is needed. 
 *
 * An instance of this class is specific to a given network and
 * start node. If eg. calculating statistics, feel free to construct 
 * a new instance for each round of iteration.
 *
 * The current shortest route can be reached by dereference operator *.
 * After initialization, * refers to the very shortest route from start
 * node, that is, to it's nearest neighbour. After that, the ++-operator
 * is used to iterate over other shortest routes from the start node in
 * an increasing order.
 *
 * As the first shortest route is already calculated in the initialization,
 * the network should be fully constructed before instantiating
 * this class for it.
 *
 * The shortest route -structure is defined in the end of this file.
 *
 * Template parameters:
 *
 * Network type   No need to say more. EdgeType should be defined, and
 *                access to edge maps and their iterators given.
 */

template <typename NetworkType>
class UnweighedDijk {
  typedef UnweighedDijk<NetworkType> MyType;
  typedef typename NetworkType::const_edge_iterator EdgeIter;
private:
  const NetworkType & myNet;
  HashKeySet<size_t> found; /* The nodes, shortest route to which 
		              	* have been found */
  HashKeySet<size_t> * fringe; /* The nodes corresponding to the 
				   * current shortest route */
  HashKeySet<size_t>::iterator fringeIter;
  HashKeySet<size_t> * nextFringe; /* The fringe to be constructed 
				       * for the next length */
  
  UnweighedDijk() {}; /* Has to get the start node as a param */
  
  /* Variable needed for pointer syntax */
  
  struct RouteType; /* Imperfect. See following for details */
  RouteType currRoute;

  bool done;
  
public:

  /**
   * Standard constructor taking the network and the start
   * node as parameters. The network should be ready when 
   * this is done, as the shortest route to any other node
   * is already calculated here.
   */

  UnweighedDijk(const NetworkType & net, const size_t start): 
    myNet(net), done(false) {
    assert (myNet.contains(start));
    /* Standard constructors should be fine for internal structs. */

    /* We init the structures by simply putting the start node to the 
     * heap with weight zero, which should be the default.  */
    currRoute.weight=1;
    found.put(start);
    fringe=new Set<size_t>;
    nextFringe=new Set<size_t>;
    for (EdgeIter iter=myNet.begin(start);
	 iter != end;
	 ++iter) {
      fringe->put(*iter);
    }
    fringeIter=fringe->begin();
    /* Just for checking for lonely nodes... */
    if (fringeIter==fringeEnd) {
      done=true;
    } else {
      currRoute.dest=*fringeIter;
    }
  }
  
  ~UnweighedDijk() {
    delete fringe;
    delete nextFringe;
  }

  /**
   * Moves the "iterator" to next shortest route.
   */


  MyType & operator++() {
    for (EdgeIter iter=myNet.begin(*fringeIter);
	 !iter.finished();
	 ++iter) {
      if (!(found.contains(*iter) ||
	    fringe->contains(*iter))) {
	nextFringe->put(*iter);
      } 
    }

    ++fringeIter;

    if (fringeIter.finished()) { 
      /* iteration over the fringe finished, time to move to the next 
       * length. */
      delete fringe;
      fringe=nextFringe;
      nextFringe=new Set<size_t>;
      fringeIter=fringe->begin();
      currRoute.weight++;
      if (fringeIter==fringeEnd) { /* Iteration finished for good... */
	done=true;
	return (*this);
      }
    }       

    currRoute.dest=*fringeIter;
    
    return (*this);
  }

  /**
   * Returns a reference to the route data structure representing
   * the current route. 
   */

  const RouteType & operator*() const {
    assert (!done);
    return currRoute;
  }
  
  bool finished() const {return done;}

    
  struct RouteType {
    size_t weight;
    size_t dest;
    
    size_t getDest() const {return dest;}
    size_t getWeight() const {return weight;}
  };
  

};

#endif




