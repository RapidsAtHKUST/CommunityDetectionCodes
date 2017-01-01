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

#ifndef MULTIITER_H
#define MULTIITER_H

#include <list>
#include "numSet.H"
#include <iostream>
#include "../Containers.H"

typedef std::list<size_t> nodeList;
typedef Set<size_t> nodeSet;

class multiIter {

private:
  numSet nodes;
  std::vector<nodeList::iterator> iters; // we can not use Set<size_t>::iterator because vector of it does not compile
  size_t maxLevel;
  bool found_end, initialized, validIters;
public:
  
  multiIter(const nodeSet & inputNodes, const size_t numIters) {
    if ( numIters <= inputNodes.size() && numIters > 0 ) {
      for (nodeSet::const_iterator i = inputNodes.begin(); !i.finished(); ++i) {
	nodes.push_back( *i );
      }
      // nodes = inputNodes;
      iters.resize(numIters);
      maxLevel = numIters-1;
      found_end = false;
      initialized = false;
      validIters = true;
    }
    else {
      validIters = false;
      std::cerr << "too many iterators for the list!\n";
    }
  }

  // moves the iterators by one 
  // returns true if the last element was found
  bool next( std::vector<size_t> & curr_nodes) {
    if ( validIters ) {
      if ( !initialized ) {
	iters.at(0) = nodes.begin();
	if ( maxLevel > 0 ) {  // more than one iterator
	  for ( size_t currIterLevel = 1; currIterLevel <= maxLevel; ++currIterLevel) {
	    iters.at(currIterLevel) = iters.at(currIterLevel-1); // move to the next element
	    iters.at(currIterLevel)++;
	  }
	}
	curr_nodes.clear();
	for ( size_t i = 0; i <= maxLevel; ++i) curr_nodes.push_back( (*(iters.at(i))) );
	initialized = true;
	return found_end;
      }
      else {
	size_t fullLevels = 0;
	iters.at(maxLevel)++;
	
	if ( levelFull(maxLevel) ) {
	  fullLevels++;
	  while ( fullLevels <= maxLevel && initItersFromLevel(maxLevel-fullLevels) ) {
	    fullLevels++;
	  }
	  //       moveIterAtLevel(maxLevel-downIndex);
	}
	
	if ( fullLevels > maxLevel ) { // we found the end
	  found_end = true;
	}
	if ( !found_end ) {
	  curr_nodes.clear();
	  for ( size_t i = 0; i <= maxLevel; ++i) curr_nodes.push_back( (*(iters.at(i))) );
	}
	return found_end;
      }
    }
    else return true;
  }





  bool levelFull(size_t levelNum) {
    if ( iters.at(levelNum) !=  nodes.end() ) return false;
    else return true;
  }

  bool moveIterAtLevel(const size_t level) {
    iters.at(level)++;
    return !levelFull(level); // return true if the level is not full yet
  }

  bool initItersFromLevel(const size_t level) {
    iters.at(level)++;
    for ( size_t currIterLevel = level+1; currIterLevel <= maxLevel; ++currIterLevel) {
      iters.at(currIterLevel) = iters.at(currIterLevel-1);
      iters.at(currIterLevel)++; // move to the next
    }
    return levelFull(maxLevel); // return true if the last level is still full
  }
  


};
#endif
