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

#ifndef LCE_LINEAR_HASH
#define LCE_LINEAR_HASH
#include"../../Randgens.H"
#include"./TableWithStatus.H"
#include<cassert> 
#include<limits>
#ifndef NODEBUG
#include<iostream>
#endif




/**
 * A linear-probing hash table index.
 *
 * The elements put in the table are stored in the slots of the table
 * itself. No pointer traversal is therefore necessary for accessing
 * data from the table. Collisions are resolved by linear probing.
 * 
 * RATIONALE: The access (cache miss) latencies of the memory systems of
 * modern computers are very large in comparison to their sequential 
 * throughput 
 * and the speed
 * of the processors themselves. To maximize performance, "random" accesses 
 * induced by eg. pointer traversals should therefore be avoided. 
 * In the scheme adopted here, only single non-sequential memory access
 * is needed for standard access, addition and removal operations.
 * If the initial place for an element is already used when adding it to
 * the table, the following slots to be probed are accessed sequentially.
 * If the table is not too populated, it is highly probable that the cache
 * line already fetched contains all the slots needed for the following probe 
 * sequence. The same logic applies to access and removal operations, also.
 * Even if the initial memory load is not soufficient for the whole operation, 
 * the following loads proceed in a sequential manner. Iteration
 * through the elements of a table is guaranteed to be extremely fast. 
 * For linear hashes, there are several alternatives to linear probing
 * (eg. double 
 * hashing, nonlinear probing), but because of cache issues, this simple
 * scheme is most efficient [?].
 *
 * The probe sequences are constructed in a greedy manner, elements 
 * arriving first taking the first slots. Growing of the hash
 * mostly maintains this order. The choice of the policy does not
 * affect the average probe length [?]. However, inserting the elements
 * to be searched for the most first may improve performance. This can lead 
 * to a kind of auto-tuning, if the insertions are drawn from the same 
 * distribution as the searches. 
 *
 * The size of the hash is only updated after insertions. The main
 * reason for this behaviour is to allow the iterators to use an unmodified
 * remove-function. If resize could be triggered by removal, the structure
 * pointed to by an iterator could simply cease to exist during iteration.
 * In addition, it is probably wise to decrease the size of the table only
 * when removals are followed by further insertions, indicating that
 * the contents of the hash are to be further manipulated in the following.
 * 
 * This version of the core performs local rehash (filling slots just
 * removed from) after each removal operation. While it is possible to 
 * implement a "removed" mark as an alternative, is would seldom be of any
 * benefit, and the code becomes much cleaner this way. 
 *
 * @sa HashControllers
 *
 */

/** 
 * The default params for the hash table
 */ 

/******************** The hash table ********************/

