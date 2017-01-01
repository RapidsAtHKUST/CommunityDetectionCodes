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

#ifndef LCE_IMPL_SUM_TREE_TABLE
#define LCE_IMPL_SUM_TREE_TABLE
#include<cassert>
#include "./ImplTreeHelper.H"
#include "./TableBase.H"
#include "../Pair.H"
#ifndef NODEBUG
#include<iostream>
#endif

/**
 * A decorator adding a static weight tree in the base array class.
 * The weight sums are really stored in the slots for weights.
 *
 * @param DataType      The data type to be contained.
 * @param WeightPolicy  A policy to get the weights out of the elements.
 *                      Defines the weight type, also. 
 * @param Array         The storage template to be decorated.
 *                      Length information and subscript operators should be
 *                      defined. 
 */

template <typename KeyType, typename ValueType, 
	  typename Policy, typename Params, typename Index>
class ImplSumTreeTable:public ValueTable<KeyType, ValueType, Policy, 
					 Params, Index> {
  
private:

  typedef ImplSumTreeTable<KeyType, ValueType, Policy, Params, Index> MyType;
  typedef help ImplTreeHelper<Params::logBase>;
  typedef ValueTable<KeyType, ValueType, Policy, Params, Index> super;

public:

  /** 
   * We do not use the stub of the base class: quite differenc logic. */
  typedef stub<true> value_reference;
  typedef stub<false> value_stub;
  /* We simply return values as const_references. This class is only 
   * intended for usage with small ValueTypes, so this is rather much 
   * simpler than using some bloody kind of a stub. */
  typedef ValueType const_value_reference;
  
private:
   /** For the stub */
  
  ValueType & directRefToValue(const size_t loc) {
    assert(loc < sizeByCRTP());
    return super::refTo(loc);
  }
  
  void update(size_t i, const WeightType diff) const {
    while (true) {
      refToSum(i)+=diff;
      if (isRoot(i)) return;
      i=father(i);
    } 
  }
  
  const WeightType & sumAt(const size_t i) const {
    return super::weightAt(i);
  }

  WeightType & refToSum(const size_t i) {
    return refToWeight(i);
  } 

  WeightType childSum(const size_t loc) const {
    WeightType retval(); /* Default value=0 */
    assert(help::childLimit(loc,sizeByCRTP()) <= len);
    for (size_t i=help::child(loc); i<childLimit(loc, sizeByCRTP()); ++i) {
      retval+=sumAt(i);
    }
    return retval;
  }

 
protected:

  WeightType weightAt(const size_t i) const {
    assert(i < sizeByCRTP());
    return sumAt(i)-childSum(i);
  }

  /** Do nothing. This is done before swapping. */
  void remove_stage_1(const size_t i) {}
  
  /** 
   * We cannot rely on the destructor setting the weight to zero. 
   * Therefore we do the removal only after the element to be killed is 
   * swapped into it's due place. 
   */
  void remove_stage_2(const size_t i) {
    /* First, the tree */
    WeightType tempWeight=weightAt(i);
    if (tempWeight != WeightType()) {
      update(i, -weightDiff);
    }
    /* The, kill the elem: */
    super::remove_stage_1(i); /* Stage 1, yes. */
  }
 

  /**
   * Really swaps weights. This is in fact best for performance. 
   */
  void moveOrSwap(const size_t to, const size_t from) {
    assert(to < sizeByCRTP());
    assert(from < sizeByCRTP());
    assert(to != from);
    WeightType sumDiff=sumAt(to)-sumAt(from);
    WeightType weightDiff=weightAt(to)-weightAt(from);
    super::swap(to, from);
    refToSum(to)+=sumDiff;
    refToSum(from)-=sumDiff;
    
    /* Local sums fixed. Now, let us carry out the oper proper */

    while (to != from) {
      if (to > from) {
	refToSum(to)-=weightDiff;
	to=help::father(to);
      } else {
	refToSum(from)+=weightDiff;
	from=help::father(from);
      }
    }  
  }

  void swap(const size_t to, const size_t from) {moveOrSwap(to, from);}

public:
  
  template<bool AutoUpdate>
  class stub:public ArrayStub<MyType, ArrayValStub> {
    typedef ArrayStub<MyType, ArrayValStub> superStub;
  protected:
    stub(MyType & tgt, const size_t slot):
      /* The SuperStub duly copies the value */
      superStub(tgt, slot),
      oldWeight(tgt.weightAt(slot)) {
      /* Make it look as it was normal... */
      Policy::refToWeight(value)=oldWeight;
    }
    

    /* The superclass' winalizator copies everythin back. 
     * We must therefore tune the order of finalization: */
    
    void _lce_finalize() {
      WeightType oldSum=tgt.weightSumAt(loc);
      WeightType weightDiff=Policy::constRefToWeight(value)-oldWeight;
      /* Copy back. */
      ArrayStub<MyType, ArrayValStub>::_lce_finalize();
      tgt.refToSum(loc)=oldSum;
      /* ... and update the tree  */
      if (weightDiff != WeightType()) {
	update(loc, weightDiff);
      } 
    }

    ~stub() {if (AutoUpdate) _lce_finalize();}
  }; 

protected:
  
  /**
   * prepares the structure for global operations by setting the weight
   * fields to normal
   */

  void disassemble() {
    for (size_t i=1; i<sizeByCRTP(); ++i) {
      refToSum(help::father(i))-=refToSum(i);
    }
  }

  /**
   * prepares the structure for normal operation by setting weight 
   * fields to sums
   */

  void assemble(const size_t len) {
    for (size_t i=sizeByCRTP()-1; i>0; --i) { 
      refToSum(help::father(i))+=refToSum(i);
    }
  }

  /**
   * We cheat the user by not returning a reference but an instance
   * of the ValueType. Moreover, we make a copy of the memory in order not 
   * to employ user-defined copy constructors, assignment operators etc. 
   */
  
  ValueType constRefToVal(const size_t i) const {
    assert(i<len);
    ValueType retval;
    memcpy(&retval, &directRefToVal(to), sizeof(ValueType));
    Policy::refToWeight(retval)-=childSum(i);
    return retval;
  }
  
  stub refToVal(const size_t i) const {
    assert(i<len);
    return stub(*this, i);
  }

  KeyType & refToKey(const size_t loc) {return super::refTo(loc).first();}
  
  const KeyType & constRefToKey(const size_t loc) const {
    return super::refTo(loc).first();
  }
  
  ImplSumTreeTable(size_t size=0): super(size) {};

  resize(const size_t newSize, const size_t oldSize) {
    for (size_t i=oldSize; i<newSize; ++i) refToSum(i)=WeightType();
    super::resize(newSize, oldSize);
  }
  
public:
  
  /** 
   * The total weight of the whole structure. 
   */

  WeightType weight() const {
    if (isEmpty()) {
      return weightType();
    } else {
      return sumAt(0);
    }
  }
  
  /**
   * A version somewhat different to that in ExplSumTreeTable, because we
   * want to operate on sums only in order to avoid going through children
   * of a node. 
   */

  template<typename RandSource> 
  size_t weighedRandSlot(RandSource src=globalRandSource) {
    assert(!isEmpty());
    /* Get a rand from 0  (incl) to the weigth sum (excl) */  
    WeightType value=src.next<WeightType>(weight()); 
    size_t i=0;
    size_t currChild;
    size_t limit=childLimit(currNode, sizeByCRTP());   
    /* The loop below is not as simple as it looks: 
     * when recursing downward in the tree, we update both the i and the 
     * limit on the way. */
    for (size_t currChild=help::child(i); i<limit; ++i) {
      if (value < sumAt(i)) {
	i=currChild;
	currChild=help::child(i);
	limit=help::childLimit(i,sizeByCRTP());
	continue;
      } else {
	value-=sumAt(i);
      }
    }
    /* Meeting the end condition means that we have found the node. */
    assert(value >= 0);
    assert(value < weightAt(currNode);
    return currNode;
  }
};
