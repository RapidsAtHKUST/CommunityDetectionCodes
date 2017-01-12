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

#ifndef CONTAINERS_H
#define CONTAINERS_H
#include<cassert>
#include"containers/indices/LinearHash.H"
#include"containers/indices/OrderedArray.H"
#include"containers/indices/Array.H"
#include"containers/indices/Vector.H"
#include"containers/WeightPolicy.H"
#include"containers/KeyPolicy.H"
#include"containers/indices/HashControllers.H"
#include"containers/tables/ValueTable.H"
#include"containers/tables/ExplSumTreeTable.H"
#include"containers/tables/WeightSumTable.H"
//#include"./PrefetchPolicy.H"
//#include"./UsagePolicy.H"

/** 
 * The standard parameters for a container. These are independendent of
 * of the keys'n'values for which the policies are used. 
 */

struct DefaultContainerParams {
  typedef EmbStatusPolicy StatusPolicy; 
  typedef SmallHashController<> HashController;
  static const bool HASH_ORDERED=false;
  static const unsigned treeLogBase=1;
};

/**
 * This is part of the Policy, which referes to handling of elements. 
 * Do not confuse with the StatusPolicy, which belongs to the Params.
 */

template<typename KeyType>
struct UsagePolicy {
  static const KeyType MagicEmptyKey(); /*deffault */
};

/**
 * Containers simply defining interfaces to superclasses. 
 *
 */

template<typename KeyType>
struct SetContainerPolicy: public WeightPolicy<KeyType>, 
			   public KeyPolicy<KeyType>, 
			   //public PrefetchPolicy<KeyType>, 
			   public UsagePolicy<KeyType> {};

template<typename KeyType, typename ValueType>
struct MapContainerPolicy: public WeightPolicy<ValueType>, 
			   public KeyPolicy<KeyType>, 
                           //public PrefetchPolicy<ValueType>, 
			   public UsagePolicy<KeyType> {};

  
/**
 * The basic linear set implementation. 
 *
 * @param KeyType     The type of elements stored. 
 * @param Index       An template class  of the Index concept, providing for 
 *                    the seek, iteration and removals.
 * @param Table       An template class of the Table concept. As no values are
 *                    to be stored in the Set, including this in the parameter
 *                    list might seem odd. However, if the keys and values 
 *                    are somehow bound to the same object, this makes it
 *                    possible to use all the special tricks on the Storage 
 *                    layer. 
 * @param Policy      The calculation of hash codes, marking of the usage 
 *                    status etc. Everything policy-like that depends on the 
 *                    type of the keys. Subclassing this default value is 
 *                    the thing to do in order to customize the function.
 * @param IndexParams The parameters for the Index. On this layer, we do
 *                    not know what this is. The default should be sensible. 
 * @param TableParams The parameters for the Table. Typically void, but
 *                    e.g. the base of the implicit tree might be passed here. 
 */

template<typename KeyType, 
	 template<typename, /* Key type*/ 
		  typename, /* Value, here to be passed from here as void 
			     * and the up the hierarchy */
		  typename, /* Policy */ 
		  typename, /* Params */ 
		  template<typename, /* Key */
			   typename, /* Val */
			   typename, /* Policy */
			   typename, /* Params */
			   typename  /* The index class, for CRTP */
			   > class foo> class Index=LinearHash,  
	 template<typename, /* Key */
		  typename, /* Val */
		  typename, /* Policy */
		  typename, /* Params */
		  typename  /* The derived Index for CRTP */
		  > class Table=ValueTable,	   
         typename Policy=SetContainerPolicy<KeyType>, /* Sensible default*/
	 typename Params=DefaultContainerParams>
class Set: public Index<KeyType, void, Policy, Params, Table> {
public:
  typedef Set<KeyType, Index, Table, Policy, Params> MyType;
  typedef Index<KeyType, void, Policy, Params, Table> super;
public:
  Set():super() {};
  Set(size_t capacity): super(capacity) {};

  ~Set() {
    //std::cerr << "Destroying a set\n";
  }
  /* Puts a key, returns if was already present. */

  bool put(const KeyType & key) {
    size_t loc;
    return super::forcedFind(key, loc);
  }

