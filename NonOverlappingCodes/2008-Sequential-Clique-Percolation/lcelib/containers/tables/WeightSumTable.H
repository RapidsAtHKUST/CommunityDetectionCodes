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

#ifndef LCE_WEIGHT_SUM_TABLE
#define LCE_WEIGHT_SUM_TABLE
#include<cassert>
#include "../WeightPolicy.H"
#include "./ValueTable.H"
#include "../ContainerPolicies.H"
#include "../WeightPolicy.H"
#ifndef NDEBUG
#include<iostream>
#endif

/**
 * A decorator adding the total weigth into an array class. 
 *
 * @param DataType      The data type to be contained.
 * @param WeightPolicy  A policy to get the weights out of the elements.
 *                      Defines the weight type, also. 
 * @param Array         The storage template to be decorated.
 *                      Length information and subscript operators should be
 *                      defined. 
 */

template<typename KeyType, typename ValueType, typename Policy, 
	 typename Params, typename Index>
class WeightSumTable:
  public ValueTable<KeyType, ValueType, Policy, Params, Index> {
private:
  typedef WeightSumTable<KeyType, ValueType, Policy, Params, Index> MyType;
  typedef ValueTable<KeyType, ValueType, Policy, Params, Index> super;

public:
  typedef typename Policy::WeightType WeightType;
  typedef WeightWeightPolicy<MyType> DefaultWeightPolicy;
  
private:

  WeightType weightSum;

protected:

  /** Before swapping or overwriting */

  void remove_stage_1(const size_t loc) {
    assert(loc < super::sizeByCRTP());
    weightSum-=super::weightAt(loc);
    super::removeVal(loc);
    //std::cerr << "In WST:s r_s_1, loc:" << loc << "\n";
  }

  void remove_stage_2(const size_t loc) {
    assert(loc < super::sizeByCRTP());
    super::clearVal(loc);
    //std::cerr << "In WST:s r_s_2, loc:" << loc << "\n";
    assert(super::weightAt(loc)==WeightType());
  }

  /* Stage 2 does nothing with ValueTable. */

  void setValue(const size_t loc, const ValueType & value) {
    //std::cerr << "Setting:";
    assert(isLegal());
    WeightType oldWeight=super::weightAt(loc);
    super::setValue(loc, value);
    weightSum+=(super::weightAt(loc)-oldWeight);
    assert(isLegal());
  }

  void pushAt(const size_t loc) {
    assert(loc < super::sizeByCRTP()+1);
    super::pushAt(loc, super::sizeByCRTP());
    super::clearVal(loc);    
  }

  void pushToTail() {
    super::pushAt(super::sizeByCRTP(), super::sizeByCRTP());
    super::clearVal(super::loc);
  }

  void pullFrom(const size_t loc) {
    assert(loc < super::sizeByCRTP()); 
    weightSum-=super::weightAt(loc);
    super::pullFrom(loc, super::sizeByCRTP());
  }
  
public:

  template<bool AutoFinalize>
  class stub:public super::value_stub {
    /* I hate this. Results in non-portable code. */
    //friend class MyType;
    friend class WeightSumTable<KeyType, ValueType, Policy, Params, Index>;
  private:
    stub() {}
    WeightType oldWeight;
    
  protected:
    stub(MyType & tgt, const size_t i):
      super::value_stub(tgt, i), 
      oldWeight(tgt.weightAt(i)) {
    }
    
    void _lce_finalize() {
      //std::cerr << "Finalize. Old sum:" << ((MyType &) target).weightSum;
      //std::cerr << ", old Weight:" << oldWeight;
      //std::cerr << ", new Weight:";

      // OLD WeightType newWeight=((MyType &) super::target).weightAt(super::loc);
      WeightType newWeight=((MyType &) super::value_stub::target).weightAt(super::value_stub::loc);

      //std::cerr << newWeight;

      //OLD: ((MyType &) super::target).weightSum+=(newWeight-oldWeight);
      ((MyType &) super::value_stub::target).weightSum+=(newWeight-oldWeight);

      //std::cerr << "new sum:" << ((MyType &) target).weightSum << "\n";
      oldWeight=newWeight;
      super::value_stub::_lce_finalize();

      //OLD: assert (((MyType &) super::target).isLegal());
      assert (((MyType &) super::value_stub::target).isLegal());

      //std::cerr << "Foo";
    }

    void _lce_update() {
      WeightType newWeight=((MyType &) super::target).weightAt(super::loc);
      ((MyType &) super::target).weightSum+=(newWeight-oldWeight);
      oldWeight=newWeight;
      super::value_stub::_lce_update(); 
      //std::cerr << "Updating _with update_";
    }
    
  public:
    ValueType & operator=(const ValueType & src) {
      return super::value_stub::operator=(src);
    }

    ~stub() {
      if (AutoFinalize) {
	_lce_finalize();
	//std::cerr << "Stub out\n";
      }  
    }
  };

  
public:
  typedef stub<true> value_reference;
  /* const_value_reference is inherited OK. */
  typedef stub<false> value_stub;
  

  friend class stub<true>;
  friend class stub<false>;

  value_reference refToVal(const size_t i) {
    //std::cerr << "Stub in...";
    //std::cerr << "Initing a stub. Old Sum:" << weightSum 
    //      << ", according to super:" << super::weight()
    //      << ", old weight:" << super::weightAt(i) << "\n";  
	      
    return stub<true>(*this, i);
  }
  
  WeightSumTable(size_t size=0): super(size), weightSum() {
    /* Might be only needed for assertions. */
    for (size_t i=0; i<size; ++i) super::clearVal(i);
  };
  
  WeightType weight() const {return weightSum;}

  bool isLegal() const {
    //if (weight() != super::weight()) { 
    //std::cerr << "WeightSumTable:isLegal:\n"
    //	<< "   Weight differs from super:s count. \n"
    //	<< "   My:" << weight() << ", super:" << super::weight() 
    //	<< "\n"; 
    //return false;
    //}
    //else 
    return super::isLegal();
  }

  /* Literally the same as in superclass, but now the weight() returns
   * the precalc'd value. */

  template<typename RandSource>
  size_t weighedRandSlot(RandSource & src) const {
    WeightType value=src.next(weight());
    //std::cerr << "Selecting val " << value << " of " << weight() << "\n";
    return weighedSlotSelect(value);
  }
};


/**
 * As the weight a duly defined, we probably want to use it as the
 * weight.
 */

template<typename KeyType, typename ValueType, 
	 typename Policy, typename Params, typename Index>
struct WeightPolicy<WeightSumTable<KeyType, ValueType, 
				   Policy, Params, Index> >:
  public WeightWeightPolicy<WeightSumTable<KeyType, ValueType, 
					   Policy, Params, Index> > {};



#endif
