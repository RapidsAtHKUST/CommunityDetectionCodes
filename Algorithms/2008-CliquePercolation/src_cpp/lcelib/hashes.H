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

#ifndef HASHES_H
#define HASHES_H
#include<limits>
#include<cassert>
#include"bits/packedhashcore.H" // The functionality lies here
#include"HashControllers.H"

/**
 * Common wrappers for hashes. Keys are passed by value, values by 
 * reference.
 *
 * IMPORTANT:
 *
 * Keys do not need to compare as equal when their hash values 
 * are the same. However, if keys DO compare as equal with == operator,
 * their hash values should be the same.
 *
 * Practically, for your own custom keys, do not use anything for 
 * calculating the hash values that you do not use for equality 
 * comparisons.
 *
 * This property is included in any hash implementation, albeit not
 * always documented very well.
 */


/**
 * The wrapper base class for objects to be put into a table. Not complete
 * yet: the hash value calculations cannot be specified at this level. 
 * 
 * RATIONALE: Put everything common to all hash keys into common base class.
 *
 * @param KeyType The data to be stored. 
 */

template<typename KeyType>
class HashKeyBase {
protected:
  KeyType myKey;
public:
  typedef KeyType HashKeyType;
  HashKeyBase(KeyType key): myKey(key) {}
  /** Key is not to be manipulated. */
  const KeyType & key() const {return myKey;}
  bool keyEquals(const HashKeyBase<KeyType> & cmp) const {
    return myKey==cmp.myKey;
  }
};

/**
 * The real hash key, suitable for integral values. 
 */

template<typename KeyType>
class HashKey: public HashKeyBase<KeyType> {
public:
  typedef HashKeyBase<KeyType> super;
  HashKey(const KeyType src=std::numeric_limits<KeyType>::max()): super(src) {}

  /* This might be specialized away... */
  IndexType getHashValue() const {return getHashValue(key());} 
  static IndexType getHashValue(const KeyType & src) {return src;} 
  
  /* These are used if the status type is void for the hash. 
   * If you do not know, don't worry. */

  void setAsUsed() const {}
  void setAsVirgin() {myKey=std::numeric_limits<KeyType>::max();}
  
  bool isInUse() const {return myKey!=std::numeric_limits<KeyType>::max();}
  bool isVirgin() const {return myKey==std::numeric_limits<KeyType>::max();}
};

/* Specialization for pointers. No sense in the hash "owning" it's 
 * keys, so the objects pointed to are not destroyed by destructor */

template<typename KeyType>
class HashKey<KeyType*>: public HashKeyBase<KeyType*> {
public:
  typedef HashKeyBase<KeyType*> super;
  HashKey(const KeyType src=0): super(src) {}

  /* This might be specialized away... */
  IndexType & getHashValue() const {return getHashValue(getKey);} 
  static IndexType & getHashValue(const KeyType & src) {
    return *(reinterpret_cast<IndexType *>(&src));
  }
    
  /* These are used if the status type is void. */
  
  void setAsUsed() const {}
  void setAsVirgin() {myKey=0;}
  
  bool isInUse() const {return myKey!=0;}
  bool isVirgin() const {return myKey==0;}
};

/* Specialization for key counters. The virgin status is marked
 * by count zero. */

template<typename KeyType> 
class HashKeyCounter: public HashKeyBase<KeyType> {
private:  
  IndexType count;
public:
  typedef HashKeyBase<KeyType> super;

  HashKeyCounter(const KeyType src=0): super(src), count(1) {}
  /* These are used if the status type is void. */

  void setAsUsed() const {}
  void setAsVirgin() {count=0;}

  bool isInUse() const {return count!=0;}
  bool isVirgin() const {return count==0;}

  IndexType getCount() {return count;}
  IndexType incrCount() {return count++;}
  IndexType decrCount() {return count--;}
  
};

template<typename KeyType, typename ValueType>
class HashPair: public HashKey<KeyType> {
protected:
  ValueType val;
public:
  typedef HashKey<KeyType> super;
  HashPair(): super() {}
  HashPair(const KeyType & key): super(key) {}
  HashPair(const KeyType & key, const ValueType & src): super(key), val(src) {}

  /* As far as I am concerned, the value can be manipulated from outside. */
  ValueType & value() {return val;} 

  /* This kind of overloading seems weird and to work.*/

  const ValueType & value() const {return val;}

};

/**
 * Simple set of keys. Does not hold duplicates: only first addition
 * of a key counts, and the key is gone after the first removal.
 */

template<typename KeyType, 
	 typename HashController=StandardHashController<>, 
	 typename StatusType=char>
