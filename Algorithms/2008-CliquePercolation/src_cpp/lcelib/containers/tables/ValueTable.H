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

#ifndef LCE_VALUE_TABLE
#define LCE_VALUE_TABLE
#include<cassert>
#include "../ArrayBase.H"
#include "./StubBase.H"
#include "../Pair.H"
#include "../ContainerPolicies.H"
#include "../WeightPolicy.H"
#ifndef NDEBUG
#include<iostream>
#endif

/**
 * This is the ValueTable that lurks behind each and every container 
 * implementation. Even if the value is void.
 */

template<typename KeyType, typename _ValueType, 
	 typename Policy, typename Params, typename Index>
class ValueTable:public ArrayBase<Pair<KeyType, _ValueType> > {
private:
  typedef ValueTable<KeyType, _ValueType, Policy, Params, Index> MyType;
  typedef ArrayBase<Pair<KeyType, _ValueType> > super;

public:
  typedef _ValueType ValueType;

  /** The Pair masks the void references away */
  typedef typename Pair<KeyType, ValueType>::second_reference  value_reference;
  typedef typename Pair<KeyType, ValueType>::const_second_reference 
                   const_value_reference;
  typedef typename Pair<KeyType, ValueType>::first_reference key_reference;
  typedef typename Pair<KeyType, ValueType>::const_first_reference 
                   const_key_reference;


  typedef ArrayStub<MyType, ArrayRefStub>  value_stub;
  friend class ArrayRefStub<MyType>;
  /* Bloody hell. This standard relly sucks. */
  //friend class value_stub;
  friend class ArrayStub<MyType, ArrayRefStub>;
  typedef typename Policy::WeightType WeightType;

  typedef CountWeightPolicy<MyType> DefaultWeightPolicy;

public: 

  /* Now, the CRTP buggers. Public so that you can define your own 
   * policies. */

  size_t sizeByCRTP() const {
    //std::cerr << "CRTP:" << ((Index *) this)->getTableSize() << "\n";
    if (!super::base_empty()) {
      return ((Index *) this)->getTableSize();
    } else {
      return 0;
    }
  }

  size_t elemsByCRTP() const {
    if (!super::base_empty()) {      
      return ((Index *) this)->size();
    } else {
      return 0;
    }
  } 

  size_t usedByCRTP(const size_t i) const {
    return ((Index *) this)->isUsed(i);
  }

protected: 
  
  /**
   * This uses CRTP, but is prtected because else you could break something.
   */
  bool isUsed(const size_t i) const {return ((Index *) this)->isUsed(i);}  

private: 
  /** Only for the stub. Your friends can access your private parts. */
  value_reference directRefToValue(const size_t slot) {
    assert(slot < sizeByCRTP());
    return super::refTo(slot).second();
  }

protected:

  ValueTable(const size_t size=0):super(size) {}

  value_reference refToVal(const size_t loc) {
    //assert(loc < sizeByCRTP());
    return super::refTo(loc).second();
  }

  const_value_reference constRefToVal(const size_t loc) const {
    //assert(loc < sizeByCRTP());
    return super::constRefTo(loc).second();
  }

  key_reference refToKey(const size_t loc) {
    //assert(loc < sizeByCRTP());
    return super::refTo(loc).first();
  }

  const_key_reference constRefToKey(const size_t loc) const {
    //assert(loc < sizeByCRTP());
    //std::cerr << "cRTK:" << loc 
    //      << "," << (super::constRefTo(loc).first()) << "\n";
    return super::constRefTo(loc).first();
  }

  WeightType weightAt(const size_t i) const {
    return Policy::getWeight(super::constRefTo(i).second());
  }

  /** Needed only by the implicit table */
  
  WeightType & refToWeight(const size_t i) {
    return Policy::refToWeight(super::refTo(i).second());
  }

  void moveOrSwap(const size_t to, const size_t from) {
    assert(to < sizeByCRTP());
    assert(from < sizeByCRTP());
    super::copy(to, from);
  }

