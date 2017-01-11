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

#ifndef LCE_ARRAY
#define LCE_ARRAY

template<typename KeyType, 
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
class Array: 
  public Table<KeyType, ValueType, Policy, Params,  
	       Array<KeyType, ValueType, Policy, Params, Table> > {
  typedef Array<KeyType, ValueType, Policy, Params, Table> MyType;
  typedef Table<KeyType, ValueType, Policy, Params, MyType> super;

private:
  size_t length;
   
  /**
   * Finds either the first occurrence of a key or, if not found,
   * the following one according to the ordering relation.
   */
  
protected:
  
  /** 
   * Do not care about the cap. 
   */

  Array(const size_t capacity=0): super(0), length(0) {}


  bool findFirst(const KeyType & key, size_t & loc) const {
    for (loc=0; loc<length; ++loc) {
      if (key == super::constRefToKey(loc)) {
	return true;
      }
    }
    return false;
  }

  bool findNext(const KeyType & key, size_t & loc) const {
    return (++loc<length && super::constRefToKey(loc) == key);
  }

  bool forcedFind(const KeyType & key, size_t & loc) {
    if (findFirst(key,loc)) return true;
    super::pushToTail();
    assert(loc==length);
    super::refToKey(length++)=key;
    return false;
  }

  /**
   */

  void placeToPut(const KeyType & key, size_t & loc) {
    super::pushToTail();
    loc=length++;
    super::refToKey(loc)=key;
  }

  void removeFrom(size_t loc) {
    super::remove_stage_1(loc);
    super::remove_stage_2(loc);
    super::pullFrom(loc);
    --length;
  } 

  bool remove(const KeyType & key) {
    size_t loc;
    if (findFirst(key, loc)) {
      removeFrom(loc);
      return true;
    } else {
      return false;
    }
  }

  /* Nothing to be trimmed. */
  void trim() {}

  template<typename AuxType>
  AuxType * auxData() {
    return (AuxType *) &length;
  }

public:

  size_t size() const {return length;}
  bool empty() const {return length==0;}
  size_t getTableSize() const {
    return length;
  }

  bool contains(const KeyType & key) const {
    size_t loc;
    return findFirst(key, loc);
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
      ++loc;
      return (*this);
    }

    const KeyType & operator*() {
      return target->constRefToKey(loc);
    }

    bool finished() {return loc==target->size();}

    void remove() {target->removeFrom(loc);}
  };

  class const_iterator {
  private: 
    const_iterator() {}
  protected:
    const MyType * target;
    size_t loc;
    /* No const-qualifier to MyType here: otherwise */
    const_iterator(const MyType * tgt): target(tgt), loc(0) {}
  public:
    const_iterator & operator++() {
      ++loc;
      return (*this);
    }

    const KeyType & operator*() {
      return target->constRefToKey(loc);
    }
    
    bool finished() {return loc==target->size();}
  };

  friend class iterator;
  friend class const_iterator;

  iterator begin() {return iterator(this);}
  const_iterator begin() const {return const_iterator(this);}

  bool keyLegal(const KeyType & key) const {
    size_t loc;
    if (findFirst(key, loc)) return super::localLegal(loc);
    return true;
  }  

  bool isLegal() const {
    return super::isLegal();
  }

};
  

#endif
