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

#ifndef LCE_EXPL_SUM_TREE_TABLE
#define LCE_EXPL_SUM_TREE_TABLE
#include<cassert>
#include "./ValueTable.H"
#include "./ImplTreeHelper.H"
#include "../Pair.H"
#ifndef NDEBUG
#include<iostream>
#endif

/**
 * A decorator adding a static weight tree in the base array class.
 * The weights are stored embedded in the array, and cannot directly 
 * be accessed from outside.
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
class ExplSumTreeTable:public ValueTable<Pair<KeyType, 
					      typename Policy::WeightType>,
					 ValueType, Policy, Params, Index> {
private:
  typedef ExplSumTreeTable<KeyType, ValueType, Policy, Params, Index> MyType;
  typedef ImplTreeHelper<Params::treeLogBase> help;
  typedef typename Policy::WeightType WeightType;
  typedef ValueTable<Pair<KeyType, WeightType>, ValueType, 
		     Policy, Params, Index> super;

  typedef typename Pair<KeyType, WeightType>::second_reference 
                   weigth_reference;
  typedef typename Pair<KeyType, WeightType>::const_second_reference 
  const_weigth_reference;
  typedef typename Pair<KeyType, WeightType>::first_reference key_reference;
  typedef typename Pair<KeyType, WeightType>::const_first_reference 
                   const_key_reference;

public:
  typedef WeightWeightPolicy<MyType> DefaultWeightPolicy;

private:

  /** For legality only.  */
  
  WeightType childSum(const size_t loc) const {
    WeightType retval=0;
    assert(help::childLimit(loc, super::sizeByCRTP()) <= super::sizeByCRTP());
    for (size_t i=help::child(loc); 
	 i<help::childLimit(loc, super::sizeByCRTP()); i++) {
      retval+=sumAt(i);
    }
    return retval;
  }

  
  void updateAt(size_t i, WeightType diff) {
    assert(i < super::sizeByCRTP());
    //std::cerr << i << " ";
    while (true) {
      refToSum(i)+=diff;
      //std::cerr << i << "," <<  childSum(i) << "," <<  sumAt(i) 
      //		<< "," << super::weightAt((i) << "\n";
      //assert(noCheck || localLegal(i));
      if (help::isRoot(i)) break;
      i=help::father(i);
      //std::cerr << i << " ";
    } 
    //std::cerr << "U";
  }
  
  WeightType & refToSum(const size_t i) {
    /*assert(i < sizeByCRTP());*/
    return super::refToKey(i).second();
  }

  WeightType sumAt(const size_t i) const {
    /*assert(i < sizeByCRTP());*/
    return (super::constRefToKey(i)).second();
  }
   
