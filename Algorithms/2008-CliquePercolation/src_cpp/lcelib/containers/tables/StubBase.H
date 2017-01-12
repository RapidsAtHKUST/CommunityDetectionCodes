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

#ifndef LCE_STUB_BASE
#define LCE_STUB_BASE
#include<cstring> /* For memcpy */


/**
 * We only want the elementary functions to be in one place. Not the
 * ultimate base class, however, as the method for getting references 
 * varies. The superclass gives them out by ref-operation. They are always
 * references, perhaps to pointers.
 * 
 * RATIONALE: Having methods in one place makes the life simpler. The downside 
 * is that we need a three-layered structure. 
 */

template<typename ArrayType, template<typename> class SuperStub>
class ArrayStub: protected SuperStub<ArrayType> {
  typedef SuperStub<ArrayType> super;
  typedef typename super::reference reference;
  typedef typename super::ValueType ValueType;
  typedef typename super::pointer pointer;
  typedef ArrayStub<ArrayType, SuperStub> MyType;
protected:
  ArrayStub(ArrayType & tgt, const size_t slot):
    SuperStub<ArrayType>(tgt, slot) {
  } 
public:

  operator reference() {return (super::ref());}
  
  template<typename SourceType>
  reference operator=(const SourceType & src) {
    return (super::ref())=src;
  }

  template<typename SourceType>
  reference operator+=(const SourceType & src) {
    //std::cerr << "+=:ing..";
    (super::ref())+=src;
    //std::cerr << "Done+=\n";
    return (super::ref());
  }

  template<typename SourceType>
  reference operator-=(const SourceType & src) {
    return (super::ref())-=src;
  }

  template<typename SourceType>
  reference operator*=(const SourceType & src) {
    return (super::ref())*=src;
  }

  template<typename SourceType>
  reference operator/=(const SourceType & src) {
    return (super::ref())/=src;
  }

  template<typename SourceType>
  reference operator%=(const SourceType & src) {
    return (super::ref())%=src;
  }

  template<typename SourceType>
  reference operator<<=(const SourceType & src) {
    return (super::ref())<<=src;
  }

  template<typename SourceType>
  reference operator>>=(const SourceType & src) {
    return (super::ref())>>=src;
  }

  template<typename SourceType>
  reference operator|=(const SourceType & src) {
    return (super::ref())|=src;
  }
 
  template<typename SourceType>
  reference operator^=(const SourceType & src) {
    return (super::ref())^=src;
  }
  
  template<typename SourceType>
  reference operator&=(const SourceType & src) {
    return (super::ref())&=src;
  }
  
  reference operator++() {
    return ++(super::ref());
  }

  ValueType operator++(int) {
    return (super::ref())++;
  }

  reference operator--() {
    return --(super::ref());
  }

  ValueType operator--(int) {
    return (super::ref())--;
  }

  pointer operator->() {
    return super::pnt();
  }

  /**
   * This is intended for safety and the possibility to implement
   * generic algorithms operating for both stubs and direct access. 
   * 
   * Thus, (&ref)->sth should always work. 
   */

  MyType & operator&() { 
    return *this;
  }

};

/** 
 * The ultimate base class for array stubs giving out references 
 * to the underlying array. */

template<typename ArrayType>
class ArrayRefStub {
private:
  ArrayRefStub(); /* Always bound. */
protected:
  typedef typename ArrayType::ValueType ValueType;
  typedef ValueType & reference;
  typedef ValueType * pointer;

  ArrayType & target;

  const size_t loc;

  ArrayRefStub(ArrayType & tgt, size_t slot): target(tgt), loc(slot) {}

  reference ref() {return target.directRefToValue(loc);}
  pointer pnt() {return &ref();}

  void _lce_finalize() {}
  void _lce_update() {}
  bool _lce_upToDate() {return true;}

public:
  
  ValueType & operator=(const ValueType & src) {
    ref()=src;
  }

};

/** 
 * The ultimate base class for array stubs giving out references 
 * to the value stored here, not directly to the table.  
 */

template<typename ArrayType>
class ArrayValStub {
private:
  ArrayValStub(); /* Always bound. */
protected:
  typedef typename ArrayType::ValueType ValueType;
  typedef ValueType & reference;
  typedef ValueType * pointer;
private:
  ValueType value;
protected:
  ArrayType & target;
  const size_t loc;
  ArrayValStub(ArrayType & tgt, const size_t slot): target(tgt), loc(slot) {
    memcpy(&value, &(target.directRefTo(slot)), sizeof(ValueType));
  }

  reference ref() {return value;}
  pointer pnt() {return &ref();}
  
  void _lce_finalize() { /* Write back. */
    memcpy(&(target.directRefTo(loc)), &value, &sizeof(ArrayType::ValueType));
  }
  /** 
   * No real difference at this level.
   */
  void _lce_update() {_lce_finalize();}
  bool _lce_upToDate() {return value==target.directRefTo(loc);}
};

/**
 * The real stub for arrays. ArrayType self-expl, BaseStub defines 
 * how the array is accessed. 
 */

// template<typename ArrayType, template<typename> class BaseStub>
// class ArrayStub:public StubBase<BaseStub<ArrayType> > {
// private:
//   ArrayStub();
// protected:
//   //ArrayStub(ArrayType & tgt, const size_t slot):
//   ArrayStub(const size_t slot, ArrayType & tgt):
//     StubBase<BaseStub<ArrayType> >(tgt, slot) {
//     std::cerr << "{" << loc << "," << tgt.constRefToKey(loc) << "}";
//     std::cerr << "FOO!";
//   }
// };

#endif
