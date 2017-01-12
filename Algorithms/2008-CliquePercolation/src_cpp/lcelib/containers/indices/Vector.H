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

#ifndef LCE_VECTOR
#define LCE_VECTOR
#include<cstring>
/**
 * The simplest container of 'em all. 
 */

template <typename KeyType, 
	  typename ValueType,
	  typename Policy,
	  typename Params,
	  template<typename, 
		   typename, 
		   typename,
		   typename,
		   typename
		   > class Table>
/* Default value makes tabs work in Emacs */
/* An inheritance definition from hell. For CRTP, we need the derived 
 * class which is the index, i.e, this. */
class Vector: 
  public Table<KeyType, ValueType, Policy, Params, 
	       Vector<KeyType, ValueType, Policy, Params, Table> > {
  typedef Vector<KeyType, ValueType, Policy, Params, Table> MyType;
  typedef Table<KeyType, ValueType, Policy, Params, MyType> super;
  
private:
  size_t length;
                             
protected:
  Vector(const size_t capacity=0): super(capacity), length(capacity) {}

  ~Vector() {
    for (size_t i=0; i<length; ++i) {
      super::final_remove(i);
    }
  }

public:
  bool isUsed(size_t i) {
    assert(i<length);
    return true;
  }

protected: 
  void removeAt(const size_t i) {
    super::remove_stage_1(i);
    super::remove_stage_2(i);
  }

  size_t forcedFind(const KeyType & key, size_t & loc) {
    loc=key;
    return true;
  }

  bool remove(const KeyType & key) {
    return removeAt(key);
  }

public:

  typedef typename super::const_value_reference const_value_reference;

  bool contains(const KeyType & key) const {
    return (key<length) && (key >= 0);
  }

  void trim() {}

  size_t size() const {
    return length;
  }

  /**
   * Resize and initialize with default value.
   * Lauri Kovanen, 20.8.2008
   */
  void resize(const size_t newSize) {
    super::resize(newSize, length); // Two parameter version
				    // initializes to default.
    length = newSize;
  }	       

  bool empty() const {return length==0;}
  size_t getTableSize() const {
    return length;
  }

  const_value_reference operator[](const KeyType & key) const {
    return constRefToVal(key);
  }

  template<typename RandSource>
  KeyType weighedRandKey(RandSource & src=globalRandSource) const {
    return weighedRandSlot(src);
  }	     

  class iterator {
  private: 
    iterator() {}
  protected:
    MyType * target;
    size_t loc;

    iterator(MyType * tgt): target(tgt), loc(0) {}
  public:
    iterator & operator++() {
      loc++;
      return *this;
    }

    KeyType operator*() {
      return loc;
    }

    bool finished() {return loc==target->size();}

    void remove() {
      target->removeAt(loc);
      loc++;
    }
  };

  class const_iterator {
  private: 
    const_iterator() {}
  protected:
    const MyType * target;
    size_t loc;

    const_iterator(const MyType * tgt): target(tgt), loc(0) {}
    
  public:
    const_iterator & operator++() {
      loc++;
      return *this;
    }

    KeyType operator*() {
      return loc;
    }

    bool finished() {return loc==target->size();}
  };
  

  friend class iterator;
  friend class const_iterator;
  
  iterator begin() {return iterator(this);}
  const_iterator begin() const {return const_iterator(this);}

  bool keyLegal(const KeyType & key) const {
    if (key < length) return super::localLegal(key);
    return false;
  }

  /* No own legality check. */

};

#endif
