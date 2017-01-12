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

#ifndef LCE_ARRAY_BASE
#define LCE_ARRAY_BASE
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <memory>
//#include <../PrefetchPolicy.H>

/** 
 * The ultimate base class for every linear representation. 
 * Just contains an array of the DataType. No length information
 * is contained in this level. 
 *
 * The main design decision on this level is to use malloc
 * instead of the operators new and delete. We do not want the
 * peculiarities of constructors, destructors and assignment operators 
 * to mess up with our structures, as we want to be free to move things
 * around. We only check whether an allocation has succeeded by assertions.
 *
 * The internal operations on the data just move memory. External ones,
 * however, 
 */

template <typename DataType>
class ArrayBase {
  typedef ArrayBase<DataType> MyType;
private:
  DataType * data;

public:
  
  typedef DataType & reference;
  typedef const DataType & const_reference;

  /** 
   * The one-parameter version of the resize function just reallocs. 
   */
  void resize(const size_t newSize) {
    /* We daringly allow for both null pointers and zero
     * sizes here. */
    data=(DataType *) realloc(data, sizeof(DataType) * newSize);
    assert(data != 0); /* Crash boom bang. Should fail here, as the old data 
			* just leaked. */
  }

  /**
   * Three-parameter version initializes the possible new slots to a given
   * value.
   */

  void resize(const size_t newSize, const size_t oldSize, 
	 const DataType & initVal) {
    resize(newSize);
    if (newSize>oldSize) {
      for (size_t i=oldSize; i<newSize; ++i) {
	memcpy(&data[i], &initVal, sizeof(DataType));
      }
    }
  }
      
  /**
   * Two-parameter version initializes to empty value.  
   * (NB! Old version didn't init, and the excuse was `For
   * compatibility'. If this breaks something, you know who to blame,
   * but at the moment the resize functions are not used at all, so I
   * doubt it.)
   */

  void resize(const size_t newSize, const size_t oldSize) {
    resize(newSize);
    if (newSize>oldSize) {
      DataType initVal;
      for (size_t i=oldSize; i<newSize; ++i) {
	memcpy(&data[i], &initVal, sizeof(DataType));
      }
    }

  }

  /**
   * Adds an empty element to a given slot, moving the old one and all 
   * following ones forward. Realloc happens, and the old slot retains the
   * old value. We _do not_ clear. 
   */

  void pushAt(const size_t loc, const size_t oldSize) {
    this->resize(oldSize+1);
    memmove(&(data[loc+1]), &(data[loc]), sizeof(DataType)*(oldSize-loc)); 
  }

  void pullFrom(const size_t loc, const size_t oldSize) {
    memmove(&data[loc], &data[loc+1], sizeof(DataType)*(oldSize-loc-1));
  }

  reference refTo(const size_t i) {return data[i];}
  
  const_reference constRefTo(const size_t i) const {return data[i];}

  static size_t elemSize() {return sizeof(DataType);}
  
protected:

  /* Cuts the reference in order to avoid freeing mem copied elsewhere */

  void shallowMoveTo(MyType & dest) {
    //std::cerr << "Move from" << data << " to:";
    //if (dest.data) {std::cerr << dest.data << ":";}
    //else { std::cerr << "empty:";}
    free(dest.data);
    //std::cerr << "done\n";
    dest.data=data;
    data=0;
  }

  /** Moves data in during the buildup phase */
  
  void initSet(const DataType & src, size_t loc) {
    memmove(&data[loc], &src, sizeof(DataType));
  }
  
  void assemble() {}
  
  void copyElemTo(MyType & dest, const size_t loc, const size_t i) {
    memcpy(&(dest.refTo(loc)), &(this->refTo(i)), sizeof(DataType));
  }

  void copy(const size_t to, const size_t from) {
    memcpy(&(this->refTo(to)), &(this->refTo(from)), sizeof(DataType));
  }

public:
  
  void swap(const size_t to, const size_t from) {
    char temp[sizeof(DataType)];
    memcpy(temp, &data[to]);
    memcpy(&data[to], &data[from]);
    memcpy(&data[from], temp);
  }    
  

public:
  
  /** 
   * The standard constructor for a table of given size. Initializes 
   * elements to default values. The template is intended for avoiding 
   * unintentional overloading: in the boolean parameter was an ordinary
   * one, ambiguity wrt. the next constructor could result.
   */

