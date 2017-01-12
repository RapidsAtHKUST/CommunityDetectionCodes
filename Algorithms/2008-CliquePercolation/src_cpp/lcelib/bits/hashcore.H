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

#ifndef HASH_CORE_H
#define HASH_CORE_H
#include<cassert> 
#include<cmath>
#ifndef NODEBUG
#include<iostream>
#endif

/**
 * Core class template for simple hash implementations. 
 *
 * Linear probing is utilized instead of linked lists. This should
 * become fashionable as the probing is inexpensive with modern processors,
 * whereas following pointers can be _very_ expensive due to the cache
 * miss latency. Space might also be saved and the iteration of elements 
 * becomes very fast, as only sequential memory access is needed.
 *
 * Keys are input and returned by value, values by reference. 
 *
 * Iterators defined here are for key sets. 
 */

static const unsigned char STATUS_VIRGIN=0;
static const unsigned char STATUS_INUSE=1;
static const unsigned char STATUS_REMOVED=2;

static const unsigned char ACTION_VOID=0;
static const unsigned char ACTION_LOCAL_REHASH=1;
static const unsigned char ACTION_REHASH_IN_PLACE=2;
static const unsigned char ACTION_REHASH_TO_SIZE=3;

typedef unsigned int IndexType;  

/******************* Key traits ****************/

/**
 * These traits define the way in which the key type
 * is transformed to an integer type. No hashing policy
 * here.
 */

template <typename KeyType> 
struct HashKeyTraits {
  static IndexType getKeyValue(KeyType key) {return key;}
};

/**
 * For pointers, we need cast
 */

template<typename KeyType> 
struct HashKeyTraits<KeyType*> {
  static IndexType getKeyValue(KeyType * key) {
    return reinterpret_cast<IndexType>(key);
  }
};

/******************* Value traits ****************/

/* In order to implement iterators here, we need 
 * traits for values returned by reference. There cannot
 * be references to void, so */

template<typename ValueType>
struct HashValueTraits {
  typedef ValueType& ref_type;
  typedef const ValueType& const_ref_type;
  typedef ValueType* pointer_type;
};

template<>
struct HashValueTraits<void> {
  typedef void ref_type;
  typedef void const_ref_type;
  typedef void pointer_type;
};

/****************** Data containers **************/

/**
 * The data container for maps and sets of objects not suitable 
 * as keys in themselves. The reason for templatizing this
 * will be given in the spezialization after this:
 */

template<typename DataType>
class ArrayDataContainer {
  typedef ArrayDataContainer<DataType> MyType;
private:
  DataType * valArray;  
  ArrayDataContainer() {};  
public:
  ArrayDataContainer(IndexType size) {
    valArray=new DataType[size];
#ifndef  NDEBUG
    //std::cerr << "Inited a data array"; 
#endif
  }
  ~ArrayDataContainer() {delete[] valArray;}  
  /**
   * The assignment operator deletes the current array, and marks the
   * arrays of the source to null, so that when the source is destroyed,
   * our data is not deleted.
   */
  MyType& operator=(MyType& src) {
    delete[] valArray;
    valArray=src.valArray;
    src.valArray=0;
    return *this;
  }

  typename HashValueTraits<DataType>::ref_type operator[](const IndexType i) {
    return valArray[i];
  }

  typename HashValueTraits<DataType>::const_ref_type 
  operator[](const IndexType i) const {
    return valArray[i];
  }

  /* Cannot generalize previous for dumb container, so:*/
  void moveElemTo(const IndexType from, 
		  MyType& target, 
		  const IndexType to) {
    target.valArray[to]=valArray[from];
  }
  
  void prefetchAt(const IndexType loc) const {
#ifdef GNU_PREFETCH
    __builtin_prefetch(&valArray[loc],1,1);
#endif
  }
};

/**
 * Spezialization for type void, that is, no data. Simple key sets
 * use this: allows the usage of the same code for the hash itself
 * regardless of whether data is stored or not. All the function calls
 * below are should be removed by the compiler.
 */ 
