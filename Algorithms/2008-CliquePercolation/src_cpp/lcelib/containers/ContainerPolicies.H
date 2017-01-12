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

#ifndef LCE_CONTAINER_POLICIES
#define LCE_CONTAINER_POLICIES

/**
 * The standard specializations for the policies for containers 
 * is defined here. This refers to the containers as whole, not to 
 * anytyhing contained. That is, if containers are contained, this is 
 * what is used. Uses CRTP.
 */

/**
 * This is the base for weight policies of weighed tables, that is, ones
 * for which you can get the weight out without going through all the 
 * elements.
 */

template<typename TableType>
struct WeightWeightPolicy {
  typedef typename TableType::WeightType WeightType;
  static WeightType getWeight(const TableType & subj) {
    return subj.weight();
  }
};

/**
 * For unweighed tables, this is what we probably mean...
 */

template<typename TableType>
struct CountWeightPolicy {
  /** 
   * If we do not store weights _explicitly_ we probably want to use
   * the number of elements as the weight. We do not give the direct
   * reference out, though. */
  typedef size_t WeightType;

  static WeightType getWeight(const TableType & subj) {
    return subj.elemsByCRTP();
  }
};





#endif