class HashKeySet: public PackedHash<HashKey<KeyType>,HashController,
				    false, StatusType> {
public:
  typedef HashKeySet<KeyType, HashController, StatusType> MyType;
  typedef PackedHash<HashKey<KeyType>, HashController, false,StatusType> super;

  HashKeySet(IndexType size=0): super(size) {};
  
  /**
   * Puts a key into the table.
   *
   * return    Whether the key (one that compares equal) was already 
   *           present, and thereby overwritten
   */

  bool put(const KeyType & key) {
    //std::cerr << "Put:" << key << "\n";
    return int_put(HashKey<KeyType>(key));
  }

  /**
   * Template for putting contents of other sets in. Only simple
   * iterators are needed for the other set. The result is 
   * the union of these sets.
   */

  template<typename RightHandSet>
  MyType& operator+=(const RightHandSet & rval) {
    typename RightHandSet::iterator i=rval.begin();
    typename RightHandSet::iterator end=rval.end();
    for (;i!=end;i++) {
      put(*i);
    }
    return *this;
  }

  MyType & operator+=(const KeyType & rval) {
    put(rval);
    return *this;
  }

  /**
  * Removes a key from the table, if found.
  *
  * return    Whether found.
  */

  bool remove(const KeyType & key) {
    return int_remove(key);
  }

  MyType & operator-=(const KeyType & rval) {
    remove(rval);
    return *this;
  }
 
  /**
   *
   */

  template<typename RightHandSet>
  MyType& operator-=(const RightHandSet & rval) {
    typename RightHandSet::const_iterator i=rval.begin();
    typename RightHandSet::const_iterator end=rval.end();
    for (;i!=end;i++) {
      remove((*i).key());
    }
  }

  IndexType size() {return controller.getNumKeys();}

  class iterator: public super::iterator {
  public:
    iterator(MyType & source): super::iterator(source) {};
    iterator(MyType & source, bool): super::iterator(source, true) {}
    
    /**
     * The raison d'être for this class: returns a const reference to 
     * the key. Overrides the corresponding method of the base class.
     */

    const KeyType & operator*() {return super::operator*().key();}
  };

  iterator begin() {return iterator(*this);}
  iterator end() {return iterator(*this, true);}


  
};
  
template<typename KeyType,
	 typename HashController=StandardHashController<>,
	 typename StatusType=void>
class HashMultiKeySet: public PackedHash<HashKeyCounter<KeyType>, 
					 HashController, false, StatusType> {
  typedef PackedHash<HashKeyCounter<KeyType>, HashController, 
		     false, StatusType> super;
  typedef HashMultiKeySet<KeyType, HashController, StatusType> MyType;
private: 
  IndexType numKeyInsts;
public:
  HashMultiKeySet(IndexType size=0): super(size), numKeyInsts(0) {};

   /**
    * Returns how many keys were present before. Compatible with the boolean
    * return value in the standard set.
    */
  
  unsigned put(const KeyType & key) {
    IndexType loc;
    numKeyInsts++;
    if (placeToPut(location)) { /* Already there? */
      return table[location].incrKey();
    } 
      
    IndexType newSize;
    if (controller.aboutToPut(newSize)) { 
      rehash(newSize);
      int_put(HashKeyCounter<KeyType>(key));
    } else {
      putToLoc(HashKeyCounter<KeyType>(key));
    }
    return 0;
    
  }

  /**
   * Returns how many keys were present before. Compatible with the boolean
   * return value in the standard set.
   */

  unsigned remove(const KeyType & key) {
    IndexType loc;
    if (findFirst(key,loc)) {
      numKeyInsts--;
      assert(table[loc].getCount() > 0);
      if (table[loc].getCount==1) {
	removeFrom(place);
	return 1;
      } else {
	return table[loc].decrCount();
      }
    } else {
      return 0; /* Was not present. */
    }
  }
  
  /**
   * Returns how many keys are present. Guess what.
   */
  
  unsigned contains(const KeyType & key) const {
    IndexType loc;
    if (this.findFirst(key,loc)) {
      assert(table[loc].getCount() > 0);
      return (table[loc].getCount());
    } else return 0;
  }

  IndexType size() const {return numKeyInsts;}
  
  class iterator: public super::iterator {
  public:
    iterator(MyType & source): super::iterator(source) {};
    iterator(MyType & source, bool): super::iterator(source, true) {}
    
    /**
     * The raison d'être for this class: returns a const reference to 
     * the key. Overrides the corresponding method of the base class.
     */

    const KeyType & operator*() {return super::operator*().key();}
  };

  iterator begin() {return iterator(*this);}
  iterator end() {return iterator(*this, true);}

  class const_iterator: public super::const_iterator {
  public:
    const_iterator(MyType & source): super::const_iterator(source) {};
    const_iterator(MyType & source, bool):super::const_iterator(source,true) {}

    /**
     * The raison d'être for this class: returns a const reference to 
     * the key. Overrides the corresponding method of the base class.
     */

    const KeyType & operator*() {return super::operator*().key();}
  };

};