template<>
class ArrayDataContainer<void> {  
  typedef ArrayDataContainer<void> MyType;
private: 
  ArrayDataContainer();
public:
  ArrayDataContainer(IndexType size) {} 
  
  HashValueTraits<void>::ref_type operator[](const IndexType i) {}

  /* If you know how to overload the operator[] for this,
   * let me know. */
  void moveElemTo(const IndexType from, 
		  const MyType& target, 
		  const IndexType to) const {}  
  
  void prefetchAt(const IndexType loc) const {}

  
};
/******************* Hash controllers *****************/

/**
 * Controller base class only storing the table size 
 */

class HashTableSize {
private:
  IndexType tableSize;  
protected:
  HashTableSize(IndexType size): tableSize(size) {}
  HashTableSize() {
    //std::cerr << "In default constr. of HtS!";
  } 
public:
  IndexType getTableSize() const {return tableSize;}
};

/**
 * Linear probing and modulo calculations implemented here. 
 * Not usable as itself, but just as a base class.
 */

template<typename KeyType, unsigned int primeFactor=29>
class Pow2LinProbHashController {
  typedef HashKeyTraits<KeyType> myTraits;
private:
  Pow2LinProbHashController() {}
  IndexType moduloMask;
protected:
  Pow2LinProbHashController(IndexType size): moduloMask(size-1) {
    assert((size & (size-1)) == 0); // Power of 2     
  }
public:
  IndexType getNextPlace(const IndexType place) const {
    /* No assertion: might be useful not to. */
    return (place+1) & moduloMask;    
  }
  IndexType getPrevPlace(const IndexType place) const {
    return (place-1) & moduloMask;
  }
  IndexType getInitPlace(const KeyType key) const {
    return (myTraits::getKeyValue(key)*primeFactor) & moduloMask;
  }
};

/**
 * Counter maintenance here. Make a subclass implementing the 
 * hashing policy, and combine with a probing and modulo 
 * class for a functional one.
 */

class HashCounter: public HashTableSize {
private:
  IndexType numVirgs;
  IndexType numKeys;
  HashCounter() {
    //std::cerr << "In default constr. of HC!";
  }  
protected:
  IndexType getNumVirgs() const {return numVirgs;}
  IndexType getNumRemoved() const {
    return getTableSize()-getNumVirgs()-getNumKeys();
  }
  HashCounter(IndexType size): HashTableSize(size), 
			       numVirgs(size), 
			       numKeys(0) {} 
public:
  
  IndexType getNumKeys() const {return numKeys;}
  void markedVirgin() {numVirgs++;}
  /**
   * Whether there are elements marked as removed in the table. 
   * If you don't know, answer no: otherwise, an infinite loop might
   * be created.
   */
  bool removedElemsLeft() const {return (getNumRemoved()>0);}

  void wroteOn(const unsigned char status) {
    assert(status==STATUS_VIRGIN || status==STATUS_REMOVED);
    if (status==STATUS_VIRGIN) numVirgs--;
    numKeys++;
  }
  
  void removedKey() {numKeys--;}
   
  /**
   * Legality checks. The form is this because the controller does
   * not necessarily maintain counters: just answer OK if you 
   * don't know.
   */

  bool virginsLegal(IndexType compVal) const {
    //std::cerr << "vrg:" << getNumVirgs() << "\n";
    return (compVal==getNumVirgs());}
  bool usedLegal(IndexType compVal) const {return (compVal==getNumKeys());}
  bool removedLegal(IndexType compVal) const {
    return (compVal==getNumRemoved());
  }
};

/**
 * Rehash policy proper. This one uses local rehashing. 
 */

template <typename KeyType, unsigned int prime=29,
	  unsigned char maxUsedPerc=80,
	  unsigned char minVirgPerc=10, /* 20 might induce oscillations */
	  unsigned char minFillPerc=30> 