  /** 
   * Only the const version of this is provided. Look for the automap 
   * if you want sth else. 
   */

  bool operator[](const KeyType & key) const {
    return contains(key);
  }

  bool remove(const KeyType & key) {
    size_t loc;
    if (super::findFirst(key, loc)) {
      super::removeFrom(loc);
      /* We can assume this to be an independent operation, therefore we: */
      super::trim();
      return true;
    } else {
      return false; /* Not found. */
    }
  }

  void setValue(const KeyType & key, bool value) {
    if (value) {
      put(key);
    } else {
      remove(key);
    }
  }
  
  template<typename SourceType>
  MyType & operator=(const SourceType & src) {
    super::clear();
    //std::cerr << "Pudding:";
    for (typename SourceType::const_iterator i=src.begin();
	 !i.finished(); ++i) {
      std::cerr << " " << (*i);
      put(*i);
    }
    //std::cerr << "\n";
    return *this;
  }

  template<typename SourceType>
  MyType & operator+=(const SourceType & src) {
    //std::cerr << "The put cascade. Source size:" << src.size() << "\n";
    for (typename SourceType::const_iterator i=src.begin();
	 !i.finished(); ++i) {
	   // std::cerr << " " << (*i);
      put(*i);
    }
    //std::cerr << "Put casc done.\n";
    return *this;
  }
  
  /** The superclass's iterators are OK. */

};


template<typename KeyType, 
	 template <typename, /* Key type*/ 
		   typename, /* Value, here to be passed from here as void 
			      * and the up the hierarchy */
		   typename, /* Policy */ 
		   typename, /* Params */ 
		   template<typename, /* Key */
			    typename, /* Val */
			    typename, /* Policy */
			    typename, /* Params */
			    typename  /* The index class, for CRTP */
			    > class foo/* Table */ > class Index,
	 template<typename, /* Key */
		  typename, /* Val */
		  typename, /* Policy */
		  typename, /* Params */
		  typename  /* The derived Index for CRTP */
		  > class Table,
         typename Policy, 
	 typename Params>
struct WeightPolicy<Set<KeyType, Index, Table, Policy, Params> > {
  typedef size_t WeightType;
  static WeightType getWeight(const Set<KeyType, Index, Table, Policy, Params> 
			      & subj) {
    return subj.size();
  }
};


/**
 * The good old boring map. Aims to be similar to that in the 
 * standard library. 
 * 
 * @param KeyType     Self expl. 
 * @param ValueType   Self expl. 
 * @param Index       An template class  of the Index concept, providing for 
 *                    the seek, iteration and removals.
 * @param Table       An template class of the Table concept for the storage 
 *                    of values. 
 * @param Policy      The calculation of hash codes, marking of the usage 
 *                    status, getting weights etc. Everything policy-like that 
 *                    depends on the keys and values. Subclassing this default 
 *                    value is 
 *                    the thing to do in order to customize the function.
 * @param Params      Parameters that the Index and Table use.
 */

template<typename KeyType, 
	 typename ValueType,
	 template<typename, /* Key type*/ 
		  typename, /* Value, here to be passed from here as void 
			     * and the up the hierarchy */
		  typename, /* Policy */ 
		  typename, /* Params */ 
		  template<typename, /* Key */
			   typename, /* Val */
			   typename, /* Policy */
			   typename, /* Params */
			   typename  /* The index class, for CRTP */
			   > class foo/* Table */ > class Index=LinearHash,
	 template<typename, /* Key */
		  typename, /* Val */
		  typename, /* Policy */
		  typename, /* Params */
		  typename  /* The derived Index for CRTP */
		  > class Table=ValueTable, 
	 typename Policy=MapContainerPolicy<KeyType, ValueType>, 
	 typename Params=DefaultContainerParams>
class Map: public Index<KeyType, ValueType, Policy, Params, Table> {
private:
  typedef Map<KeyType, ValueType, Index, Table, Policy, Params> MyType;
  typedef Index<KeyType, ValueType, Policy, Params, Table> super;

protected:
  /* value_reference is inherited ok: either as a stub or as a direct ref. 
   * The stub needs a logic for initializing new vals to "zero", however. 
   */

