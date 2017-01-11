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

#ifndef LCE_ORDERED_ARRAY
#define LCE_ORDERED_ARRAY

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
class OrderedArray: 
  public Table<KeyType, ValueType, Policy, Params,  
	       OrderedArray<KeyType, ValueType, Policy, Params, Table> > {
  typedef OrderedArray<KeyType, ValueType, Policy, Params, Table> MyType;
  typedef Table<KeyType, ValueType, Policy, Params, MyType> super;

private:
  size_t length;
   
  /**
   * Finds either the first occurrence of a key or, if not found,
   * the following one according to the ordering relation.
   */

  size_t findFirstSlot(const KeyType & key) const {
    if (length==0) return 0;
    size_t startLoc=0;
    size_t lastLoc=length-1;
    //std::cerr << "Finding first:\n";
    while (lastLoc != startLoc) {
      size_t probeLoc=(startLoc+lastLoc) >> 1; /* /2, rounded down */
      //std::cerr << "Probe at:" << probeLoc;
      /* If the probe location incidentally holds an exactly equal key, 
       * we are not sure if it is the first occurrence and hence 
       * want to continue the search in the first half of the region of 
       * interest */
      if (Policy::firstSmaller(super::constRefToKey(probeLoc), key)) {
	//std::cerr << ", Last half\n";
	startLoc=probeLoc+1; /* Key larger to that at the probeLoc  */
      } else { 
	//std::cerr << ", First half\n";	
	/* Key smaller or equal to  that at the probeLoc */
	lastLoc=probeLoc; 
      }      
      //std::cerr << "Start:" << startLoc << ", limit" << lastLoc << "\n";
    } 
    /* Homed on a single element. */
    if (Policy::firstSmaller(super::constRefToKey(startLoc), key)) {
      startLoc++;
    }
    //std::cerr << "Done FF, at" << startLoc << " \n";
    return startLoc;
  }
  
protected:
  
  /** 
   * Do not care about the cap. 
   */

  OrderedArray(const size_t capacity=0): super(0), length(0) {}

  bool findFirst(const KeyType & key, size_t & loc) const {
    loc=findFirstSlot(key);
    //std::cerr << "FF returning, len" << length << "\n";  
    /* Following: length !=0 , short-circuit evaluation */
    return (loc<length && super::constRefToKey(loc) == key);
  }

  bool findNext(const KeyType & key, size_t & loc) const {
    return (++loc<length && super::constRefToKey(loc) == key);
  }

  bool forcedFind(const KeyType & key, size_t & loc) {
    //std::cerr << "Forced find: old len:" << length << ", key" << key;
    //std::cerr << "Old key/val pairs:\n";
    //for (size_t i=0; i<length ; ++i) {
    //std::cerr << super::constRefToKey(i) << ":" << constRefToVal(i) << "\n";
    //}

    loc=findFirstSlot(key);
    if (loc<length && super::constRefToKey(loc) == key) {
      //std::cerr << ", Found.\n";
      return true;
    }
    super::pushAt(loc);
    //assert(weightAt(loc)==0);
    ++length;
    
    //std::cerr << "New len:" << length;
    super::refToKey(loc)=key;
    return false;
  }

  /**
   * The function counts all the equal keys. 
   *
   * RATIONALE: One does not want to use this kind of structure when 
   * multiple equal keys are to be put in. All too costly: one should
   * consider an array of sets. 
   */

  size_t placeToPut(const KeyType & key, size_t & loc) {
    loc=findFirstSlot(key);
    size_t retval=0;
    while (loc<length && super::constRefToKey(loc) == key) {
      loc++;
      retval++;
    }
    super::pushAt(loc);
    ++length;
    setKeyAt(loc, key);
    return retval;
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
      return target->super::constRefToKey(loc);
    }
    
    bool finished() {return loc==target->size();}
  };

  friend class iterator;
  friend class const_iterator;

  iterator begin() {return iterator(this);}
  const_iterator begin() const {return const_iterator(this);}

  bool keyLegal(const KeyType & key) const {
    size_t loc;
    if (findFirst(key, loc)) {
      if ((loc>0) 
	  && (Policy::firstSmaller(super::constRefToKey(loc), super::constRefToKey(loc-1))
	      || (super::constRefToKey(loc) == super::constRefToKey(loc-1))))
	return false;
      if ((loc+1<length) 
	  && (Policy::firstSmaller(super::constRefToKey(loc+1), super::constRefToKey(loc))
	      || (super::constRefToKey(loc+1) == super::constRefToKey(loc))))
	return false;
      return super::localLegal(loc);
    }
    else return true;
  }  

  bool isLegal() const {
    for (size_t i=0; i<length; ++i) {
      if (!keyLegal(super::constRefToKey(i))) return false;
    }
    return super::isLegal();
  }

};
  

#endif