protected:

  key_reference refToKey(const size_t loc) {
    //assert(loc < sizeByCRTP());
    return super::refToKey(loc).first();
  }

  const_key_reference constRefToKey(const size_t loc) const {
    //assert(loc < sizeByCRTP());
    return super::constRefToKey(loc).first();
  }

  void disassemble() {
    super::disassemble();
    //std::cerr << "dizz";
    for (size_t i=0; i<super::sizeByCRTP(); ++i) {
      if (super::usedByCRTP(i))
	refToSum(i)=super::weightAt(i);
      else
	refToSum(i)=WeightType();
      refToSum(i)=super::weightAt(i);
    }
  }
  
  /**
   * After initial puts, run this. Assumes that everything has been init'd
   * to default values.
   */

  void assemble() {
    super::assemble();
    //std::cerr << "Assembling, size by crtp:" << sizeByCRTP() << "\n";
    /* We cannot substract 0 from zero */
    for (size_t i=super::sizeByCRTP(); i>1; --i) {
      //std::cerr << "Azzert";
      refToSum(help::father(i-1))+=sumAt(i-1);
      assert(localLegal(i-1));
    }
  }

  /**
   * In this case, the moveOrSwap swaps. This is due to the fact that
   * this is probably extremely fast, as updates do not generally need
   * to proceed far up the tree until a common ancestor is found.
   */

  void moveOrSwap(size_t to, size_t from) {
    assert(to < super::sizeByCRTP());
    assert(from < super::sizeByCRTP());
    assert(to != from);
    WeightType weightDiff=super::weightAt(to)-super::weightAt(from);
    /* We swap keys here and let the superclass swap the values. 
     * We use the memcpy as usual. */
    KeyType tempKey;
    memcpy(&tempKey, &refToKey(to), sizeof(KeyType));
    memcpy(&refToKey(to), &refToKey(from), sizeof(KeyType));
    memcpy(&refToKey(from), &tempKey, sizeof(KeyType));
    super::swapValues(to, from);

    /* Propagate the changes in the weight until a common ancestor
     * is found: above and including that, no changes are necessary.*/
    while (to != from) {
      if (to > from) {
	refToSum(to)-=weightDiff;
	assert(localLegal(to));
	to=help::father(to);
      } else {
	refToSum(from)+=weightDiff;
	assert(localLegal(from));
	from=help::father(from);
      }
    }
    assert(localLegal(from));
  }

  void swap(const size_t to, const size_t from) {
    moveOrSwap(to, from);
  }
 
  /* We did swap. */

 
  /** Do nothing. This is done before swapping. */
  void remove_stage_1(const size_t i) {}
  
  /** 
   * We cannot rely on the destructor setting the weight to zero. 
   * Therefore we do the removal only after the element to be killed is 
   * swapped into it's due place. 
   */
  void remove_stage_2(const size_t i) {
    /* First, the tree */
    WeightType tempWeight=super::weightAt(i);
    /* The, kill the elem completely in order to set weight to zero. 
     * We do it here, otherwise our assertions shall fail in the 
     * updateAt. 
     */
    super::removeVal(i);
    super::clearVal(i);
    if (tempWeight != WeightType()) {
      updateAt(i, -tempWeight);
    }
  } 

  void setValue(const size_t loc, const ValueType & value) {
    WeightType oldWeight=WeightType(); 
    if (super::usedByCRTP(loc)) oldWeight=super::weightAt(loc);
    super::setValue(loc, value);
    this->updateAt(loc, super::weightAt(loc)-oldWeight);
    assert(localLegal(loc));
  }

 
public:

  template<bool AutoUpdate>
  class stub:public super::value_stub {
    /* Blah. */
    //friend class MyType;
    friend class ExplSumTreeTable<KeyType, ValueType, Policy, Params, Index>;
    typedef typename super::value_stub super_stub;
  private:
    stub() {} /* Not to be called: always bound to a slot. */
    WeightType oldWeight;

    void _int_lce_update() {
      WeightType diff=
	((MyType &) super_stub::target).weightAt(super_stub::loc)-oldWeight;
      //std::cerr << "FI";
      if (diff != WeightType()) { /* Did change. */
	//std::cerr << "IN";
	((MyType &) super_stub::target).updateAt(super_stub::loc, diff);
	//std::cerr << "OUT";
	//assert(((MyType &) target).localLegal(loc));
      }
    }

  protected:
    stub(MyType & tgt, const size_t i):
      super_stub(tgt, i), 
      oldWeight(tgt.weightAt(i)) {
      //std::cerr << "Stubified..";
      //std::cerr << "Stubify at:" << i << ", oldWeight:" << oldWeight
      //	<< ", oldSum" << tgt.constRefToSum(i) << "...";
    }

    void _lce_update() {
      _int_lce_update(); 
      oldWeight=((MyType &) super_stub::target).weightAt(super_stub::loc);
      super_stub::_lce_update();
    }

    void _lce_finalize() {
      _int_lce_update();
      super_stub::_lce_finalize();
    } 
    
    bool _lce_upToDate() {
      if (oldWeight != 
	  ((MyType &) super_stub::target).weightAt(super_stub::loc)) {
	return false;
      } else {
	return super_stub::_lce_upToDate();
      }
    }
    
  public:
    
    ValueType & operator=(const ValueType & src) {
      return super_stub::operator=(src);
    }

    template<typename SourceType> 
    ValueType & operator=(const SourceType & src) {
      //std::cerr << "Copy constructing a _Stub_";
      return super_stub::operator=(src);
    }

    ~stub() {
      if (AutoUpdate) { 
	//std::cerr << "Fin stub...";
	_lce_finalize();
	//std::cerr << "done\n";
      }  
    }
  };

  //friend class stub<true>;
  //friend class stub<false>;