/**
 * The core class for the hash tables. Is derives from the array
 * data storage type and includes the hash controller.
 * Intended to be used as a base class.
 *
 * \invariant The hash invariant. If an element exists in the table,
 * there are no slots marked as virgin between it's calculated 
 * initial place and the real place in a increasing, cyclic slot 
 * sequence. 
 *
 * \invariant The counter invariants. The controller knows the length
 * of the table and probably the number of elements, also.
 *
 * \invariant The table should never become full. This is to be
 *            guaranteed by the controller.
 * 
 * See the subclass candidates for the description of parameters. 
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
class LinearHash: 
  public TableWithStatus<KeyType, ValueType, Policy, Params, Table, 
			 typename Params::StatusPolicy, 
			 LinearHash<KeyType, ValueType, Policy, Params,
				    Table> > {	
private:
  typedef LinearHash<KeyType, ValueType, Policy, Params, Table> MyType;
  typedef TableWithStatus<KeyType, ValueType, Policy, Params, Table, 
			  typename Params::StatusPolicy, MyType> super;
  typedef typename Params::HashController HashController;

public:
  typedef KeyType IndexKeyType;
private:
  HashController controller;

  /**
   * Rehashes the structure into a new one of specified size, then copies
   * it into this. Assumes that the \c = -operator takes care of the 
   * deleting of the original table etc.
   * 
   * \param[in] size The size of the new table _as_the_controller_native_
   */
  
  void rehash(const size_t nativeSize) {    
    //std::cerr << "Rehashing. Table:";
    //printTable();
    assert(isLegal());
    super::disassemble();
    /* We use the constructor that takes the controller's 
     * native size for the table (might be true size or logarithmic)  
     * instead of the capacity in: */
    MyType newHash(nativeSize, true);
    /* We maintain the ordering of elems by iterating in an 
     * iterator-fashion, that is, starting from the first used element
     * after an unused one */
    /* Thanks to assertions, we found out that this does not work with 
     * an empty hash.*/
    if (size()) /* !=0 */ {
      size_t initLoc;
      for (initLoc=0; super::isUsed(initLoc); ++initLoc);
      /* the slot at initLoc is not used, all up to it are */
      for (size_t i=initLoc+1; i<getTableSize(); ++i) {
	if (super::isUsed(i)) {
	  /* We perform a _direct_ copy that memcopies everything as is. */
	  rehash_put(i, newHash);
	}
      }
      /* And then the beginning, all are used */
      for (size_t i=0; i<initLoc; ++i) {
	assert(super::isUsed(i));
	rehash_put(i, newHash);
      }
    }
    /* This should maintain everything defined by the superclasses */
    /* Controller can be copied directly, we _know_ that 
     * it behaves nice. */
    controller=newHash.controller;
    newHash.shallowMoveTo(*this);
    /* Updates done. */ 
    super::assemble();
    assert(isLegal());
    //std::cerr << "Done.Table:\n";
    //printTable();
  }

  /** 
   * The put operation used for rehashes. Everything is copied, including 
   * the usage status. Rehashes are not queried, but number of elems
   * are updated OK.
   */
  void rehash_put(const size_t i, MyType & dest) {
    size_t initLoc=
      dest.controller.getInitPlace(Policy::getHashValue(super::constRefToKey(i)));
    size_t loc=initLoc;
    while (dest.isUsed(loc)) {
      if (Params::HASH_ORDERED && dest.initPlaceBefore(initLoc, loc)) {
	dest.pushProbeAt(loc);
	break;
      }
      loc=dest.controller.getNextPlace(loc);
    }
    /* The superclass moves the statuses etc. */
    super::copyElemTo(dest, loc, i);
    dest.controller.added();
    assert(dest.keyFoundAt(loc));
  }

//   /** The bool is just a marker for the operation being the internal one */

//   LinearHash(size_t nativeSize, bool): 
//     super(Controller::sizeForNative(nativeSize)), 
//     controller(nativeSize) {
//       assert(isLegal());
//     }
  
  /** 
   * Convenience. You see. 

   *
   * RATIONALE: The policies for arbitrary elements can directly
   * inherit the policies of their KeyTypes.
   */
  
  size_t initPlaceAt(const size_t loc) const {
    return controller.getInitPlace(Policy::getHashValue(super::constRefToKey(loc)));
  }

  /**
   * Checks the _cyclic_ ordering of initial places of elements in the table.
   *
   * @param initProbe Where did the probe sequence we are considering 
   *                  start: i.e. the initial place for the key we are 
   *                  putting, seeking, considering etc.
   * @param currLoc   The current location in we are considering. Should
   *                  be used: the initial place for the key at currLoc
   *                  is considered.
   * 
   * @return          Whether the initial location of the key at the 
   *                  currLoc is cyclically following to the initProbe.
   */

  size_t initPlaceBefore(const size_t initProbe, const size_t currLoc) const {
    assert(super::isUsed(currLoc));
    size_t initAt=initPlaceAt(currLoc);
    bool currRotated=(initAt > currLoc);
    bool probeRotated=(initProbe > currLoc);
    return ((probeRotated==currRotated) &&// normal ordering
	    (initProbe < initAt)) || (!currRotated && probeRotated);
  }
	
  /**
   * Moves the sequnce beginning at the given location cyclically forward
   * so that the slot becomes free for use.
   */
	   
  void pushProbeAt(const size_t loc) {
    assert(super::isUsed(loc));
    //std::cerr << "Pushing at:" << loc << ". Table:";
    //printTable();
    /* A recursion would be more elegant, I afraid. */
    size_t i=loc;
    for (; super::isUsed(i); i=controller.getNextPlace(i));
    size_t prev=controller.getPrevPlace(i);
    for (; i!=loc; prev=controller.getPrevPlace(prev)) {
      super::moveOrSwap(i, prev);
      i=prev;
    }
    //std::cerr << "Done. Table:";
    //printTable();
  }
		       
  /** The constructor for rehashing  */
  LinearHash(const size_t nativeSize, bool): 
    super(HashController::sizeForNative(nativeSize)), controller(nativeSize) {}
    