  void swapValues(const size_t to, const size_t from) {
    /* IN order to avoid destructors etc... */
    char tempVal[sizeof(ValueType)];
    assert(to < sizeByCRTP());
    assert(from < sizeByCRTP());
    memcpy(tempVal, &refToVal(to), sizeof(ValueType));
    memcpy(&refToVal(to), &refToVal(from), sizeof(ValueType));
    memcpy(&refToVal(from), tempVal, sizeof(ValueType));
  }

  void removeVal(const size_t loc) {
    //std::cerr << "Removing...";
    super::refTo(loc).removeSecond();
    //std::cerr << "Done.\n";
  }

  void clearVal(const size_t loc) {
    //std::cerr << "Clearing.";
    super::refTo(loc).clearSecond();
    //std::cerr << "Done.\n";
  }

  
  
  /** This class does not need clearing as such. */

  void remove_stage_1(const size_t loc) {
    removeVal(loc);
  }

  
  void remove_stage_2(const size_t loc) {}

  /** 
   * Destroying the container. This should be OK for all the bases, too. 
   */

  void final_remove(const size_t loc) {
    //std::cerr << "Final...";
    super::refTo(loc).~Pair<KeyType, _ValueType>();
    //std::cerr << "Done.";
  }


  /* No need to specify the stub more accurately. */

  void swapVals(const size_t to, const size_t from) {
    ValueType tempValue;
    memcpy(&tempValue, &super::directRefTo(to), sizeof(ValueType));
    memcpy(&super::directRefTo(to),
	   &super::directRefTo(from),sizeof(ValueType));
    memcpy(&super::directRefTo(from), &tempValue, sizeof(ValueType));
  }

  /* The swap itself is inherited. */

  size_t weighedSlotSelect(WeightType val) const {
    assert(!super::base_empty());
    //std::cerr << "WSS. Val:" << val << "\n";
    for (size_t i=0; i<sizeByCRTP(); ++i) {
      if (usedByCRTP(i)) 
	if (val<weightAt(i)) return i;
	else val-=weightAt(i);
    }
    assert(false); /* Empty container? */
    return 0;
  }

  /** Nothing to do. */
  void assemble() {}
  /** Nothing to do. */
  void disassemble() {}

  bool localLegal(const size_t i) const {return true;}

  void setValue(const size_t loc, const_value_reference value) {
    refToVal(loc)=value;
  }

  bool isLegal() const {return true;}

  void pushAt(const size_t loc) {
    assert(loc < sizeByCRTP()+1);
    super::pushAt(loc, sizeByCRTP());
    //clearVal(loc);    
  }

  void pushToTail() {
    super::pushAt(sizeByCRTP(), sizeByCRTP());
  }

  void pullFrom(const size_t loc) {
    assert(loc < sizeByCRTP()); 
    //weightSum-=weightAt(loc);
    super::pullFrom(loc, sizeByCRTP());
  }
  
public:
  
  WeightType weight(size_t len) const {
    WeightType retval=WeightType();
    //std::cerr << "Start:" << retval << "Weights:\n";
    for (size_t i=0; i<len; ++i) {
      if (usedByCRTP(i))
	retval+=Policy::getWeight(constRefToVal(i));
      //std::cerr << i << ":";
      //std::cerr << Policy::getWeight(constRefToVal(i)) << " " << retval;
      //std::cerr << "\n";
    }
    return retval;
  }

  WeightType weight() const {return weight(sizeByCRTP());}

 //  template<typename RandSource>
//   size_t weighedRandSlot(RandSource & src=globalRandSource) const {
//     WeightType value=src.next(this->weight());
//     return weighedSlotSelect(value);
//   }

  /**
   * In order to avoid unnecessary calcs when the table is not full. 
   */
  
  template<typename RandSource>
  size_t weighedRandSlot(RandSource & src=globalRandSource) const {
    WeightType value=src.next(this->weight());
    //std::cerr << "Val for select:" << value << "\n";
    //std::cerr << "Slot:" << weighedSlotSelect(value) << "\n";
    return weighedSlotSelect(value);
  }
  
 
};

#endif
