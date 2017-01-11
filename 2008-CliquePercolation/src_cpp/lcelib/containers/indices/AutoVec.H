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

#ifndef LCE_AUTO_VEC
#define LCE_AUTO_VEC

struct SizeHelper {
  static size_t sizeForCapacity(size_t cap) {
    if (cap==0) return 0;
    size_t retval=1;
    while (cap > retval) retval <<= 1;
    return retval;
  }
}

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
class AutoVec: 
  public Table<KeyType, ValueType, Policy, Params, 
	       AutoVec<KeyType, ValueType, Policy, Params, Table> > {
  typedef AutoVec<KeyType, ValueType, Policy, Params, Table> MyType;
  typedef Table<KeyType, ValueType, Policy, Params, MyType> super;
  
private:
  /** The array size */
  size_t lenght;
  /** Number of elems */
  size_t numElems;
  
  /** 
   * Number of used elements in the second half of the table. If reduced 
   * to zero, we can safely downsize. 
   */
  size_t usedOnTail;

  /**
   * The index of the first elements on the free lists on each half.  
   */
  size_t headFreePtr; 
  size_t tailFreePtr;

  /** 
   * The ugly thingy. 
   */

  size_t & freePtr(size_t i) { 
    assert(i<lenght);
    assert(!isUsed(i));
    return *(super::directRefToValue(i).auxData<size_t>);
  }

  /**
   * Upsize is only done if the vector is full. Returns the new slot. 
   */
  size_t upsize() {
    tailFreePtr=length+1;
    numElems++;
    /* HeadFreePtr OK. */
    usedOnTail=1;
    resize(lenght <<= 1);
    for (size_t i=(lenght >> 1)+1; i < length; ++i) freePtr(i)=i+1
    return lenght >> 1;
  }

  /**
   * Downsized the structure if possible. 
   */
    
  void downsize() {
    resize(length >>= 1);
    size_t next;
    size_t curr=headFreePtr;
    /* All used on tail until proven otherwise */
    usedOnTail=length >> 1;
    /* Go through the whole empty list of the former head part 
     * (now, the whole array) and redistribute the slots. 
     * The pointers on the tail of the list are undefined: therefore 
     * we have to count. */
    for (size_t i=0; i<length-numElems; ++i) {
      assert(!isUsed(curr));
      next=freePtr(curr); 
      /* The meat: if the struct is resized to 1, everthing goes to the tail.*/
      if (curr < (length >> 1)) { /* Never if len 0*/
	/* To the new head */
	freePtr(curr)=headFreePtr;
	headFreePtr=curr;
	/* If it was the original HeadFreePtr, it points to itself now. 
	 * No need to care.  */
      } else {
	freePtr(curr)=tailFreePtr;
	tailFreePtr=curr;
	usedOnTail--;
      }
      curr=next;
    }
  }
 
                           
protected:
  AutoVec(const size_t capacity=0):
    super(SizeHelper::sizeForCapacity(capacity)), 
    length(SizeHelper::sizeForCapacity(capacity)),
    numElems(0), headFreePtr(0), tailFreePtr(length >> 1), 
    usedOnTail(0) {
      for (size_t i=0; i<length; ++i) {
	freePtr(i)=i+1;
      }
    }

  bool isUsed(size_t i) {
    assert(i<length);
    return(!super::directRefToValue(i).isEmpty());
  }
  
  /**
   * Remove the data, set it as unused, update ptrs etc.
   */

  void removeAt(const size_t i) {
    numElems--;
    if (i < (length >> 1)) {
      freePtr(i)=headFreePtr;
      headFreePtr=i;
    } else {
      freePtr(i)=tailFreePtr;
      tailFreePtr=i;
      usedOnTail--;
      /* The downsizing loop */
      while (!usedOnTail) {
	downsize();
      }
    }
  }

  size_t newElem() {
    size_t retval;
    numElems++;
    if (numElems-usedOnTail < (lenght >> 1)) {
      /* Free elements on the head, too. */
      retval=headFreePtr;
      headFreePtr=freePtr(headFreePtr);
    } else if (usedOnTail < lenght >> 1) { /* != 0 */
      retval=tailFreePtr;
      tailFreePtr=freePtr(tailFreePtr);
    } else {
      retval=upsize();
    }
    assert(!isUsed(retval));
  }

  /** In this case,  */

  size_t placeToPut() 



#endif