class StandardHashController: public HashCounter, 
			    public Pow2LinProbHashController<KeyType,prime> {  
private:  
  static const IndexType minSize=8;
  IndexType minVirgins;
  IndexType minKeys;
  IndexType maxKeys;
  StandardHashController() {
    //std::cerr << "In default constructor of SHC\n";
  }
public:
  StandardHashController(IndexType size=minSize): 
    HashCounter(size), 
    Pow2LinProbHashController<KeyType, prime>(size) {
    assert(size >= minSize);
    //std::cerr << "Table size:" << (this->getTableSize()) << "\n";
    minVirgins=static_cast<IndexType>(ceil(this->getTableSize()
					   *(minVirgPerc/100.0F))); 
    maxKeys=static_cast<IndexType>(floor(this->getTableSize()
					 *(maxUsedPerc/100.0F)));  
    minKeys=static_cast<IndexType>(floor(this->getTableSize()
					 *(minFillPerc/100.0F)));   
    //std::cerr << minVirgins << "," << maxKeys << "," << minKeys << "\n";
  }
  
  
  unsigned char justRemoved(IndexType & newSize) {
    this->removedKey(); // Counter maintenance
    if ((this->getNumKeys() <= minKeys) && (this->getTableSize() > minSize)) {
      newSize=this->getTableSize() >> 1; // i.e. /2
      return ACTION_REHASH_TO_SIZE;
    } 
    return ACTION_LOCAL_REHASH;
  }
  
  bool aboutToPut(IndexType & newSize) { 
    assert(getNumKeys() <= maxKeys);
    assert(getNumVirgs() >= minVirgins);
    if (getNumKeys() ==  maxKeys) {
      //std::cerr << "Table in full size, recommending rehash to size:";
      newSize=this->getTableSize() << 1; // i.e. *2
      //std::cerr << newSize << "\n";
      return true;
    } else if (getNumVirgs() == minVirgins) {
      //std::cerr << "Too few virgins, recommending rehash to current size.\n";
      newSize=this->getTableSize();
      return true;
    }    
    return false;
  }
};

/******************** The hash table ********************/

/*
 * For explanations for hash tables, linear probing etc. consult 
 * Knuth's book. "Keys" and "Values" should not be interpreted 
 * strictly: former just contains data needed for placement and
 * the latter additional data that is bound to the respective key.
 *
 * @param KeyType     The key type. 
 * @param ValueType   Value type. If "void", no storage is allocated
 *                    and manipulation code is eliminated compile-time
 * @param HashPolicy  The class handling rehashes, counters, probing
 *                    and modulo operations.
 * 
 */

template <typename KeyType,   
	  typename ValueType, 
	  typename HashController, 
	  bool ALLOW_DUPLICATES=false> 
class Hash {
  typedef Hash<KeyType, ValueType, HashController, 
	       ALLOW_DUPLICATES> MyType;
private: 
   
  KeyType * keyTable;
  unsigned char * status;

  /**
   * Rehashes the structure into a new one of specified size, then copies
   * it into this.
   * 
   * Fixme: use iterators.
   */
  
  void rehash(const IndexType size) {
    MyType newHash(size);
    IndexType newPlace;
    //std::cerr << "Rehashing to size:" << size << "\n";
    for (IndexType i=0; i<controller.getTableSize(); i++) {
      if (status[i] == STATUS_INUSE) {
	newHash.putKey(keyTable[i], newPlace);
	data.moveElemTo(i, newHash.data, newPlace); 
      }
    }
    *this=newHash;    
  }

  /**
   * The core of a local rehash operation. Tries to fill a slot 
   * marked as "removed" by moving in the next applicable element in the
   * probe sequence. If successful, returns "true" and the 
   * place of the next element with "removed" status in the probe 
   * sequence, which might be the one generated by the move
   * operation performed. If not, the original place just gets marked 
   * as virgin. If probe sequence contained additional removed elements,
   * the first of them is returned by reference as above. If not,
   * the return value is "false" and the value returned by 
   * reference contains the number of the first unchecked element,
   * not necessarily a removed one.
   *
   * The controller is kept up to date about the status changes.
   *
   * Now matter the conditions for returning from this method,
   * the table will be in a legal state after the call.
   * 
   * NB. Probably only applicable with linear probing.
   *
   * The author has previously implemented a quite different version
   * of this method for Syslore Oy. No IP problems here.
   * 
   * @param place The location of the "removed" slot to be filled
   *              or marked "virgin". The location of the next
   *              slot to be filled is returned by reference, if 
   *              one found, otherwise the first element not
   *              inspected in the probe seq.
   *
   * @return      Whether a new place to be filled is found
   *              in the probe sequence.
   */