  /**
   * This is a convenience method for initializing stubs and new elements
   * through the subscript op. We cannot use the idiom of returning the
   * table slot through a reference in the stub's constuctor.
   */

  size_t findAndInit(const KeyType & key) {
    size_t loc = 0;
    //std::cerr <<"\nFIND_AND_INIT, key:" << key;
    if (!forcedFind(key, loc)) {
      //std::cerr << ", was new\n";
      //this->refToVal(loc)=ValueType();
      /* FIXME: not always necessary */
      super::clearVal(loc);
    } else {
      //std::cerr << ", was present\n";
    }
    //std::cerr << " F&I done.\n";
    return loc;
  }

public:

  Map():super() {}
  Map(size_t capacity): super(capacity) {
    //std::cerr << "Making a map at" << this << " with size" << capacity << "\n";
  }
  ~Map() {
    //std::cerr << "Killing a map at" << this << "\n";
  }
  Map(const Map & src) {
    std::cerr << "Copy constructing at " << this << " from " << (&src) 
    << " WTF?\n";
  }
  typedef typename super::value_reference reference;
  typedef typename super::const_value_reference const_reference;

  /** 
   * The array subscript operator works as its STL counterpart:
   * if the key is not present, a new entry is generated, initialized
   * to the default value. As in std lib, there is no const-version of
   * this operator. Automap has. 
   */

  reference operator[](const KeyType & key) {
    //std::cerr << "A reference from a map " << this << " with key: " << key 
    //	      << "\n";
    return refToVal(findAndInit(key));
  }

  /**
   * ...except when the superclass has a suitable operator.
   * FIXME: this is unbeliveably ugly. 
   */
 
  const_reference operator[](const KeyType & key) const {
    return super::operator[](key);
  }
     
  void setValue(const KeyType & key, const ValueType & value) {
    size_t loc = 0;
    forcedFind(key, loc);
    super::setValue(loc, value);
  }

  /**
   * The value_stub has to automagically set new elems to zero.
   * Only the constructor has to be overridden: 
   * the destructor is to be defined in a subclass and we have nothing 
   * to finalize here. 
   */
  
  class stub: public super::value_stub {
  protected:
    stub(MyType & target, const KeyType & key): 
      super::value_stub(target, target.findAndInit(key)) {
	//std::cerr << "Map's stub constructed, lenght:" 
	//<< target.getTableSize() << ", target:" << (&target) << "\n";
      } 
    void _lce_finalize() {super::value_stub::_lce_finalize();}
  public:    
    stub & operator=(const ValueType & source) {
      super::value_stub::operator=(source);
      return *this;
    }    
  };
  
  typedef stub value_stub;

  /**
   * The iterator simply adds accessing the values to the Index's one. 
   */

  class iterator: public super::iterator {
    //friend class MyType;
    friend class Map<KeyType, ValueType, Index, Table, Policy, Params>;
  private:
    //iterator() {}
  protected:
    iterator(MyType * tgt):super::iterator(tgt) {}
  public:
    typedef typename MyType::reference reference;
    typedef typename MyType::const_reference const_reference;
    /* reference refers to superclass's refToValue */
    reference value() {
      return 
	((MyType *) super::iterator::target)->refToVal(super::iterator::loc);
    }
    const_reference constValue() const {
      return ((MyType *) super::iterator::target)->constRefToVal(super::iterator::loc);
    }
  };

  class const_iterator: public super::const_iterator {
    /* Boring...*/
    //friend class MyType;
    friend class Map<KeyType, ValueType, Index, Table, Policy, Params>;
    typedef typename super::const_iterator super_iter;
  private:
    const_iterator() {}
  protected:
    const_iterator(const MyType * tgt):super::const_iterator(tgt) {
      //std::cerr << "Initing const iter of Map\n";
    }
  public:
    typedef typename MyType::const_reference const_reference;
    /* reference refers to superclass's refToValue */
    const_reference value() {
      return ((MyType *) super_iter::target)->constRefToVal(super_iter::loc);
    }
  };

  friend class iterator;
  friend class const_iterator;
  