  ArrayBase(const size_t size=0) {
    //std::cerr << "(Base constr:" << this << ")";
    if (size>0) {
      data=(DataType *) malloc(sizeof(DataType)*size);
      if (data==0) {
	//std::cerr << "Failed in the allocation, table:" << size << ", elem:" 
	//  << sizeof(DataType) << "\n";
      } else {
	DataType initVal;
	for (size_t i=0; i<size; ++i) {
	  memcpy(&data[i], &initVal, sizeof(DataType));
	}
      }
      assert(data != 0);
    } else {
      //std::cerr << "Init empty\n";
      data=0;
    }
  }

  ArrayBase(const size_t size, const DataType & initVal) {
    if (size>0) {
      data=(DataType *) malloc(sizeof(DataType)*size);
      assert(data != 0);
      std::uninitialized_fill_n(data, size, initVal);
    } else {
      //std::cerr << "Init empty\n";
      data=0;
    }
  }

  /** The standard destructor. Deleting 0:s (nulls) is of no harm.  */

  ~ArrayBase() {
    if (data) { 
      //std::cerr << "Destr at:" << data;
      free(data);
      //std::cerr << "Done.\n";
    } 
  }  

  /* We use the subscript operators because we both want these to be
   * publicly visible in order to allow the usage as a status array 
   * without complex friendships, but also control the access from 
   * subclasses: these will be overridden in them. */

  /**
   * Direct array access.
   *
   * \param[in] i The place in the array
   *
   * \return A reference to the element at \c i. 
   */

  void prefetch(const size_t loc=0) const {
#ifdef GNU_PREFETCH
    __builtin_prefetch(&(data[loc]),0,0); /* For read, low temporal locality */
#endif
  }

  bool base_empty() const {return data==0;}

  /** If no usage status is defined, this is what we want */
  bool isUsed(const size_t loc) const {return true;}

};

// template<typename DataType, unsigned depth>
// struct PrefetchPolicy<ArrayBase<DataType>, depth> {
//   static void prefetch(const ArrayBase<DataType> & tgt, const size_t loc=0) {
//     PrefetchPolicy<DataType, depth-1>::prefetch(tgt[loc]);
//   }
//   static void safe_prefetch(const ArrayBase<DataType> & tgt, 
// 			    const size_t loc=0) {}
// };

// template<typename DataType>
// struct PrefetchPolicy<ArrayBase<DataType>, 0> {
//   static void prefetch(const ArrayBase<DataType> & tgt, 
// 		       const size_t loc=0) {
//     tgt.prefetch(loc);
//   }
//   static void safe_prefetch(const ArrayBase<DataType> & tgt, 
// 			    const size_t loc=0) {
//     tgt.prefetch(loc);
//   }
// };

/**
 * A piece of template metaprogramming magic to calculate the 2-base 
 * logarithm of a number (rounded down) at compile-time.
 */

template<unsigned NumBits>
struct Log2BitsDown {
  enum {value=1 + Log2BitsDown<NumBits / 2>::value};
};

/**
 * A specialization to end the compile-time recursion. Log2(1)=0. 
 */

template<>
struct Log2BitsDown<1> {
  enum {value=0};
};

template<typename ReprType>
struct BitHelper {
  /** 
   * The amount of shift needed to convert the index of a bool to the 
   * index in array. 0 for types with 1 bits, 1 for 2, 2 for 4 etc. */  
  enum {shift=Log2BitsDown<sizeof(ReprType)*8>::value};
  /**
   * The remaining bits, i.e. all that is not shifted above. 0...0000 for 1,
   * 0...0001 for 2, 0...0011 for 4 etc. A mask for the bits representing
   * the location for a given bit in ReprType. */ 
  enum {mask=(1 << (Log2BitsDown<sizeof(ReprType)*8>::value))-1};
};

/**
 * An array of bits stored in an array of the template parameter type. Should
 * be an integral type: for calculations, the size is rounded 
 * down to the nearest power of two in order to allow some binary magic
 * for calculation modulos and division. The logarithms needed are calculated 
 * by template metaprogramming tricks. 
 *
 * We do not make this a template proper, as we can be sure that the
 * size_t is what we want.
 */

template<>
class ArrayBase<bool>: private ArrayBase<size_t> {
  typedef ArrayBase<bool> MyType;
  typedef ArrayBase<size_t> super;
private:
  typedef BitHelper<size_t> vals;

  /**
   * The length of the representation array. Rounding goes up. 
   */

