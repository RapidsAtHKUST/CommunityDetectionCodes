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

#ifndef NUMSET2_H
#define NUMSET2_H

#include "../Containers.H"

// #include <list>
// #include <algorithm> 

class numSet2 {

private:

  Set<size_t> nums; 

public: 

  // these are public because I was not able to make the iterators work otherwise...
  // however, if not using iterators it is best to use the functions below...

  numSet2() {
  }

  ~numSet2() {
    //    delete nums;
  }

  // returns true, if vector contains val
  bool contains(const size_t val ) {
    return nums.contains(val);
  }

  // returns true, if the value was not in the list and puts the value to the list
  bool put( const size_t val  ) {
    return nums.put(val);
  }

  /*
  void push_back( size_t val  ) {
      nums.push_back( val );
  }
  */

  bool isEmpty() {
    return nums.size() == 0;
  }

  bool remove( size_t val ) {
    return nums.remove(val);
  }

  void clearAll() {
    std::vector<size_t> backup;
    for (Set<size_t>::iterator i=nums.begin(); !i.finished(); ++i) {
      backup.push_back(*i);
    }
    for (size_t i = 0; i < backup.size(); ++i) {
      nums.remove( backup.at(i) );
    }
    if ( nums.size() != 0 ) std::cerr << "problems when trying to clear numSet2\n";
  }

  // gives one element from the list and removes it

  size_t size() const {
    return nums.size();
  }

  Set<size_t>::iterator begin() {
    return nums.begin();
  }



  void operator= ( numSet2 & set2) {
    clearAll();
    for (Set<size_t>::iterator i = set2.begin(); !i.finished(); ++i) 
      nums.put(*i);
  }


};

#endif
