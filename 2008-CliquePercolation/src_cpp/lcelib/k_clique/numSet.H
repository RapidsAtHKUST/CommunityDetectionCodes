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

#ifndef NUMSET_H
#define NUMSET_H

#include <list>
#include <algorithm> 

class numSet {

private:

  std::list<size_t> nums; 

public: 

  // these are public because I was not able to make the iterators work otherwise...
  // however, if not using iterators it is best to use the functions below...

  numSet() {
   nums.clear(); // make empty vector
  }

  ~numSet() {
    //    delete nums;
  }

  // returns true, if vector contains val
  bool contains( size_t val ) {
    return find( nums.begin(), nums.end(), val) != nums.end();
  }

  // returns true, if the value was not in the list and puts the value to the list
  bool put( size_t val  ) {
    if ( !contains(val) ) {
      nums.push_back( val );
      return true;
    }
    else return false;
  }

  void push_back( size_t val  ) {
      nums.push_back( val );
  }


  bool isEmpty() {
    return nums.empty();
  }

  void clearAll() {
    nums.clear();
  }

  // gives one element from the list and removes it
  size_t pop() {
    size_t rval = nums.back(); // remember what was the last value
    nums.pop_back(); // remove val
    return rval;
  }


  size_t size() const {
    return nums.size();
  }

  std::list<size_t>::iterator begin() {
    return nums.begin();
  }

  std::list<size_t>::iterator end() {
    return nums.end();
  }




};

#endif
