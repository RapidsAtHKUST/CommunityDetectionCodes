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

#ifndef LCE_WEIGHT_POLICY
#define LCE_WEIGHT_POLICY

/** 
 * Policies for element weights in containers. Getting, and in the case
 * of implicit trees only storing sums, setting elements. 
 *
 * Don't worry about the setting part: it is only used with specific 
 * data structures. If you still do, just leave the correponding 
 * functions undefined. 
 *
 * You probably want to consider specializing the specialization
 * for container data types instead of this. 
 */

template<typename DataType> 
struct WeightPolicy {
  typedef DataType WeightType;
  /* The type of a reference to a weight */
  typedef DataType & weight_reference;
  
  static WeightType getWeight(const DataType & subj) {return subj;}
  
  static weight_reference refToWeight(DataType & subj) {
    return subj;
  }
};

#endif