protected:
  
  /**
   * The removal operation proper. Tries to overwrite the contents
   * of a slot by a following element in the same, uninterrupted
   * sequence of used slots. If unsuccesful, marks the initial slot
   * as virgin, otherwise repeats the sequence for the slot where the
   * filling element was from.
   * 
   * The hash invariant is maintained and locally enforced 
   * by this method. 
   *
   * The author has previously implemented a quite different version
   * of this method for Syslore Oy. No IP problems here.
   * 
   * @param[in] toBeFilled The location of the "removed" slot to be filled  
   *                      
   */
  
  void removeFrom(size_t toBeFilled) {
    
    size_t currSlot;
    size_t initPlace;
    
    assert(super::isUsed(toBeFilled));
    
    currSlot=controller.getNextPlace(toBeFilled); 
    
    /* remove runs destructor.  */
    super::remove_stage_1(toBeFilled);
    controller.removed();
   
    /* Let us find an element to fill the slot with: */
    while (super::isUsed(currSlot)) {
      initPlace=initPlaceAt(currSlot);

      /* Now, we must figure out whether we can use the key 
       * at "currSlot" to fill the empty place. For linear probing
       * that is assumed here it suffices to check whether the 
       * initial place for the key is at or before the empty slot. 
       * Simple comparison of values is not enough, however, 
       * because either of the two probes concerned (this one
       * or the one corresponding to the put operation 
       * of the element in question) 
       * can have rotated back to the beginning of the table. */
      
      bool currRotated=(initPlace > currSlot); 
      bool probeRotated=(toBeFilled > currSlot);
      
      if (((probeRotated == currRotated) &&  // The "normal" case: 
	   (initPlace <= toBeFilled)) ||     // move by comparison. 
	  (currRotated && !probeRotated)) {  // OR just current element did, 
	                                     // no need to even compare.      
	/* We may move the element to the empty place. Usage status
	 * should follow. */
	
	/* The superclass can decide wheter */
	super::moveOrSwap(toBeFilled, currSlot);
	assert(super::isUsed(toBeFilled));
	assert(keyFoundAt(toBeFilled));
        toBeFilled=currSlot;
      } /* Else, continue the search. */
      currSlot=controller.getNextPlace(currSlot);
    } 
    /* Observe the order: We do not know what the destructor does, 
     * so we perform the setting only after everthin in place.*/
    super::remove_stage_2(toBeFilled);
    super::setAsEmpty(toBeFilled);
    assert(!super::isUsed(toBeFilled));
  }
   
  /**
   * The default constructor for an empty table of a given size.
   */

  LinearHash(size_t capacity=0): 
    super(HashController::sizeForCapacity(capacity)),
    controller(HashController::nativeSizeForCapacity(capacity)) {
      assert(isLegal());
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

  bool findFirst(const KeyType & key, size_t & location) const {
    /* The table might be uninitialized (size=0). We can, however,
     * check for an empty table with the same work, also: */
    if (super::base_empty()) return false;
    location=controller.getInitPlace(Policy::getHashValue(key));
    //std::cerr << "InitPlace:" << location << "\n"; 
    return findFrom(key, location);
  }
  
  bool findFrom(const KeyType & key, size_t & location) const {
    size_t initLoc=location;
    for (; super::isUsed(location); location=controller.getNextPlace(location)) {
      assert(keyFoundAt(location));
      if (key == super::constRefToKey(location)) {
	return true; 
      }
      /* The compiler should eliminate the known condition. 
       * In the case of an ordered hash, it is impossible to find 
       * the desired key after a one for which the initial location is
       * higher. */
      //std::cerr << "*";
      if (Params::HASH_ORDERED && initPlaceBefore(initLoc, location)) break;
    }
    /* Not found in the probe sequence */
    return false;
  } 

  template<typename AuxType> 
  AuxType * auxData() {
    assert(sizeof(AuxType) < sizeof(Params::HashController));
    return (AuxType *) &controller;
  }
    

protected:

  /**
   * Puts the key into next suitable place, returning how many
   * equal ones were found on the way. 
   *
   * Useful for put operations of multisets and multimaps.  
   *
   * @param key      The key
   * @param location The location it was put into 
   *
   * @return The number of keys comparing as equal 
   *         previously present. As boolean, it translates to 
   *         whether any were. 
   */

  size_t placeToPut(const KeyType & key, size_t & location) {
    size_t oldSize;
    size_t numFound=0;
    if (controller.aboutToPut(oldSize)) rehash(oldSize);   
    size_t initLoc=controller.getInitPlace(Policy::getHashValue(key));    
    for (location=initLoc; super::isUsed(location); 
	 location=controller.getNextPlace(location)) {
      /* Runtime check for _previous_ validity. */      
      assert(keyFoundAt(location));    
      if (key == super::constRefToKey(location)) {
	++numFound;
      } else {
	/* In the case of an ordered hash tabel, we might want to put the key 
	 * into middle of a probe sequence*/
	if (Params::HASH_ORDERED && initPlaceBefore(initLoc, location)) {
	  //std::cerr << "Foo!";
	  pushProbeAt(location);
	  break;
	}
      }
    }
    super::refToKey(location)=key;
    super::setAsUsed(location);
    controller.added();
    return numFound;
  }

  /** 
   * Looks for a key, and puts a bloody new one in if not found otherwise.
   * Useful for put operations of non-multi sets and maps. 
   *
   * @param key      The key
   * @param location The location it was put into of found from
   *
   * @return Whether the key was already present 
   */

  bool forcedFind(const KeyType & key, size_t & location) {
    size_t newSize;
    if (findFirst(key, location)) {
      return true;
    } else {      
      if (controller.aboutToPut(newSize)) {
	rehash(newSize);
	/* We do not do anything with the return value, so the compiler 
	 * is free to optimize the calculation away. Even if not, this is
	 * rare. */
	placeToPut(key, location);
	assert(keyFoundAt(location));
      } else {/* Location already OK. */
	if (Params::HASH_ORDERED && super::isUsed(location)) {
	  /* need to push */
	  //std::cerr << "Foo!";
	  pushProbeAt(location);
	} else {assert(!super::isUsed(location));}
	super::refToKey(location)=key;
	super::setAsUsed(location);    
	controller.added();
	//std::cerr << "(3)";
	assert(keyFoundAt(location));
	//std::cerr << "(4)";
      }	
      return false; /* A new one. */
    }
  }
    
  
public:

  ~LinearHash() {
    if (!super::base_empty()) {
      //std::cerr << "Destroying a non-empty-linear hash:..";
      for (size_t i=0; i<getTableSize(); ++i) {
	if (super::isUsed(i)) super::final_remove(i);
      }
      //std::cerr << "Done. To Super?\n";
    }
  }

  void clear() {
    if (!super::base_empty()) {
      for (size_t i=0; i<getTableSize(); ++i) {
	if (super::isUsed(i)) super::final_remove(i);
      }
    }
    rehash(0);
    super::assemble();
  }
    
  template<typename RandSource>
  KeyType weighedRandKey(RandSource & src=globalRandSource) const {
    return super::constRefToKey(weighedRandSlot(src));
  }
		 
  template<typename RandSource> 
  size_t randSlot(RandSource & src=globalRandSource) const {
    size_t slot;

    do {
      slot=src.next(getTableSize());
    } while (!super::isUsed(slot));
    return slot;
  }
	         
  template<typename RandSource>
  KeyType randKey(RandSource & src=globalRandSource) const {
    return super::constRefToKey(randSlot(src));
  }

  bool contains(const KeyType & key) const {
    size_t foo;
    return findFirst(key, foo);
  }
  
  size_t getTableSize() const {return controller.getNumSlots();}
  
  size_t size() const {return controller.getNumKeys();}

  /**
   * Remove by key is free to rehash if needed. 
   */

  bool remove(const KeyType & key) {
    size_t loc;
    if (findFirst(key, loc)) {
      /* This notifies the controller: */
      removeFrom(loc);
      trim();

      return true;
    } else {
      return false;
    }
  }
  
  void trim() {
    size_t newSize;
    if (controller.trim(newSize)) { 
      rehash(newSize);
    } 
  }

  void prefetch(const KeyType & key) const {
    super::prefetch(controller.getInitPlace(Policy::getHashValue(key)));
  }
  
  bool preload(const KeyType & key) const {
    return isUsed(controller.getInitPlace(Policy::getHashValue(key)));
  }

  class iterator;
  friend class iterator;

  class const_iterator;
  friend class const_iterator;

  iterator begin() {return iterator(this);}
  
  const_iterator begin() const {return const_iterator(this);}

  /** 
   * A local legality check. 
   * Checks whether the key at location "place" is found 
   * during a normal probe sequence. 
   *
   * Is not dependent on find-operations of the table:
   * otherwise, infinite assertion recursion might be triggered.
   *
   * \param place The slot to be checked.
   * \return      Whether the element belongs to the slot, that is,
   *              whether the hash invariant is true for this slot. 
   */ 

  bool keyFoundAt(const size_t place) const {
    size_t i=initPlaceAt(place);
    size_t initToKey=i;
    //std::cerr << ":";
    while (true) {
      if (!super::isUsed(i)) {
	std::cerr << "Key at " << place << " not to be found. Violation at" 
		  << i << ". Table:";
	printTable();
	return false; 
      }
      if (i==place) return true;
      /* Ordering: if an element is before the initial the place in the 
       * probe seq. of the key at place, it should have cyclically previous
       * initial place. */
      if (Params::HASH_ORDERED) {
	//std::cerr << "H";
	if (initPlaceBefore(initToKey, i) && 
	    initToKey != initPlaceAt(i)) {
	  std::cerr << "Key at " << place << " not to be found. Violation at" 
		    << i << ". Table:";
	  printTable();
	  return false;
	}
	//std::cerr << "h";
	//std::cerr << i;
      }
      i=controller.getNextPlace(i);
    }    
  }
  
  /**
   * Legality check for the whole structure. Checks the hash 
   * invariant for each element in the table, and the counter invariants.
   * 
   * @return Whether the invariants are duly obeyed.
   */

  bool isLegal() const {
    if (!super::isLegal()) return false;
    size_t usedCount=0;
    for (size_t i=0; i < getTableSize(); ++i) {
      if (super::isUsed(i)) { 
	usedCount++;
	if (!keyFoundAt(i)) return false;
      }
      if (!super::localLegal(i)) return false;
    }
    return controller.isLegal(usedCount);
  }

  bool keyLegal(const KeyType & key) const {
    size_t loc;
    if (findFirst(key, loc)) return super::localLegal(loc);
    else return true;
  }

  void printTable() const {
    std::cerr << "\n";
    for (size_t i=0; i < getTableSize(); ++i) {
      std::cerr << i;
      if (super::isUsed(i)) std::cerr << " " << super::constRefToKey(i) << " " 
			       << initPlaceAt(i);
      std::cerr << "\n";
    }
  }

  /**
   * Const-iterator. Nuff said.
   */

  class const_iterator {
    /** To allow construction */
    /* standard suxx: */
    // friend class MyType;
    friend class LinearHash<KeyType, ValueType, Policy, Params, Table>; 
  private:
    const_iterator() {};
  protected:
    const MyType * target;
    size_t loc;

    /** 
     * Advances the iterator to the next used element, that is, the
     * previous one. If no table exists, does not refer to it.
     */

    void advanceIter() {
      do {
	loc++;
      } while ((loc < (target->getTableSize())) && (!(target->isUsed(loc))));
    }
    /**
     * Begin iterator
     */

    const_iterator(const MyType * hash): target(hash), loc(0) {
      if ((target->getTableSize() > 0) && !(target->isUsed(loc))) {
	advanceIter();
      }
    }

    /**
     * Standard increment operator as expected for iterators. 
     */
    
  public:

    ~const_iterator() {/*std::cerr << "Const iter dying.\n";*/}

    const_iterator& operator++() {
      advanceIter();
      return *this;
    }

    /**
     * An alternative to an end iterator for checking whether 
     * all keys have been iterated through: 
     */
    
    bool finished() const {
      return (loc == target->getTableSize());
    }

    /**
     * The dereference operator. Would return a const reference 
     * in any case, as the manipulation of the keys while in the
     * table would surely break something.
     */

    const KeyType & operator*() const {
      assert(loc < target->getTableSize()); 
      return target->constRefToKey(loc);
    }
    
    /**
     * Comparison operator made extremely simple.
     * 
     * RATIONALE: Loop end condition should be as simple as possible.
     */

    bool operator==(const const_iterator & cmp) const {
      return (loc == cmp.loc);
    }

    /**
     * Not-equal operator in the simplest possible form.
     */

    bool operator!=(const const_iterator & cmp) const {
      return (loc != cmp.loc);
    }

    size_t getLoc() const {
      return loc;
    }
    
  };

  /**
   * The non-const iterator allows removal of keys, also. The main
   * problem is that local rehash operations can bring elements from
   * the beginning of table to the rear, therefore causing them to 
   * be iterated over again. We solve this by starting the iterator from the 
   * first virgin element. 
   *
   * If the user wants to generate new end-iterators during the iteration,
   * the starting element is not guaranteed to stay constant (the first virgin
   * element might move). Therefore, we use a magic value (table size)
   * to denote an iterator that has come to an end. 
   *
   * RATIONALE:
   * This really seems to be the only decent way to guarantee that 
   * each element is iterated over exactly once. 
   */
  
  class iterator {
    /** To allow construction */
    /* standard suxx: */
    // friend class MyType;
    friend class LinearHash<KeyType, ValueType, Policy, Params, Table>; 
  private:
    iterator() {};
  protected:
    /** Pointer to the hash table */
    MyType * target;
    /** Current position of the iterator */
    size_t loc;
    /** The place we started the iteration from. */
    size_t endLoc; 

    /** 
     * Advances the iterator to the next used element. 
     */

    void advanceIter() {
      while (loc != endLoc) {
	/* We do not simply ++ because we are ment to go around. */
	loc=target->controller.getNextPlace(loc);
	if (target->isUsed(loc)) return;
      }
      /* Finished. */
      loc=target->getTableSize();
    }
    
    /**
     * Begin iterator
     */
    iterator(MyType * hash):    
      target(hash), loc(0) {
      /* We must check whether the table exists. In fact, we can spare
       * some time by checking for an empty table simultaneously: */
      if (target->size() == 0) { 
	/* Mark as finished: */
	loc=target->getTableSize();
      } else {
	/* Let's find the first virgin... */
	while (target->isUsed(loc)) {loc++;}
	/* ...which serves as the end point of iteration:*/
	endLoc=loc;
	/* and for the start point, the first used one. We must use 
	 * the getNextPlace here, because the rest of the table might 
	 * be empty. */
	do { 
	  loc=target->controller.getNextPlace(loc);
	} while (!target->isUsed(loc));
      }
    }

  public:
    
    ~iterator() {
      //std::cerr << "Trimming the target:";
      target->trim();
      //std::cerr << "Trim done.\n";
    }
    
  /* No need to override the special member functions */
  
  /**
   * Moves the iterator to point to the next element
   */
  
    iterator& operator++() {
      advanceIter();
      return *this;
    }

    /**
     * An alternative to an end iterator for checking whether 
     * all keys have been iterated through. Not compatible with
     * the STL, but sometimes more convenient. 
     * 
     * @return Whether the iteration has come to an end.
     */

    bool finished() {return loc==target->getTableSize();}
    
    /**
     * The dereference operator returns a reference to the _key_.
     * Data is to be handled irrespective of this level of 
     * implementation. 
     *
     * @return A const reference to the data pointed to by the iterator.
     */
 
    const KeyType & operator*() {
      assert(loc < target->getTableSize()); 
      return target->constRefToKey(loc);
    }

    
    /**
     * Comparison operator made extremely simple.
     * 
     * RATIONALE: Loop end condition should be as simple as possible.
     */
    
    bool operator==(const iterator & cmp) const {
      return (loc == cmp.loc);
    }

    /**
     * A wrapper implementing the not-equal -operator.
     */

    bool operator!=(const const_iterator & cmp) const {
      return (!operator==(cmp));
    }
    
    /**
     * Removal of the element pointed to by the iterator. This is
     * the real \e raison \e d'être of the non-const iterator as
     * a separate class. The iterator is moved to point at the next
     * element automatically, so that it remains in a valid state 
     * after the operation. This behaviour is required by the subclasses.
     *
     * @return A reference to this iterator after the removal
     */
  
    iterator & remove() {
      assert(target->isUsed(loc));
      target->removeFrom(loc);
      if (!(target->isUsed(loc))) {
	advanceIter();
      }
      /* Otherwise, a new element has been brought here by a 
       * fillSlot-operation */
      return *this;
    }    

    /**
     * The current table slot of the iterator.
     *
     * @return The index of the slot the iterator points to. One past
     *         the end of the table, if finished. 
     */

    size_t getLoc() const {
      return loc;
    }
    
  };
};


#endif