  bool fillSlot(IndexType & emptyPlace) {
    
    IndexType toBeFilled;
    IndexType currSlot;
    IndexType initPlace;
    bool remFound=false; /* Whether another removed element is found 
			  * in the probe sequence. Used for the return
			  * value */
    
    /* This one should only be called for slots with STATUS_REMOVED */
    toBeFilled=emptyPlace;
    assert(status[toBeFilled]==STATUS_REMOVED);
    currSlot=controller.getNextPlace(toBeFilled);
    
    /* Let us find an element to fill the slot with: */
    while (status[currSlot] != STATUS_VIRGIN) {
      if (status[currSlot] == STATUS_INUSE) {
	assert(keyFoundAt(currSlot));
	initPlace=controller.getInitPlace(keyTable[currSlot]);
	/* Now, we must figure out whether we can use the key 
	 * at "currSlot" to fill the empty place. For linear probing
	 * that is assumed here it suffices to check whether the 
	 * initial place for the key is before the empty slot. 
	 * Simple comparison of values is not enough, however, 
	 * because either of the two probes concerned (this one
	 * or the one corresponding to the put operation 
	 * of the element in question) 
	 * can have rotated back to the beginning of the table. */
      
	bool currRotated=(initPlace > currSlot); 
	bool probeRotated=(toBeFilled > currSlot);
      
	if (((probeRotated == currRotated) && // The "normal" case: 
	     (initPlace <= toBeFilled)) ||     // move by comparison. 
	    (currRotated && !probeRotated)) { // OR just current element did, 
	                                      // no need to even compare.      
	  /* We may move the element to the empty place */
	  //std::cerr << currSlot << "->" << toBeFilled << ":" << initPlace << "\n"; 
	  keyTable[toBeFilled]=keyTable[currSlot];
	  data.moveElemTo(currSlot, data, toBeFilled);
	  status[toBeFilled]=STATUS_INUSE;
	  status[currSlot]=STATUS_REMOVED;
	  assert(keyFoundAt(toBeFilled));
	  assert(!keyFoundAt(currSlot));
	  /* No need to update the counters. */
	  if (!remFound) {
	    emptyPlace=currSlot;
	  } /* Otherwise, returns the place of the first "removed" slot */
	  return true; /* For a new call to the new empty place */
	} /* Else, continue the search. */
      } else {
	assert(status[currSlot]==STATUS_REMOVED);
	/* The current slot is marked as removed. It is probably 
	 * to be filled in a successive call: */
	if (!remFound) {
	  remFound=true;
	  emptyPlace=currSlot;
	} // Else it already contains a location previous to this.
      }      
      currSlot=controller.getNextPlace(currSlot);
    } // loop
    /* Status: found a virgin element. No suitable replacement found. */
    status[toBeFilled]=STATUS_VIRGIN;
    controller.markedVirgin(); // counter update
    if (!remFound) {
      /* No removed elements seen, rehash over for the 
       * probe sequence. The value to be returned by reference
       * is the place from which to continue a possible search
       * for elements with removed status: */
      emptyPlace=controller.getNextPlace(currSlot);      
    }
  
    return remFound;;
  }

  /**
   * Empties the table of removed elements by successive element
   * moves.
   */

  void rehashLocally() {
    IndexType place=0;
    while (controller.removedElemsLeft()) {
      /* Search for next elem with status:removed*/
      while (status[place] != STATUS_REMOVED) {
	place=controller.getNextPlace(place);
      }
      while (fillSlot(place)); // Place is updated by reference 
    }
  }

protected:
 
  HashController controller;

  ArrayDataContainer<ValueType> data; /* With void data, this
				       * does not really exist */ 
  
  /**
   * Default constructor when called with no arguments
   */

