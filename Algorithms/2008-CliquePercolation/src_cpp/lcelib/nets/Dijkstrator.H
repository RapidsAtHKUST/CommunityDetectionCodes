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
#ifndef DIJKSTRATOR
#define DIJKSTRATOR
#include "../Containers.H"
#include "../misc/FiboHeap.H"

/**
 * A policy for getting the edge weights out of the edges of
 * a network. This standard one simply treats the edge data as weights.
 * This should be useful for primitive types and types for which 
 * <, + -operators are sensibly defined. 
 *
 * JP: Check the NDEdgeData for useful specializations. 
 */

// template <typename EdgeData> 
// struct WeightPolicy {
//   /* This is the one to specialize away. Eg. "float" could be useful. */
//   typedef EdgeData WeightType;
//   /* And this should return the weight proper for a given edge */
//   static WeightType getWeight(const EdgeData & data) {return data;}
// };

/**
 * A class iteratively calculating shortest routes from a given
 * node to other ones reachable. Uses simple iterator syntax.
 * The sets are supposed to be hashes. They are so bloody general
 * that there should be no need to use anything else. The type of the heap
 * can, however, be specified.
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
 * Policy         A class telling what is the type for weights, and
 *                how to get them from edges.
 * Heap           A map-type data structure (weight->node index) 
 *                having methods for push, pop (smallest) and decreasing
 *                a weight.
 */

template <typename NetworkType,
          typename Policy=WeightPolicy<typename NetworkType::EdgeData>,
          template <typename, typename> class HeapType=FiboHeap> 
class Dijkstrator {
  typedef typename Policy::WeightType WeightType;
  typedef Dijkstrator<NetworkType, Policy, HeapType> MyType;

  struct RouteEnds {
    size_t source;
    size_t dest;
  };

  typedef HeapType<WeightType, RouteEnds> MyHeapType;
  typedef typename MyHeapType::NodeType HeapNodeType;
  typedef HeapNodeType * HeapNodePtr;
  typedef typename NetworkType::const_edge_iterator EdgeIter;
private:
  const NetworkType & myNet;
  MyHeapType myHeap;
  Set<size_t> found; /* The nodes, shortest route to which 
		      * have been found */
  Map<size_t, HeapNodePtr> candidates; 
  
  Dijkstrator() {}; /* Has to get the start node as a param */
  
  /* Variable needed for pointer syntax */
  
      
  struct RouteType {
    WeightType weight;
    RouteEnds ends;
    
    size_t getDest() const {return ends.dest;}
    size_t getSource() const {return ends.source;}
    WeightType getWeight() const {return weight;}
  };


  RouteType currRoute;

  bool done;
  
public:

  /**
   * Standard contructor taking the network and the start
   * node as parameters. The network should be ready when 
   * this is done, as the shortest route to any other node
   * is already calculated here.
   */

  Dijkstrator(const NetworkType & net, const size_t start): 
    myNet(net), done(false) {
    assert (myNet.contains(start));
    /* Standard constructors should be fine for internal structs. */

    /* We init the structures by simply putting the start node to the 
     * heap with weight zero, which should be the default.  */
    currRoute.ends.dest=start;
    currRoute.ends.source=start; /* No real need... */
    currRoute.weight=0;
    found.put(start);
    ++(*this); /* ... 'cause this should do away with it. */
  }
  
  /* Standard destructor should be fine. */
  
  /**
   * Moves the "iterator" to next shortest route.
   */

  MyType & operator++() {

    WeightType currWeight;
    WeightType prevWeight;

    assert (!done); /* Iteration not completed. */
    /* Iterate through the current node's siblings */
    for (EdgeIter iter=myNet(currRoute.getDest()).begin();
	 !iter.finished();
	 ++iter) {
      if (!found.contains(*iter)) { 
	/* No shortest path found, yet */
	currWeight=
	  currRoute.getWeight() + Policy::getWeight(iter.value());
	if (candidates.contains(*iter)) {
	  /* Some path already found, let's check which one is shorter  */
	  prevWeight=candidates[*iter]->getKey();
	  /* Expl:     hash map |      heap node     |  weight  */   
	  if (currWeight < prevWeight) {
	    /* Set new source: */
	    candidates[*iter]->value().source=currRoute.getDest();
	    myHeap.decreaseKey(candidates[*iter], currWeight);
	    /* Explanation as before */
	  } /* Else, nothing. Better one in the heap already. */
	} else { /* No route whatsoever found yet. Let's add one: */
	  RouteEnds ends;
	  ends.source=currRoute.getDest();
	  ends.dest=*iter;
	  candidates[*iter]=myHeap.push(currWeight, ends);
	}
      } else {
	/* Shortest path to node already found */
	assert (!candidates.contains(*iter));
      }
    } /* Edge iteration */
    
    /* Then, let's pop the new shortest route: */
    if (myHeap.finished()) {
      done=true;
    } else {
      WeightType oldWeight=currRoute.weight;     //Unused variable
      currRoute.weight=*myHeap;
      currRoute.ends=myHeap.value();
      ++myHeap;

      assert(oldWeight <= currRoute.weight);
   
      assert(candidates.isLegal());
      assert(candidates.contains(currRoute.ends.dest));
      candidates.remove(currRoute.ends.dest);
      found.put(currRoute.ends.dest);
    }
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

  const Set<size_t> & getFoundSet() const {
    return found;
  }

};

#endif