  iterator begin() {return iterator(this);}
  const_iterator begin() const {return const_iterator(this);}

  
  template<typename SourceType>
  MyType & operator=(const SourceType & src) {
    super::clear();
    //std::cerr << "Pudding:";
    for (typename SourceType::const_iterator i=src.begin();
	 !i.finished(); ++i) {
      //std::cerr << " " << (*i);
      this->operator[](*i)=i.value();
    }
    //std::cerr << "\n";
    return *this;
  }
  
  template<typename SourceType>
  MyType & operator+=(const SourceType & src) {
    //std::cerr << "The put cascade. Source size:" << src.size() << "\n";
    for (typename SourceType::const_iterator i=src.begin();!i.finished();++i) {
      this->operator[](*i)+=i.value();
    }
    //std::cerr << "Put casc done.\n";
    return *this;
  }
  
};


/** 
 * The multimap. Not necessarily very efficient in the case that repeated
 * keys are very common: use a map to sets in that case. 
 *
 * 
 * 
 */

// template<typename KeyType, typename ValueType, 
// 	 typename Policy=ContainerPolicy<std::pair<KeyType, ValueType> >,
// 	 template<typename, typename> class Impl=hash,
// class Multimap: public Impl<std::pair<KeyType, ValueType>, Policy> {
// private:
//   typedef Multimap<KeyType, ValueType, Policy, Impl> MyType;
//   typedef Impl<Pair<KeyType, ValueType>, Policy> super;
// public:

//   Multimap():super() {};
//   Multimap(size_t capacity): super(capacity) {};

//   /** 
//    * The array subscript operator works as its STL counterpart:
//    * if the key is not present, a new entry is generated, initialized
//    * to the default value. As in std lib, there is no const-version of
//    * this operator. Automap has. 
//    */

//   reference operator[](const KeyType & key) {
//     size_t loc;
//     if (!placeToPut(key, loc)) {
//       initValAt(loc);
//     }
//     return refToVal(loc);
//   }

//   class iterator: public super::iterator {
//   private:
//     iterator() {}
//   protected:
//     iterator(MyType * tgt):super::iterator(tgt) {}
//   public:
//     /* reference refers to superclass's refToValue */
//     reference value() {return refToVal(loc);}
//   };

//   class const_iterator: public super::const_iterator {
//   private:
//     iterator() {}
//   protected:
//     iterator(const MyType * tgt):super::const_iterator(tgt) {}
//   public:
//     /* reference refers to superclass's refToValue */
//     const_reference value() {return constRefToVal(loc);}
//   };
  
//   iterator begin() {return iterator(this);}
//   const_iterator begin() const {return const_iterator(this);}

// };


/**
 * We assume that the const_reference of the superclass is either 
 * a direct value or a const reference. This pair of templates 
 * helps us to return a
 * sensible default value in any case.
 */

/**
 * The default one, used in the case when the const_reference is
 * really an object init'd to the default value.
 */

// template<typename ValueType>
// struct DefaultProvider {
//   static ValueType default() const {return ValueType();}
// };

// /**
//  * A specialization for real const references, requiring a 
//  * static (class-spesific) member. 
//  */

// template<typename ValueType>
// struct DefaultProvider<ValueType &> {
//   static const ValueType def;
//   /* Reference to the default value: */
//   static const ValueType & default() const {return def;}
// }



/** 
 * This has nothing to do with car trips. 
 *
 * A map, in which every possible key maps to the default value of the 
 * ValueType, unless set otherwise. If a key is set to map to the default,
 * it is automagically destroyed. Extremely useful for sparse vectors
 * etc. Iterator only iterates over elements which differ from the 
 * default value.
 * 
 * For convenience, values are returned by value.
 * 
 * All the parameters are the same as for the Map.
 * 
 * @param KeyType     The type of elements stored. 
 * @param Index       An template class  of the Index concept, providing for 
 *                    the seek, iteration and removals.
 * @param Table       An template class of the Table concept for the storage 
 *                    of values. 
 * @param Policy      The calculation of hash codes, marking of the usage 
 *                    status, getting weights etc. Everything policy-like that 
 *                    depends on the keys and values. Subclassing this default 
 *                    value is 
 *                    the thing to do in order to customize the function.
 * @param IndexParams The parameters for the Index. On this layer, we do
 *                    not know what this is. The default should be sensible. 
 * @param TableParams The parameters for the Table. Typically void, but
 *                    e.g. the base of the implicit tree might be passed here. 
 */