public:
  typedef stub<true> value_reference;
  /* const_value_reference is inherited OK. */
  typedef stub<false> value_stub;


protected:

  value_reference refToVal(const size_t i) {
    //std::cerr << "ESTT:S rTV. WTF? OMG! LOL!";
    return stub<true>(*this, i);
  }

  ExplSumTreeTable(size_t size=0): super(size) {
    for (size_t i=0; i<size; ++i) {
    refToSum(i)=WeightType();
    //clearVal(i);
    }
  };
  
  bool localLegal(const size_t loc) const {
    /* Redundant information should be OK. */
    WeightType weight=WeightType();
    if (super::usedByCRTP(loc)) weight=super::weightAt(loc);
    if (childSum(loc)+weight==sumAt(loc)) {
      return super::localLegal(loc); 
    } else {
      std::cerr << "Sum tree not legal. Loc:" << loc 
		<<  " childSum:" << childSum(loc) 
      	<< ", local weight:" << super::weightAt(loc) << ", sum:"
      	<< sumAt(loc) << "\n";
      return false;
    }
  }

  bool isLegal() const {
    for (size_t i=0; i<super::sizeByCRTP(); i++) {
      if (!localLegal(i)) return false;
    }
    if (weight()!=super::weight()) return false;
    return super::isLegal();
  }

  /* Use resize in ArrayBase directly. (Lauri, 20081023)
  void resize(const size_t newSize, const size_t oldSize) {
    super::resize(newSize, oldSize, true);
    for (size_t i=oldSize; i<newSize; ++i) refToSum(i)=WeightType();
   }
  */

public:
  
  WeightType weight() const {
    if (super::base_empty()) {
      return WeightType();
    } else {
      return sumAt(0);
    }
  }
  
  /**
   * Select a slot from the table according to the cumulative sum value 
   * given as a parameter. Perhaps the most ingenious recursion the Author 
   * has ever written. No length is really needed as a parameter, so
   * it is eliminated in the case that assertions are off. 
   */

  template<typename RandSource>
  size_t weighedRandSlot(RandSource & src) const {
    assert(!super::base_empty());
    assert(weight() != WeightType());
    //assert(isLegal());
    /* Get a rand from 0  (incl) to the weigth sum (excl) */  
    WeightType value=src.next(weight());//(/*weight()*/); 
    //std::cerr << "Selecting, value=" << value << "\n";
    //std::cerr << "Table looks like:\n";
    //for (size_t i=0; i< super::sizeByCRTP(); ++i) {
    //std::cerr << i << ":" << super::weightAt(i) << "," << sumAt(i) << "\n";
    //} 
    size_t i=0; /* Start from the root */
    while (!(value < super::weightAt(i))) { /* Clearer this way. */
      if (value < sumAt(i)) {
	value-=super::weightAt(i);
	i=help::child(i); /* Found the subtree */
	//std::cerr << i << ", left:" << value << "\n";
      } else {
	value-=sumAt(i); /* Next subtree */
	++i;
	//std::cerr << i << ", left:" << value << "\n";
      }
      assert(i < super::sizeByCRTP());
    }
    //std::cerr << "Selected:" << i << "\n";
    return i;
  }
  
};

/**
 * As the weight a duly defined, we probably want to use it as the
 * weight.
 */

template<typename KeyType, typename ValueType, 
	 typename Policy, typename Params, typename Index>
struct WeightPolicy<ExplSumTreeTable<KeyType, ValueType, 
				     Policy, Params, Index> >:
  public WeightWeightPolicy<ExplSumTreeTable<KeyType, ValueType, 
					     Policy, Params, Index> > {};

#endif