  explicit Hash(IndexType size=8): controller(size), data(size) {
    keyTable=new KeyType[size]; 
    status=new unsigned char[size];
    for (IndexType i=0; i<size; status[i++]=STATUS_VIRGIN);
    assert(isLegal());
#ifndef  NDEBUG
    //std::cerr << "Successfully inited a core of size" << size << "\n"; 
#endif
  } 
  
  ~Hash() {
#ifndef  NDEBUG
  //std::cerr << "Deleting a core. Key table null: " << (keyTable==0) << "\n"; 
#endif
    delete[] keyTable;
    delete[] status;
#ifndef  NDEBUG
    //std::cerr << "Success!"; 
#endif 
  }

  /**
   * Assignment operator for copying the contents. Intended 
   * primarily for rehash operations triggered in a wrapping class.
   *
   * NB: The array members are marked null in the source. 
   */

  MyType& operator=(MyType& src) {
#ifndef  NDEBUG
  //std::cerr << "Assigning a core. Key table null: " << (keyTable==0) << "\n";
#endif 
    delete[] keyTable;
    delete[] status;
#ifndef  NDEBUG
    //std::cerr << "Success!"; 
#endif 
    keyTable=src.keyTable;
    status=src.status;
    data=src.data; /* Assignment is overloaded, so that no memory leaks
		    * happen and the data here is safe from destruction 
		    * of the original. */    
    src.keyTable=0;
    src.status=0;
    /* FIXME: Document the assignment polizy somewhere*/
    controller=src.controller;
    //std::cerr << "*";
    assert(isLegal());
    //std::cerr << "*";
    return *this;
  }

  /**
   * Serves as both "contains" and "getPlace".
   *
   * @param  key        The key.
   * @param  location   Where the key was found in the table, returned
   *                    by reference. If not, the next place for virgins.
   *
   * @return Whether the key was found.
   */

  bool findFirst(const KeyType key, IndexType & location) const {
    location=controller.getInitPlace(key);
    return findFrom(key, location);
  }
  
  bool findFrom(const KeyType key, IndexType & location) const {
    while (status[location] != STATUS_VIRGIN) {
      /* No runtime assertion of validity here: in order to keep
       * the implementation compact, infinitely recursive usage 
       * of "find" would be needed. */
      if (status[location] == STATUS_INUSE) {
	assert(keyFoundAt(location));
	if (keyTable[location] == key) return true; 
      }
      location=controller.getNextPlace(location);
    }    
    /* Not found in the probe sequence */
    return false;
  }
  
  /**
   * Puts a key into the table. If duplicates are not allowed,
   * the old key gets overwritten. 
   *
   * In less brief words, if ALLOW_DUPLICATES is false and a
   * key for which the == -operator returns true is present in the
   * table, the latter one gets overwritten. 
   *
   * @param  key        The key.
   * @param  location   Where it was put, returned by reference
   *                   
   * @return Whether the key was already present
   */ 
 
  bool putKey(const KeyType key, IndexType & location) {
    IndexType newSize;
    if (controller.aboutToPut(newSize)) { 
      rehash(newSize);
    }
    /* Full table would induce an eternal loop: */
    location=controller.getInitPlace(key);
    while (status[location] == STATUS_INUSE) {
      /* Runtime check for _previous_ validity. */      
      assert(keyFoundAt(location));
    
      if (!ALLOW_DUPLICATES) {
	if (keyTable[location] == key) {
	  /* The operator might be overloaded: */
	  keyTable[location] = key;
	  return true; // Counters OK.
	}
      } 
      location=controller.getNextPlace(location);
    }    
    /* Counter maintenance */
    controller.wroteOn(status[location]);   
    status[location]=STATUS_INUSE;
    keyTable[location]=key;
    /* Did we do it? */
    assert(keyFoundAt(location));
    return false; // Was not previously present.
  }
  
 /**
  * Removes a key from the given place in the table, returning whether
  * there was one. In addition, tries to mark elements to STATUS_VIRGIN
  * where possible.
  */