  static size_t reprLen(const size_t len) {
    size_t lenDown=len >> vals::shift;
    if (len-(lenDown << vals::shift)) lenDown++;
    return lenDown;
  }

  static size_t filledSlot(bool val) {
    if (val) return (0-1);
    else return 0;
  }

  size_t & reprSlot(const size_t loc) {
    return super::refTo(loc >> vals::shift);
  }

  const size_t & reprSlot(const size_t loc) const {
    return super::constRefTo(loc >> vals::shift);
  }

  static size_t reprBit(const size_t loc) {
    return 1 << (loc & vals::mask);
  }

public:
  /* Keys inherited OK*/
  
  typedef bool const_reference;

protected:
  /* ? WTF ? */
  void initSet(const bool val, const size_t loc) {
    refTo(loc)=val;
  }

  void assemble() {}

  /** 
   * The one-parameter version of the resize function just reallocs. 
   */

public:

  void resize(const size_t newSize) {
    /* We daringly allow for both null pointers and zero
     * sizes here. */
    super::resize(reprLen(newSize));
  }
  
  /**
   * Three-parameter version initializes the possible new slots to a given
   * value.
   */

  void resize(const size_t newSize, const size_t oldSize, 
	 const bool val) {
    size_t oldRep=reprLen(oldSize);
    size_t newRep=reprLen(newSize);
    if (newRep != oldRep) {
      if (val) super::resize(newRep, oldRep, size_t()-1);
      else super::resize(newRep, oldRep, 0);
    }
    if (newSize > oldSize) {
      /* If the last element of the former representation 
       * was not full, we must also set itäs remaining bits, the 
       * number of which is: */
      size_t remBits=oldRep*sizeof(size_t)-oldSize;
      if (remBits != 0) {
	/* ...and the corresponfding mask: */
	size_t remMask=(size_t()-1) << remBits;	
	if (val) {
	  /* Remaining bits to one, rest unchanged */
	  super::refTo(oldRep-1) |= remMask;
	} else {
	  /* Remaining bits to zero, rest unchanges */
	  super::refTo(oldRep-1) &= (~remMask);
	}
      }
    }
  }

  void resize(const size_t newSize, const size_t oldSize) {
    this->resize(newSize, oldSize, false);
  }
    
  ArrayBase(size_t size=0):super(reprLen(size)) {}
  
  ArrayBase(size_t size, bool initVal): 
    super(reprLen(size),filledSlot(initVal)) {}

  /* The superclass's destructor is OK */
  
  /**
   * The STL refers to these as "references", but we know better:
   */
  
  class stub {
    /** To allow construction... */
    friend class ArrayBase<bool>; /* =MyType;. Standard sux.*/ 
  private:
    /** A direct reference to the data at the slot */
    size_t & slot;
    /** A mask for the bit: ..0001 for 0, ..0010 for 1, 0100 for 2 etc.*/
    size_t bit; 
  
    /* Cannot do even this: would result in uninit'd ref */
    //stub() {}; /* Not to be constructed by just anyone */

    /** 
     * This is how the slot and the bit are computed for a location in
     * the bit array: 
     */
    stub(MyType & tgt, size_t loc):
      slot(tgt.reprSlot(loc)), bit(MyType::reprBit(loc)) {}
  public:
    /**
     * Type conversion to the type pretended to be a reference of.  
     * For other types, the standard conversion operators from bool are
     * used: no need for further conversion operators here.
     */
    operator bool() const {return slot & bit;}
    /**
     * Some nice bit-magic to assign a bool value. 
     * For other types, the standard conversion operators to bool are
     * used: no need for further conversion operators here. 
     */
    bool operator=(const bool val) {
      if (val) {
	slot |= bit;
      } else {
	slot &= ~bit;
      }
      return val;
    }
  };

  friend class stub;
public:
  typedef stub reference;
protected:

  reference refTo(const size_t i) {
    return stub(*this, i);
  }

  reference operator[](const size_t i) {
    return stub(*this, i);
  }

  /** This returns plain bool */

  const_reference constRefTo(const size_t loc) const {
    return reprSlot(loc) & reprBit(loc);
  }

  void prefetch(const size_t i) const {
    super::prefetch(i >> vals::shift);
  }

  /** 
   * Unless overridden by subclasses, this is what we want:
   */

  bool isUsed(const size_t loc) const {return true;}
  
};

#endif