template <typename KeyType, typename ValueType,
	  typename HashController=StandardHashController<>,
	  typename StatusType=char>
class HashMap: public PackedHash<HashPair<KeyType, ValueType>, 
				 HashController, false, StatusType> {
public:
  typedef PackedHash<HashPair<KeyType, ValueType>, 
		     HashController, false, StatusType> super;
  typedef HashMap<KeyType, ValueType, HashController, StatusType> MyType;

  HashMap(IndexType size=0): super(size) {};
  
  IndexType size() const {return controller.getNumKeys();}
  
  /**
   * The array access operator works in the same way as for the maps
   * in the standard library. If the key is found, reference to it's value
   * is returned. Else, a new mapping is added for the key with value
   * initialized to default one, reference to which is then returned.
   */

  ValueType & operator[](const KeyType & key) {
    IndexType location;
    if (placeToPut(key, location)) {
      return table[location].value();
    } else {
      return putToLoc(HashPair<KeyType, ValueType>(key), location).value();
    }
  }

  /**
   * A less general implementation of the above operator,
   * intended for those interested in whether there was a key
   * already present.
   */

  bool put(const KeyType & key, const ValueType & value) {
    return int_put(HashPair<KeyType, ValueType>(key,value));
  }
  
  bool remove(const KeyType key) {
    return int_remove(key);
  }

  MyType & operator-=(const KeyType key) {
    int_remove(key);
    return *this;
  }

  /* The inherited iterators will do fine! */
  
};

template<typename ValueType> 
class HashVectorPair: public HashKeyBase<IndexType> {
  friend class HashVector::Stub;
  typedef HashKeyBase<IndexType> super;
  ValueType val;
public:
  HashVectorPair(const IndexType & key, const ValueType & value): 
    super(key), val(value) {}
    
  IndexType getHashValue() const {return getHashValue(key());} 
  static IndexType getHashValue(const KeyType & src) {return src;} 
  
  ValueType & value() {return val};
  void setValue(const ValueType & newValue) {value=newValue;}
  
  void setAsUsed() {}
  void setAsVirgin() {val=ValueType();}
  
  bool isVirgin() {return val==ValueType();}
  bool isInUse() {return val!=VAlueType();}
}

/**
 * A template specialization for boolean values. No values are 
 * really stored: if a key exists in the table, it has the value true.
 *
 * \warning Unlike the non-specialized template, this stores the
 * usage status in the key as defined in the base class. 
 */

template<typename KeyType> 
class HashVectorPair<KeyType, bool>: public HashKey<KeyType> {
public:
  HashVectorPair(const KeyType & key, bool): 
    super(key) {}

  bool value() {
    return isInUse();
  }

  void setValue(bool val) {
    if (val=true) {
      myKey=1; /* A magic value. Improbable for somebody denote
		* unused one by this. */
    } else {
      setAsVirgin();
    }
  }
}

/* A base class for decorating the hash vector by a sum of 
 * the edge values. The call conventions are such that 
 * if a value of an entry is altered, the old one is removed
 * and new one added. Thus, the interface can be used to extract higher
 * order statistics, also. */

template<typename ValueType> 
class HashSum {
private:
  ValueType mySum;
protected:  
  void addToSum(const ValueType & value) {mySum+=value;}
  void substractFromSum(const ValueType & value) {mySum-=value;}
public:
  const ValueType & sum() {return mySum;}
};

/**
 * A fake sum template, nicely accepting reports but doing nothing.
 * Should allow for empty base class optimization, 
 * 
 */

template<typename ValueType>
class FakeSum {
protected:
  void addToSum(const ValueType & value) {}
  void substractFromSum(const ValueType & value) {}
};


/**
 * A special kind of a hash map, treating elements not found in
 * the table as having theír class's default value. The
 * usage status is stored in this way, also. 
 */

template<typename ValueType, 
	 template<typename ValueType> class Sum=FakeSum
	 typename HashController=SmallHashController<> >
