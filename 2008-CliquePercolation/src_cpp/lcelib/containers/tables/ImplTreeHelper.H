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

#ifndef LCE_IMPL_TREE_HELPER
#define LCE_IMPL_TREE_HELPER
#include<cassert>

template<size_t logBase>
struct ImplTreeHelper {
  
  static bool isRoot(const size_t i) {return i==0;} 

  /** 
   * The father in the implicit tree. /base is implemented as bit shift.
   */

  static size_t father(const size_t i) {return (i-1) >> logBase;}
  
  /**
   * The smallest child in the implicit tree. *base is impleneted as bit shift.
   */

  static size_t child(const size_t i) {
    return (i << logBase) + 1;
  }

  /** 
   * Convenience operator for iterating over children of a node
    * in the implicit tree. Returns the index of the last child
   * or the length of the array, whichever smaller. For-iteration
   * can start form the first child and proceed to limit given by this
   * function;
   *
   */

  static size_t childLimit(const size_t i, const size_t len) {
    return child(i+1)<len ? child(i+1) : len;
  }
};

/**
 * Original bug only applicable to 2. Now as a specialization: at least
 * this works.
 */

template <>
struct ImplTreeHelper<1> {
  
  static bool isRoot(const size_t i) {return i==0;} 

  /** 
   * The father in the implicit tree. /base is implemented as bit shift.
   */

  static size_t father(const size_t i) {return ((i+1) >> 1)-1;}
  
  /**
   * The smallest child in the implicit tree. *base is impleneted as bit shift.
   */

  static size_t child(const size_t i) {
    return ((i+1) << 1) - 1;
    /* The real meaning: j' = j * base => i'+1 = (i+1) * base
     * => i'= (i+1)*base-1 => i'=((i+1) << logBase) - 1*/
  }

  /** 
   * Convenience operator for iterating over children of a node
    * in the implicit tree. Returns the index of the last child
   * or the length of the array, whichever smaller. For-iteration
   * can start form the first child and proceed to limit given by this
   * function;
   *
   */

  static size_t childLimit(const size_t i, const size_t len) {
    return child(i+1)<len ? child(i+1) : len;
  }
};

#endif
