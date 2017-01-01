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

#ifndef LCE_TABLE_WITH_STATUS
#define LCE_TABLE_WITH_STATUS
#include<cassert>
#include "../ArrayBase.H"
#include "../Pair.H"
#include<string> 
#ifndef NODEBUG
#include<iostream>
#endif


struct EmbStatusPolicy {};
struct ExtStatusPolicy {};

/**
 * The non-specialized template uses an implicit status.
 */

template<typename KeyType, typename ValueType, 
	 typename Policy, typename Params,/* To be passed up */
	 template<typename, typename, typename, 
		  typename, typename> class Table,
	 typename Status, 
	 typename Index>
class TableWithStatus: 
  public Table<KeyType, ValueType, Policy, Params, Index> {
  typedef TableWithStatus<KeyType, ValueType, Policy, 
			  Params, Table, Status, Index> MyType;
  typedef Table<KeyType, ValueType, Policy, Params, Index> super;
protected:
  TableWithStatus(size_t size=0):super(size) {
    for (size_t i=0; i<size; ++i) {
      setAsEmpty(i);
    }
  }

  KeyType & refToKey(const size_t i) {
    //assert(!isUsed(i));
    return super::refToKey(i);
  }

  const KeyType & constRefToKey(const size_t i) const {
    assert(isUsed(i));
    return super::constRefToKey(i);
  }		    

public:
  bool isUsed(const size_t i) const {
    return super::constRefToKey(i) != Policy::MagicEmptyKey;
  }

protected:
  void setAsUsed(const size_t i) {} 
  void setAsEmpty(const size_t i) {
    super::refToKey(i)=Policy::MagicEmptyKey;
  }
};

/** 
 * Specialization for external usage of statuses...
 */

template<typename KeyType, typename ValueType, 
	 typename Policy, typename Params,/* To be passed up */
	 template<typename, typename, typename, 
		  typename, typename> class Table,
	 typename Index>
class TableWithStatus<KeyType, ValueType, Policy, Params, Table,
		      ExtStatusPolicy, Index>: 
  public Table<KeyType, ValueType, Policy, Params, Index> {
  typedef TableWithStatus<KeyType, ValueType, Policy, 
			  Params, Table, ExtStatusPolicy, Index> MyType;
  typedef Table<KeyType, ValueType, Policy, Params, Index> super;
private:
  ArrayBase<bool> status;
  
protected:

  void shallowMoveTo(MyType & dest) {
    status.shallowMoveTo(dest.status);
    super::shallowMoveTo(dest);
  }

  /** 
   * The constructor for a table of given size. The status 
   * is automagically set as false: otherwise, manipulating 
   * the packed array would be a nuisance. 
   */
  TableWithStatus(const size_t size=0): super(size), status(size) {}
  
  /** The standard destructor will do fine. */

  void resize(size_t newSize, size_t oldSize) {
    super::resize(newSize, oldSize);
    status.resize(newSize, oldSize);
  }

  /** We only move elemenst that are used, so no need for real copy. Just set. 
   */
  
  void copyElemTo(MyType & dest, const size_t loc, const size_t i) {
    dest.status[loc]=true;
    super::copyElemTo(dest, loc, i);
  } 
  
public:
  
  void prefetch(const size_t i) const {
    super::prefetch(i);
    status.prefetch(i);
  }

protected:
  void setAsUsed(const size_t i) {status[i]=true;}
  void setAsEmpty(const size_t i) {status[i]=false;}
public:
  bool isUsed(const size_t i) const {return status[i];}  
};

/**
 * Embedded storage of statuses 
 */
template<typename KeyType, typename ValueType, 
	 typename Policy, typename Params,/* To be passed up */
	 template<typename, typename, typename, 
		  typename, typename> class Table,
	 typename Index>
class TableWithStatus<KeyType, ValueType, Policy, Params, Table,
		      EmbStatusPolicy, Index>: 
  public Table<Pair<KeyType,bool>, ValueType, Policy, Params, Index> {
  typedef TableWithStatus<KeyType, ValueType, Policy, 
			  Params, Table, EmbStatusPolicy, Index> MyType;
  typedef Table<Pair<KeyType,bool>, ValueType, Policy, Params, Index> super;
  
  
protected:
  	
  TableWithStatus(size_t size=0):super(size) {
    for (size_t i=0; i<size; ++i) {
      setAsEmpty(i);
    }
  }

  const KeyType & constRefToKey(size_t i) const {
    return super::constRefToKey(i).first();
  }

  KeyType & refToKey(size_t i) {
    return super::refToKey(i).first();
  }	    
public:
  bool isUsed(size_t i) const {
    return super::constRefToKey(i).second()==true;
  }
protected:
  void setAsUsed(size_t i) {
    super::refToKey(i).second()=true;
  }

  void setAsEmpty(size_t i) {
    //std::cerr << "Setting empty at" << i << "\n";
    super::refToKey(i).second()=false;
  }
		    


};


// template<typename KeyType, typename ValueType, typename Policy, 
// 	 typename StatusPolicy, 
// 	 template <typename, typename, typename> class Cont>
// struct PrefetchPolicy<TableWithStatus<KeyType, ValueType, Policy, 
// 				      StatusPolicy, Cont>, depth>: 
//   public PrefetchPolicy<Cont<KeyType,ValueType, Policy>, depth> {
// private:
//   typedef TableWithStatus<KeyType, ValueType, Policy, 
// 			  StatusPolicy, Cont> TargetType;

//   void safe_prefetch(const TargetType & tgt, size_t loc=0) {
//     if (tgt.isUsed(loc)) {
//       PrefetchPolicy<Cont<KeyType,ValueType, Policy>, 
// 	depth-1>::prefetch(tgt, loc);
//     }
//   }
// };



#endif