class HashVector: public PackedHash<HashVectorPair<IndexType, ValueType>, 
				    HashController, false>,
		  public Sum<ValueType> {
  typedef HashVector<ValueType, Sum, HashController> MyType;
  
  class Stub;
  friend class Stub;
  
public:

  HashVector(IndexType size=0): super(size) {};

  /**
   * A stub pretending to be a lvalue. 
   *
   * @param key The index in the vector
   * @return A stub that can be assigned to etc. 
   */
  
  Stub operator[](const IndexType key) {return Stub(*this, key)}

  /**
   * The array access operator returning rvalues.
   *
   * @param key The index in the vector
   * @return The corresponding value in the table, or the default
   *         value for the ValueType, if not found. 
   */

  ValueType operator[](const IndexType key) const {
    IndexType loc;
    aboutToPut(key,loc);
    /* This is the ingenious part: we can return the value
     * in the table, even if the corresponding element is not found!*/
    return table[loc].value(); 
  }
  
  template<typename SrcVal, typename SrcCont, typename SrcSum>
  MyType & operator+=(const HashVector<SrcVal, SrcCont, SrcSum>  & src) {
    RightHandVector::const_iterator end=src.end();
    for (RightHandVector::const_iterator i=src.begin();
	 i != end; i++) {
      operator[]((i*).key())+=(i*).value();
    }
  }

  template<typename SrcVal, typename SrcCont, typename SrcSum>
  MyType & operator-=(const HashVector<SrcVal, SrcCont, SrcSum>  & src) {
    RightHandVector::const_iterator end=src.end();
    for (RightHandVector::const_iterator i=src.begin();
	 i != end; i++) {
      operator[]((i*).key())-=(i*).value();
    }
  }
  
protected:
  /**
   * A stub class pretending to be a member of a vector.
   *
   * This should not be stored anywhere: only to be used
   * as a temporary variable in assignments of type \c vec[i]=value.
   * In this way, the destructor takes care of eliminating 
   * elements which are set to the default value.
   *
   * The compiler should be able to eliminate the unnecessary steps
   * in object creation/destruction.
   *
   */

  class Stub {
    friend class MyType; /* Just for allowing construction */
    /** The vector referred to */
    MyType & ref;
    /** The index in the vector */
    const IndexType myKey; /* Cannot assign to this. */
    /** Location in the hash or the place where to put a new value to */
    IndexType loc;
    /** Whether there was a corresponding element present when the
     * stub was created
     */
    bool found;
    
    /** Default contructor, not to be used */
    Stub() {}
    /** 
     * The constructor called by the non-const []-operator of the net. 
     * Either find a location for an existing element OR a place
     * to put a new one into.
     */

    Stub(MyType & src, IndexType key): ref(src), myKey(key), 
				       found(ref.placeToPut(key,loc)) {}
    
  public:
    
    /**
     * The destructor, removing elements not used. Behold:
     */

    ~Stub() {
      if (table.isInUse(loc) != found) { /* Something has changed... */
	if (found) {
	  /* The element has been removed... */
	  ref.controller.removed();
	  ref.fillSlot(loc);
	} else {
	  /* An element has been added */
	  controller.added();
	  table[loc].myKey=myKey;
	}
      }
    }

    /**
     * The assignment 
     */

    ValueType operator=(const ValueType & value) {
      ref.addToSum(value);
      ref.substractFromSum(table[loc].value());
      table[loc].setValue(value);
      return value;
    }
    
    ValueType operator+=(const ValueType & value) {
      return operator=(table[loc].value()+value);
    }

    ValueType operator-=(const ValueType & value) {
      return operator=(table[loc].value()-value);
    }

    ValueType operator*=(const ValueType & value) {
      return operator=(table[loc].value()*value);
    }

    ValueType operator/=(const ValueType & value) {
      return operator=(table[loc].value()/value);
    }
    
    operator ValueType() {
      return table[loc].value()
    }
  };
};

template<typename ValueType, typename FactorType, > {
    

	
      
	       

template<typename InputType, 
	 typename OutputType=unsigned, 
	 typename StatusType=char>
class KeyPacker: public  PackedHash<HashPair<InputType, OutputType>,  
				    SmallHashController<>, false> {
  /* No reason to allow any other hash controllers, as no keys shall be 
     removed. */
  typedef PackedHash<HashPair<InputType, OutputType>, 
		     SmallHashController<>, false> super;
  typedef KeyPacker<InputType, OutputType, StatusType> MyType;
public:
  KeyPacker(IndexType size=0): super(size) {};
  
  OutputType getPacked(InputType input) {
    IndexType loc;
    if (this->findFirst(input,loc)) {
      return data[loc];
    } else {      
      IndexType retval=controller.getNumKeys();
      int_put(HashPair<InputType, OutputType>(key, retval));
      return retval;
    }
  }

  OutputType getKeyCount() {
    return controller.getNumKeys();
  }
};



#endif    