  bool removeFrom(const IndexType place) {
    assert(place >= 0);
    assert(place < controller.getTableSize());
    if (status[place] == STATUS_INUSE) { // Found!
      /* Let's check whether it was in the right place */
      assert(keyFoundAt(place));
      status[place]=STATUS_REMOVED;
      assert(!keyFoundAt(place));
      IndexType newSize;      
      IndexType localRehashLoc=place;

      switch (controller.justRemoved(newSize)) {
      case ACTION_LOCAL_REHASH:
      	while(fillSlot(localRehashLoc));
	break;
      case ACTION_REHASH_IN_PLACE: 
	rehashLocally();
	break;
      case ACTION_REHASH_TO_SIZE:
	rehash(newSize);
	break;
      }
      return false;
    } 
    /* not found. */
    return false;    
  }
  
  /**
   * Removes a key from the table, returning whether it was found.
   */

  bool removeKey(const KeyType key) {
    IndexType placeForRemoval;
    if (findFirst(key, placeForRemoval)) { 
      /* Somebody should check that find() works properly: */
      assert(keyFoundAt(placeForRemoval));
      removeFrom(placeForRemoval);
      assert(!(!ALLOW_DUPLICATES && containsKey(key)));
      return true;
    } else {
      return false;
    }
  }
  
public:

  bool containsKey(const KeyType key) const {
    IndexType foo;
    return findFirst(key, foo);
  }

  IndexType getTableSize() const {return controller.getTableSize();}
   
  class iterator;
  friend class iterator;

  /* Only const_iterators for now. */

  iterator begin() const {return iterator(this);}
  iterator end() const {return iterator(this, true);}

  void prefetchKey(const KeyType key) const {
#ifdef GNU_PREFETCH
    IndexType initPlace=controller.getInitPlace(key);
    __builtin_prefetch(&keyTable(initPlace), 0, 0);
    __builtin_prefetch(&status(initPlace), 0, 0);
#endif
    data.prefetchAt(initPlace);
  }
    

#ifndef  NDEBUG
  /** 
   * Checks whether the key at location "place" is found 
   * during a normal probe sequence. If a removed key is not erased
   * from the keyTable  --- no reason why it should be --- this
   * function can be used to check whether the removal was
   * successful. However, this does not work after local rehashes,
   * as a new key has probably taken the place.
   *
   * Is not dependent on find-operations of the table:
   * otherwise, infinite assertion recursion might be triggered.
   */ 
  bool keyFoundAt(const IndexType place) const {
    IndexType currIndex;
    KeyType key;

    key=keyTable[place];
    currIndex=controller.getInitPlace(key);
    while (status[currIndex] != STATUS_VIRGIN) {
      if ((status[currIndex] == STATUS_INUSE) &&
	  (keyTable[currIndex] == key)) {
	if (currIndex==place) {
	  return true;	  
	} else {
	  if (!ALLOW_DUPLICATES) {
	    return false;
	  }
	}
      }
       currIndex=controller.getNextPlace(currIndex);
    }

    /* Not found */
    return false;
  }
  
  /**
   * Legality check for the whole structure.
   * 
   */
  bool isLegal() const {
    IndexType statCounts[3]={};

    assert(keyTable != 0);
    assert(status != 0);

    for (IndexType i=0; i < controller.getTableSize(); i++) {
      assert(status[i] < 3);
      statCounts[status[i]]++;
      /* Implication in disguise: if in use, then the key is found here*/
      assert(!(status[i] == STATUS_INUSE) || keyFoundAt(i));
    }

    assert(controller.virginsLegal(statCounts[STATUS_VIRGIN]));
    assert(controller.usedLegal(statCounts[STATUS_INUSE]));
    assert(controller.removedLegal(statCounts[STATUS_REMOVED]));
    return true;
  }
#endif 

  class iterator {
  private:
    iterator() {};
  protected:
    const MyType * target;
    IndexType index;

    /** 
     * Advances the iterator to the next used element. 
     * However, not past the end_iterator 
     */
    void advanceIter() {
      while (index < target->controller.getTableSize()) {
	index++;
	if (target->status[index] == STATUS_INUSE) break;
      }
    }
    