template<typename KeyType, 
	 typename ValueType,
	 template <typename, /* Key type*/ 
		   typename, /* Value, here to be passed from here as void 
			      * and the up the hierarchy */
		   typename, /* Policy */ 
		   typename, /* Params */ 
		   template<typename, /* Key */
			    typename, /* Val */
			    typename, /* Policy */
			    typename, /* Params */
			    typename  /* The index class, for CRTP */
			    > class foo/* Table */ > class Index=LinearHash,
	 template<typename, /* Key */
		  typename, /* Val */
		  typename, /* Policy */
		  typename, /* Params */
		  typename  /* The derived Index for CRTP */
		  > class Table=ValueTable,
         typename Policy=MapContainerPolicy<KeyType, ValueType>, 
	 typename Params=DefaultContainerParams>
class AutoMap: public Map<KeyType, ValueType, Index, Table,  
			  Policy, Params> {
  typedef AutoMap<KeyType, ValueType, Index, Table, Policy, Params> MyType;
  typedef Map<KeyType, ValueType, Index, Table, Policy, Params> super;
  
public:
  
  //typedef typename super::const_iterator const_iterator;

  AutoMap(): super() {};
  AutoMap(size_t capacity): super(capacity) {};

  /** 
   * A class used for non-const references to values. The StubBase
   * is a convenience class for chaining operations.
   */

  template<bool AutoFinalize> 
  class stub:public super::stub {
    friend class  AutoMap<KeyType, ValueType, Index, Table, Policy, Params>;
    typedef typename super::stub SuperStub;
  protected:
    stub(MyType & target, const KeyType & key):
      SuperStub(target, key) {
      //std::cerr << "Initing AM:s stub. TS:" << target.getTableSize() << "\n";
    }

    /**
     * Nothing to update here. 
     */

    void _lce_update() {
      SuperStub::_lce_update();
    }

    /** 
     * The finalizer has the meat. We finalize the 
     * the superclass first.
     */
    void _lce_finalize() {
      if (SuperStub::ref()==ValueType()) { // ORIGINALLY: if ((*this)==ValueType()) {
	SuperStub::_lce_finalize();
	//std::cerr << "Automap removing from loc " << SuperStub::loc << "\n";
	((MyType &) SuperStub::target).removeFrom(SuperStub::loc);
      } else {
	//std::cerr << "Finalizing AM:stub. TS:";
	//std::cerr << ((MyType &) target).getTableSize() << "\n";
	SuperStub::_lce_finalize();
	//std::cerr << "done AM.";
      }               
    }
    
    bool _lce_upToDate() {
      return SuperStub::_lce_upToDate();
    }

  public:
    
    stub & operator=(const ValueType & source) {
      SuperStub::operator=(source);
      return (*this);
    }
    ~stub() {
      if (AutoFinalize) _lce_finalize();
    }
  };

  friend class stub<true>;
  friend class stub<false>;

  typedef stub<true> reference;
  typedef stub<false> value_stub;

  /**
   * The superclass deals with everything. 
   */

  reference operator[](const KeyType & key) {
    /* All happens through the superclass's stub.  */
    return stub<true>(*this, key);
  }
  /**
   * For reasons I cannot tell you now, we return the values by value.
   */

  ValueType operator[](const KeyType & key) const {
    size_t loc;
    //std::cerr << "In oper[]";
    if (findFirst(key, loc)) {
      return super::constRefToVal(loc);
    } else {
      //std::cerr << "Foobar";
      return ValueType();
    }
  }

  void setValue(const KeyType & key, const ValueType & value) {
    if (value==ValueType()) {
      this->remove(key);
      assert(!contains(key));
    } else {
      super::setValue(key, value);
    }
  }
  
  class iterator:public super::iterator {
    typedef typename super::iterator SuperIter;
    friend class AutoMap<KeyType, ValueType, Index, Table, Policy, Params>;
  public:
    //typedef typename SuperIter::reference reference;
  private:
    //iterator() {};
  protected:
    iterator(MyType * tgt): SuperIter(tgt) {}
  public:   
    typedef typename super::iterator::reference reference;
    /* Practically everything in the SuperIter simply works. Nuff said. */

    /**
     * The only difference to the superclass's destructor is that in the case
     * we are not finished, we must check for the current element:
     */
    ~iterator() {
      if (!SuperIter::finished() && SuperIter::value()==ValueType()) {
	SuperIter::remove();
      }
    }

    iterator & operator++() {
      //std::cerr << "AutoMaps:s ++:";
      assert(SuperIter::target->isLegal());
      if (SuperIter::constValue()==ValueType()) {
	//std::cerr << "Removing:";
	SuperIter::remove();
	//std::cerr << "done. Asserting:"; 
	assert(((MyType *) SuperIter::target)->isLegal());
      } else {
	//std::cerr << "++ing super.";
	assert(((MyType *) SuperIter::target)->isLegal());
	SuperIter::operator++();
	assert(((MyType *) SuperIter::target)->isLegal());
      }
      //std::cerr << "Done.";
      return *this;
    } 

  };    
  /* The const_iterator of the superclass is OK. */
  typedef typename super::const_iterator const_iterator;
  
  bool keyLegal(const KeyType & key) const {
    if (!super::keyLegal(key)) {
      std::cerr << "Super failed!";
      return false;
    }
    if (contains(key)) {
      if (operator[](key) == ValueType()) {
	std::cerr << "Shit! Key with def value present!\n";
	std::cerr << "Key:" << key << ", value" << operator[](key);
	return false;
      }
    }
    return true;
  }

  bool isLegal() const {
    if (!super::isLegal()) return false;
    for (const_iterator i=super::begin(); !i.finished(); ++i) {
      if (i.value()==ValueType()) return false;
    } 
    return true;
  }
};

