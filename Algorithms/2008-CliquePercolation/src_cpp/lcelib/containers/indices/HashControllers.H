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

#ifndef LCE_HASH_CONTROLLERS
#define LCE_HASH_CONTROLLERS
#include<cassert> 
#include<cmath>



template<unsigned numBytes>
struct FiboHashMult {
  static const unsigned long factor=2654435769ul;
};

template<>
struct FiboHashMult<2> {
  static const unsigned factor=40503u;
};

template<>
struct FiboHashMult<8> {
  static const unsigned long long factor=11400714819323198485ull;
};

/**
 * Linear probing and modulo calculations implemented here. 
 * Not usable as itself, but just as a base class.
 */

class Pow2HashController {
protected:
  /** Why not const? For assignments. */
  size_t logSize;
  Pow2HashController(size_t logTableLen): logSize(logTableLen) {
    //std::cerr << "Inited to native size:" << logSize << "\n";
  }

  /* No checks here necessary. If the user is to access a zero-sized array. 
   * we cannot help. */
  size_t getModuloMask() const {return (1 << logSize)-1;} 
  size_t modloc(size_t key) const {return key & getModuloMask();}
public:
  size_t getNextPlace(const size_t place) const {
    return (place+1) & (getModuloMask());    
  }
  size_t getPrevPlace(const size_t place) const {
    return (place-1) & (getModuloMask());
  }

  size_t getNumSlots() const {
    return sizeForNative(logSize);
  }
  
  static size_t sizeForNative(const size_t nativeSize) {
    return nativeSize==0 ? 0 : 1 << nativeSize;
  }

};

template<size_t primeFactor=29> 
struct Pow2DivHashController: public Pow2HashController {
  Pow2DivHashController(size_t logSize): Pow2HashController(logSize) {}
  size_t getInitPlace(const size_t hashValue) const {
    return modloc(hashValue*primeFactor);
  }
};

/**
 * For Fibonacci hashing, the size of the optimal multiplier depends on
 * the size of the type used for its representation. Hence the peculiar
 * definition below.
 */

template<size_t factor=FiboHashMult<sizeof(size_t)>::factor >
struct Pow2MultHashController: Pow2HashController {
  Pow2MultHashController(size_t logSize): Pow2HashController(logSize) {}
  size_t getInitPlace(const size_t hashValue) const {
    //std::cerr << "Init" << ((hashValue*factor) >> (sizeof(size_t)*8-logSize))
    //	      << "\n";
    return (hashValue*factor) >> (sizeof(size_t)*8-logSize);
  }
}; 

/**
 * The rehash policy proper.
 */

template<unsigned char maxUsedPerc=80,
	 unsigned char minFillPerc=30, 
	 typename logic=Pow2MultHashController<> >
class SmallHashController: public logic {
private:
  size_t numKeys;
  /** 
   * To be certain, we assure that this is not conatructed directy 
   */
  SmallHashController() {}
  
public:
  /* */
  SmallHashController(size_t nativeSize): logic(nativeSize), numKeys(0) {}
  
  bool aboutToPut(size_t & nativeSize) const { 
    nativeSize=logic::logSize;
    /* Should work even if there was no table, i.e. logSize was 0 */

    assert(numKeys <= floor((0.01*maxUsedPerc)*(1 << nativeSize)));

    if (nativeSize==0) { 
      nativeSize=1;
      return true;
    }
    
    if (numKeys == floor((0.01*maxUsedPerc)*(1 << nativeSize))) {
      nativeSize++; // i.e. *=2
      return true;
    }
    return false;
  }
       
    
  /* For decreasing the table size after removals. */
  bool trim(size_t & nativeSize) const {
    nativeSize=logic::logSize;
    bool retval=false;
    if (nativeSize==0) return false;
    /* DEBUGGED_AS_BULLSHIT */
    /* Again, this should work nicely with regard to the fact 
     * that size is not exactly 2^nativeSize (0->0) */
    /* \DEBUGGED_AS_BULLSHIT */
    while (numKeys < ceil((0.01*minFillPerc)*(1 << nativeSize))) {
      nativeSize--; // i.e. /=2
      retval=true;
      if (nativeSize==0) break;
    } 
    return retval;;
  }
  
  size_t getNumKeys() const {return numKeys;}
  
  void removed() {numKeys--;}    
  void added() {numKeys++;}

  bool isLegal(const size_t keyCount) const {
    return (keyCount==numKeys);
  }

  /** 
   * The native size, used for construction in internal operations, 
   * is logarithmic. 
   */
  
  static size_t nativeSizeForCapacity(const size_t capacity, 
				      const unsigned fillPerc=maxUsedPerc) {
    size_t logSize=0;
    while (floor((1<<logSize)*(0.01*fillPerc)) < capacity) logSize++;
    return logSize;
  } 
  
  static size_t sizeForCapacity(const size_t capacity, 
				const unsigned fillPerc=maxUsedPerc) {
    if (capacity==0) return 0; /**/
    return 1 << nativeSizeForCapacity(capacity, fillPerc);
  }
};

#endif