  public:
    /**
     * Begin iterator
     */
    iterator(const MyType * hash) {
      target=hash;
      index=0;
      if (target->status[0] != STATUS_INUSE) advanceIter();
    }

    /**
     * End iterator. Call syntax copied from Eckel.
     */
    iterator(const MyType * hash, bool) { 
      target=hash;
      index=target->getTableSize();
    }

    /**
     * Copy constructor
     */
    iterator(const iterator& src) {
      target=src.target;
      index=src.index;
    }
    
    iterator& operator=(const iterator& src) {
      target=src.target;
      index=src.index;
      return *this;
    }
    
    iterator& operator++() {
      advanceIter();
      return *this;
    }

    /* An alternative to an end iterator for checking whether 
     * all keys have been iterated through: */

    bool isAtEnd() {
      return (index == target->controller.getTableSize());
    }

    /* The keys are not returned by reference. */

    KeyType operator*() {
      assert(index<target->getTableSize()); 
      return target->keyTable[index];
    }
    
    /* The trait trick is in order to make the set with 
     * void values to behave in similar manner */
    
    typename HashValueTraits<ValueType>::const_ref_type getValue() {
      return target->data[index];
    }

    bool operator==(const iterator& cmp) const {
      return (index == cmp.index);
    }

    bool operator!=(const iterator& cmp) const {
      return (index != cmp.index);
    }

    /* NB. Remove does not require resize events: we can
     * use this iterator in real implemetations, also. */

    /* FIXME: do not cause rehash ops. */
    //void remove( ) {
    //assert (target.status(index) == target.STATUS_INUSE);
    //target.removeFrom(index);
    //}

    unsigned int getCurrLoc() const {
      return index;
    }
        
  };

  /**
   * These are operations on KEY SETS, not caring about whether values
   * are involved. 
   *
   * For unary additions etc, overload in deriving classes. We do not
   * know what an unary addition means here, as this could be also a map
   * or something else.
   *
   * Only a generic constructor and an addition operator is needed for
   * the return type. The righthand operand needs an iterator, and in some 
   * cases, a contains -operation.
   */

  /**
   * Set intersection, i.e. logical OR. The right operand only needs
   * to implement an iterator.
   */

  template<typename OutputType, typename RightOperandType>
  OutputType& operator*(RightOperandType rval) {
    OutputType retval=new OutputType();
    typename RightOperandType::iterator i=rval.begin();
    typename RightOperandType::iterator end=rval.end();
    for (;i!=end;i++) {
      if (this.containsKey(*i)) retval.put(*i);
    }
    return *retval;
  }

  /**
   * Logical XOR:    
   */

  /**
   * The right hand operand needs a contains -op, also. 
   */

  template<typename OutputType, typename RightOperandType>
  OutputType& operator^(RightOperandType rval) {
    OutputType retval=new OutputType();
    typename RightOperandType::iterator i=rval.begin();
    typename RightOperandType::iterator end=rval.end();
    for (;i!=end;i++) {
      if (!this.containsKey(*i)) retval.put(*i);
    }
    for (iterator j=this->begin(); !j.isAtEnd(); j++) {
      if (!rval.containsKey(*j)) retval.put(*j);
    }
    return *retval;
  }

  template<typename OutputType, typename RightOperandType>
  OutputType& operator|(RightOperandType rval) {
    OutputType retval=new OutputType();
    typename RightOperandType::iterator i=rval.begin();
    typename RightOperandType::iterator end=rval.end();
    for (;i!=end;i++) {
      retval.put(*i);
    }
    for (iterator j=this->begin(); !j.isAtEnd(); j++) {
      retval.put(*j);
    }
    return *retval;
  }

  /* This one assumes that the right-hand side has a contains-op*/

  template<typename OutputType, typename RightOperandType>
  OutputType& operator-(RightOperandType rval) {
    OutputType retval=new OutputType();
    for (iterator j=this->begin(); !j.isAtEnd(); j++) {
      if (!rval.containsKey(*j)) retval.put(*j);
    }
    return *retval;
  }
  
  
  

};

#endif