template<typename KeyType, 
	 typename ValueType,
	 template <typename, /* Key type*/ 
		   typename, /* Value, here to be passed from here as void 
			      * and the up the hierarchy */
		   typename, /* Policy */ 
		   typename, /* Params */ 
		   template<typename, /* Key */
			    typename, /* Val */
			    typename, /* Policy */
			    typename, /* Params */
			    typename  /* The index class, for CRTP */
			    > class foo/* Table */ > class Index,
	 template<typename, /* Key */
		  typename, /* Val */
		  typename, /* Policy */
		  typename, /* Params */
		  typename  /* The derived Index for CRTP */
		  > class Table,
         typename Policy, 
	 typename Params>
struct WeightPolicy<AutoMap<KeyType, ValueType, Index, Table, Policy, Params
			    > >: public AutoMap<KeyType, ValueType, Index, 
						Table, Policy, Params
						>::DefaultWeightPolicy {};

/** NB! When using ExplSumTreeTable, the AutoMap::DefaultWeightPolicy
 * is WeightWeightPolicy<ExplSumTreeTable<...> >, which is defined in
 * ExplSumTreeTable. WeightWeightPolicy is again defined in
 * ContainerPolicies.H.
 */


/** 
 * A specialization of the Automap for boolean values. The point is that
 * we do not need storage of values: everything present in the map 
 * has the default value (true). We thus just pretend. 
 * 
 * Quite similar to the Set, except that we do have a stub. In addition,
 * the default policy is the one for a _set_.
 */

template<typename KeyType, 
	 template<typename, /* Key type*/ 
		  typename, /* Value, here to be passed from here as void 
			     * and the up the hierarchy */
		  typename, /* Policy */ 
		  typename, /* Params */ 
		  template<typename, /* Key */
			   typename, /* Val */
			   typename, /* Policy */
			   typename, /* Params */
			   typename  /* The index class, for CRTP */
			   > class foo /* Table */ > class Index,  
	 template<typename, /* Key */
		  typename, /* Val */
		  typename, /* Policy */
		  typename, /* Params */
		  typename  /* The derived Index for CRTP */
		  > class Table,
	 typename Policy, 
	 typename Params>
class AutoMap<KeyType, bool, Index, Table, Policy, Params>: 
  public Set<KeyType, Index, Table, Policy, Params> {
  typedef AutoMap<KeyType, bool, Index, Table, Policy, Params> MyType;
  typedef Set<KeyType, Index, Table, Policy, Params> super;

public:

  AutoMap():super() {}
  AutoMap(size_t capacity): super(capacity) {};
 
  typedef bool value;
  typedef bool const_reference;

  /** The stub inherits from nothing. Sets do not have stubs. */
			     
  class stub {
    friend class AutoMap<KeyType, bool, Index, Table, Policy, Params>;
  private:
    stub() {};
  protected:
    MyType & target;
    const KeyType _key; /* Can we do this? I guess so. */ 
    stub(MyType & tgt, const KeyType & key): target(tgt), _key(key) {};
    void _lce_update(); 
    bool _lce_upToDate() {return true; /* Always. */}
  public:
    void _lce_finalize() {/*std::cerr << "foo";*/} /* Do nothing, everything is already ready. */
    operator bool(){
      return target.contains(_key);
    }
    bool operator=(bool val) {
      size_t loc;
      if (val) {
	target.forcedFind(_key, loc);
      } else {
	if (target.findFirst(_key, loc)) target.removeFrom(loc);
      }
      return val;
    }
  };
  
  const_reference operator[](const KeyType & key) const {
    return contains(key);
  }

  friend class stub;
			     
  typedef stub reference;
  typedef stub value_stub;
			     
  reference operator[](const KeyType & key) {
    return stub(*this, key);
  }
			         
  /** 
   * In order to allow value manipulation, the iterator contains a local 
   * boolean, which is used to judge whether to remove the 
   * element pointed to. Behold:
   */

  class iterator: public super::iterator {
  protected:
    //friend class MyType;
    friend class AutoMap<KeyType, bool, Index, Table, Policy, Params>;
    /* This is used to store the value set */
    bool val;
    
    iterator(MyType * target): super::iterator(target), val(true) {}
      
  public:

    ~iterator() {
      /* If we point to the end, the value is necessarily true. */
      if (!val) { 
	super::iterator.remove();
      }
    }

    void remove() {
      val=true; /* Whatta f*ck? Oh yes, the next one is used. */
      super::iterator::remove();
    }

    iterator & operator++() {
      if (!val) {
	/* This should move it onwards: */
	super::iterator::remove();
	val=true;
      } else {
	super::iterator::operator++();
      } 
      return (*this);
    }
    bool & value() {return val;}
  };
  
  class const_iterator: public super::const_iterator { 
  protected:
    //friend class MyType;
    friend class AutoMap<KeyType, bool, Index, Table, Policy, Params>;
    const_iterator(const MyType * const target): 
      super::const_iterator(target) {}
  public:
    bool value() {return true;}
  };
  
  iterator begin() {return iterator(this);}
  const_iterator begin() const {return const_iterator(this);}

  void setValue(const KeyType & key, bool value) {
    if (value) {
      put(key);
    } else {
      remove(key);
    }
  }

  /* super:s leg check is OK */			     
};

template<typename KeyType, 
	 template <typename, /* Key type*/ 
		   typename, /* Value, here to be passed from here as void 
			      * and the up the hierarchy */
		   typename, /* Policy */ 
		   typename, /* Params */ 
		   template<typename, /* Key */
			    typename, /* Val */
			    typename, /* Policy */
			    typename, /* Params */
			    typename  /* The index class, for CRTP */
			    > class foo/* Table */ > class Index,
	 template<typename, /* Key */
		  typename, /* Val */
		  typename, /* Policy */
		  typename, /* Params */
		  typename  /* The derived Index for CRTP */
		  > class Table,
         typename Policy, 
	 typename Params>
struct WeightPolicy<AutoMap<KeyType, bool, Index, Table, Policy, Params
			    > > {
  typedef size_t WeightType;
  static WeightType getWeight(const AutoMap<KeyType, bool, Index, Table, 
			      Policy, Params> & subj) {
    return subj.size();
  }
};




#endif    





