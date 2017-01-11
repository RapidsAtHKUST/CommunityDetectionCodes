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

#ifndef LCE_NETS_H
#define LCE_NETS_H
#define NDEBUG /* Comment this out to evaluate asserts. */
#include<cassert>
#include<cstdlib>
#include<climits>
#include<vector>
#include"Containers.H"
#include<climits>

#ifdef NDEBUG // Remove the NDEBUG flag, if set, so that asserts are
#undef NDEBUG // on by default in all codes that include Nets.H.
#endif

/** 
 * Some policies. We want to use magic values for the status. 
 */

template<typename EdgeData>
struct NetEdgePolicy: public MapContainerPolicy<size_t, EdgeData> {
  static const size_t MagicEmptyKey=UINT_MAX;
};

struct NetEdgeParams: public DefaultContainerParams {
  typedef void StatusPolicy;
};

template<typename _EdgeData,
	 template<typename, typename, typename, typename, typename
		  > class EdgeTable=ValueTable,
	 template<typename, typename, typename, typename, typename
		  > class NodeTable=ValueTable,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table > class EdgeIndex=LinearHash,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table> class NodeIndex=Vector>
class SymmNet:public Map<size_t, 
			 AutoMap<size_t, _EdgeData, EdgeIndex, EdgeTable, 
				 NetEdgePolicy<_EdgeData>, NetEdgeParams>, 
			 NodeIndex, NodeTable> {
private:

  // The network size as reported to the user. Corresponds to the
  // largest explicitely set net size.
  size_t virtual_size;

public:
  typedef AutoMap<size_t, _EdgeData, EdgeIndex, EdgeTable, 
		  NetEdgePolicy<_EdgeData>, NetEdgeParams> EdgeMap;
  typedef Map<size_t, EdgeMap, NodeIndex, NodeTable>  NodeMap;
  typedef SymmNet<_EdgeData, EdgeTable, NodeTable, 
		  EdgeIndex, NodeIndex> MyType;
  typedef NodeMap super;
  typedef _EdgeData EdgeData;

  SymmNet(size_t capacity=0): super(capacity), virtual_size(capacity) {}

  static const EdgeMap emptyEdgeMap;
  static const EdgeData emptyEdgeData;

protected:
  /** Private methods helping the iterators, stubs etc.  */
  void int_Set(const size_t source,const size_t dest, const EdgeData & value) {
    assert(source != dest);
    //std::cerr << "To int set";
    /* The following works regardless of whether we use stubs or not. */
    (&(super::operator[](source)))->setValue(dest,value);
    //std::cout << "Value " << value << " set to " << source << "->" << dest << "\n";
    //std::cerr << "Int set!";
    assert (operator()(source)[dest]==value);
  }
  
  EdgeMap & int_Edges(const size_t source) {
    return super::operator[](source);
  }

  bool edgesLegal(const size_t source, const size_t dest) const {
    if (!operator()(source).keyLegal(dest)) {
      std::cerr << "source-dest illegal";
      return false;
    } 
    if (!operator()(dest).keyLegal(source)) {
      std::cerr << "dest-source illegal";
      return false;
    } 
    if (operator()(source)[dest]!=operator()(dest)[source]) { 
      std::cerr << "Asymm:";
      std::cerr << operator[](source)[dest] << "," 
      << operator()(dest)[source];
      return false;
    }
    return true;
  }    

  bool localLegal(const size_t source, const size_t dest) const {
    if (!(this->keyLegal(source))) {
      std::cerr << "Source not legal\n";
      return false;
    }
    if (!(this->keyLegal(dest))) {
      std::cerr << "Dest not legal\n";
      return false;
    } 
    return edgesLegal(source, dest);
  }

public:

  /**
   * Returns the virtual size of the network. To access the size of
   * the underlying data structure, use super::size().
   */
  size_t size() const {
    return virtual_size;
  }

  /**
   * Explicit resize of the network. Also changes the virtual size to
   * the new size, since this is what the user knows.
   */
  void resize(const size_t newSize) {
    this->virtualSize = newSize;
    super::resize(newSize);
  }	       

private:

  /**
   * Returns the true size of the underlying structure, not the
   * virtual size shown to the user.
   */
  size_t structure_size() const {
    return super::size();
  }

  /** 
   * Increase size to max{2*old_size, i+1}. Called only when the new
   * index does not fit in, and therefore we always have 
   * i+1 > this->virtual_size.
   */
  size_t increase_size(const size_t i) {
    const size_t newSize = (2*super::size() > i ? 2*super::size() : i+1);
    super::resize(newSize);
    //std::cerr << "Resized to " << newSize << ", virtual size: " << this->virtual_size << "\n";
    return newSize;
  }

public:

  const MyType & c() const {
    return *(const_cast<const MyType *>(this));
  }

  const EdgeMap & operator()(const size_t i) const {
    // Returns an empty EdgeMap if i does not exist in the net.
    if (i >= super::size()) return MyType::emptyEdgeMap;
    return super::operator[](i);
  }

  const EdgeData & operator()(const size_t i, const size_t j) const {
    // Returns an empty EdgeData if i or j does not exist in the net.
    if (i >= super::size() || j >= super::size()) return MyType::emptyEdgeData;
    return super::operator[](i)[j];
  }

  class Node;
 
  class Edge:public EdgeMap::value_stub {
    friend class Node;
    Node & node;
    const size_t dest;
  protected:
    Edge(Node & source, EdgeMap & myRef, const size_t dst):
      EdgeMap::value_stub(myRef, dst), node(source), dest(dst) {
      //std::cerr << "Edge stub init";
      assert(source.myIndex!=dst);
      /* Symmetry req. Bloody hell: we cannot assert legality here. */
    }
  public:

    ~Edge() {
      //std::cerr << "Deleting edge, value:" << (*this) << "\n"; //EdgeMap::value_stub::ref() << "\n";
      /* We write tha data on the second end of the edge...*/
      
      node.getNet().int_Set(dest,node.myIndex,*this);
      //std::cerr << "Other end set, value:" 
      //<< node.getConstNet()[dest][node.myIndex] 
      //<< "\n";
      /* ...using the superclass's methods: we do not want 
       * an eternal loop. */ 
      EdgeMap::value_stub::_lce_finalize();
      //std::cerr << "This end:" 
      //<< node.getConstNet()[node.myIndex][dest]
	     //<< ", but" << (*this) << "\n";
     
      //std::cerr << "Finished\n";
      /* The node from which this was called should be finalized OK. */
      /* Cannot assert fully here, as teh nodes's stub is yet 
       * to be finalized. */
      assert(node.getConstNet().edgesLegal(node.myIndex, dest));
    }
    
    Edge & operator=(const EdgeData & src) {
      EdgeMap::value_stub::operator=(src);
      return (*this);
    }    
  };
  
  friend class Edge;

  /* The EdgeIter does not update the container 
   * until destructed. container on the fly. Easy to change 
   * the default behaviour. */

  class EdgeIter:public EdgeMap::iterator {
  private:
    friend class Node;
    Node & node;
  protected:
    EdgeIter(Node & source): 
      EdgeMap::iterator(&(source.ref())),
      node(source) {
      assert(this->finished() || 
	     node.getConstNet().edgesLegal(**this, node.myIndex));
    }

  public:    
    typedef typename EdgeMap::iterator::reference reference;
    EdgeIter & operator++() {
      //std::cerr << "Setting the edge from:" << (**this) << " to:"; 
      //std::cerr << node.myIndex << " to value:";
      //std::cerr << this->constValue() << "\n";
      node.getNet().int_Set(**this, node.myIndex, this->constValue());
      assert(node.getConstNet()[**this].isLegal());
      node._lce_update();
      assert(node.getConstNet()[**this][node.myIndex]
	     ==node.getConstNet()[node.myIndex][**this]);
      EdgeMap::iterator::operator++();
      assert(this->finished() || 
	     node.getConstNet().localLegal(**this, node.myIndex)); 
      /* this would update the container: */
      /* node._lce_update();*/
      return *this;
    }
  };

  class Node:public NodeMap::value_stub {
    friend class 
    SymmNet<_EdgeData, EdgeTable, NodeTable, EdgeIndex, NodeIndex>;
    typedef typename NodeMap::value_stub SuperStub;
    friend class Edge;
    friend class EdgeIter;
    friend class NodeIter;
    const size_t myIndex;
  private:
    MyType & getNet() {return (MyType &) SuperStub::target;}
    const MyType & getConstNet() {return getNet().c();}
  protected:
    Node(MyType & net, size_t index): NodeMap::value_stub(net, index),
				      myIndex(index) {
      //std::cerr << "Node stub inited";
    }
  public:
    Edge operator[](size_t i) {
      // Resize if necessary.
      if ( i >= getNet().size() ) {
	if (i >= getNet().structure_size()) getNet().increase_size(i);
	getNet().virtual_size = i+1;
      }
      //assert(getConstNet().localLegal(myIndex, i));
      /* This would be the place to prefetch. */
      //std::cerr << "Gettin' edge\n";
      return Edge(*this, SuperStub::ref(), i);
    }
    ~Node() {
      //std::cerr << "Finalizing node stub";
      NodeMap::value_stub::_lce_finalize();
      assert(getConstNet().keyLegal(myIndex));
    }

    EdgeIter begin() {
      return EdgeIter(*this);
    } 

    template<typename SourceType>
    Node & operator=(const SourceType & src) {
      for (typename SourceType::const_iterator i=src.begin(); 
	   !i.finished(); 
	   ++i) {
	this->operator[](*i)=i.value();
      }
      return *this;
    }

    /*
    operator EdgeMap() {
      return SuperStub::target;
    }
    */
  };
  
  friend class Node;

  class NodeIter:public NodeMap::iterator {
  protected:
    friend class 
    SymmNet<_EdgeData, EdgeTable, NodeTable, EdgeIndex, NodeIndex>;
    typedef typename NodeMap::iterator SuperIter;
    NodeIter(MyType * target):NodeMap::iterator(target)  {}
  public:
    Node value() {
      return Node(*((MyType *) SuperIter::target), *(*this));
    }
    /* All else OK. */
  };

  typedef NodeIter iterator;
  typedef typename super::const_iterator const_iterator;
  typedef EdgeIter edge_iterator;

  iterator begin() {return NodeIter(this);}
  
  typedef typename EdgeMap::const_iterator const_edge_iterator;

  Node operator[](const size_t i) {
    // Resize net if necessary, initializing with empty values.
    if ( i >= size() ) {
      if (i >= structure_size()) increase_size(i);
      this->virtual_size = i+1;
    }
    assert(this->keyLegal(i));
    return Node(*this, i);
  }

  typename NodeMap::const_reference operator[](const size_t i) const {
    //assert(this->keyLegal(i));
    return NodeMap::operator[](i);
  }

  bool isLegal() const {
    if (!super::isLegal()) {return false;}
    for (const_iterator i=super::begin(); !i.finished(); ++i) {
      if (!i.value().isLegal()) return false;
      for (const_edge_iterator j=i.value().begin();
	   !j.finished(); ++j) {
	if (j.value() != (*this)[*j][*i]) {
	  std::cerr << "Bummer!\n";
	  return false;
	}
      }
    }
    return true;
  }

};


// Definitions for the static template variables. Yes, they really
// need to be that complicated.
template<typename _EdgeData,
	 template<typename, typename, typename, typename, typename
		  > class EdgeTable,
	 template<typename, typename, typename, typename, typename
		  > class NodeTable,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table > class EdgeIndex,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table> class NodeIndex> 
const AutoMap<size_t, _EdgeData, EdgeIndex, EdgeTable, 
	      NetEdgePolicy<_EdgeData>, NetEdgeParams>
SymmNet<_EdgeData, EdgeTable, NodeTable, EdgeIndex, NodeIndex>::emptyEdgeMap;

template<typename _EdgeData,
	 template<typename, typename, typename, typename, typename
		  > class EdgeTable,
	 template<typename, typename, typename, typename, typename
		  > class NodeTable,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table > class EdgeIndex,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table> class NodeIndex>
const _EdgeData
SymmNet<_EdgeData, EdgeTable, NodeTable, EdgeIndex, NodeIndex>::emptyEdgeData;


/**
 * Class for the weights of directed edges. Contains the weight in
 * both directions in each node, allowing for iteration over all
 * incident edges.  For users of DirNet, this implementation is almost
 * completely invisible. The edge net[i][j] may be used as if it
 * containted only the true edge i->j.
 */

template<typename EdgeData>
class WeightPair {
private:
  EdgeData _out;
  EdgeData _in;
public:
  typedef WeightPair<EdgeData> MyType;

  /* // Implicit conversion of Edge or WeightPair to EdgeData is
     // used instead of this. EdgeData must be streamable.
  friend std::ostream& operator << (std::ostream & os, const MyType & obj) {
    os << "(" << obj.out() << "," << obj.in() << ")";
    //os << obj.out();
    return os;
  }
  */

  /* Constructors */
  WeightPair(): _out(), _in() {}
  WeightPair(EdgeData out, EdgeData in): _out(out), _in(in) {}
  
  EdgeData & out() { return _out; }
  EdgeData & in() { return _in; }
  const EdgeData & out() const { return _out; }
  const EdgeData & in() const { return _in; }

  /* These two operators are needed by DirNet.
   */
  bool operator==(const MyType & other) const {
    return _in == other.in() && _out == other.out(); 
  }

  bool operator!=(const MyType & other) const {
    return _in != other.in() || _out != other.out(); 
  }

  /* Implicit conversion to EdgeData gives the outgoing edge.
   */
  operator EdgeData() const {
    //std::cerr << "Implicit conversion in WeightPair!\n";
    return _out;
  }

  /* Following operators are implemented to make this class
   * as transparent as possible. All normal operations with EdgeType
   * as the other operand work on the outgoing edge alone. This
   * allows writing net[i][j] += 1, etc.
   */
  template<typename SourceType>
  MyType & operator=(const SourceType & src) {
    _out = src;
    return *this;
  }

  template<typename SourceType>
  MyType & operator+=(const SourceType & src) {
    _out+=src;
    return *this;
  }

  template<typename SourceType>
  MyType & operator-=(const SourceType & src) {
    _out-=src;
    return *this;
  }

  template<typename SourceType>
  MyType & operator*=(const SourceType & src) {
    _out*=src;
    return *this;
  }

  template<typename SourceType>
  MyType & operator/=(const SourceType & src) {
    _out/=src;
    return *this;
  }

  template<typename SourceType>
  MyType & operator%=(const SourceType & src) {
    _out%=src;
    return *this;
  }

  template<typename SourceType>
  MyType & operator<<=(const SourceType & src) {
    _out<<=src;
    return *this;
  }

  template<typename SourceType>
  MyType & operator>>=(const SourceType & src) {
    _out>>=src;
    return *this;
  }

  template<typename SourceType>
  MyType & operator|=(const SourceType & src) {
    _out|=src;
    return *this;
  }
 
  template<typename SourceType>
  MyType & operator^=(const SourceType & src) {
    _out^=src;
    return *this;
  }

  template<typename SourceType>
  MyType & operator&=(const SourceType & src) {
    _out&=src;
    return *this;
  }
  
  MyType & operator++() {
    return ++_out;
    return *this;
  }

  MyType operator++(int) {
    _out++;
    return *this;
  }

  MyType & operator--() {
    return --_out;
    return *this;
  }

  MyType operator--(int) {
    _out--;
    return *this;
  }

};



/**
 * A directed network implementation. Saves weights in both direction
 * in both nodes. This allows easy iteration over either incoming,
 * outgoing or all edges.
 *
 * Modified by Lauri Kovanen from Jörkki's SymmNet, 8/2008.
 */

template<typename _EdgeData,
	 template<typename, typename, typename, typename, typename
		  > class EdgeTable=ValueTable,
	 template<typename, typename, typename, typename, typename
		  > class NodeTable=ValueTable,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table > class EdgeIndex=LinearHash,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table> class NodeIndex=Vector>
class DirNet:public Map<size_t, 
			 AutoMap<size_t, WeightPair<_EdgeData>, EdgeIndex, EdgeTable, 
				 NetEdgePolicy<WeightPair<_EdgeData> >, NetEdgeParams>, 
			 NodeIndex, NodeTable> {
private:

  // The network size as reported to the user. Corresponds to the
  // largest explicitely set net size.
  size_t virtual_size;
  
public:
  typedef _EdgeData EdgeData;
  typedef WeightPair<EdgeData> EdgePair;
  typedef AutoMap<size_t, EdgePair, EdgeIndex, EdgeTable, 
		  NetEdgePolicy<EdgePair>, NetEdgeParams> EdgeMap;
  typedef Map<size_t, EdgeMap, NodeIndex, NodeTable>  NodeMap;
  typedef DirNet<EdgeData, EdgeTable, NodeTable, 
		  EdgeIndex, NodeIndex> MyType;
  typedef NodeMap super;

  DirNet(size_t capacity=0): super(capacity), virtual_size(capacity) {}

  static const EdgeMap emptyEdgeMap;
  static const EdgePair emptyEdgePair;

protected:
  /** Private methods helping the iterators, stubs etc.  */
  void int_Set(const size_t source,const size_t dest, const EdgePair & value) {
    assert(source != dest);
    //std::cerr << "To int set";
    /* The following works regardless of whether we use stubs or not. */
    /* The out-edge from source to dest has been changed.
       Swap in-out values for the other node.*/
    (&(super::operator[](source)))->setValue(dest,EdgePair(value.in(),value.out()));
    //std::cerr << "Int set!";
  }
  
  EdgeMap & int_Edges(const size_t source) {
    return super::operator[](source);
  }

  bool edgesLegal(const size_t source, const size_t dest) const {
    if (!operator()(source).keyLegal(dest)) {
      std::cerr << "source-dest illegal";
      return false;
    } 
    if (!operator()(dest).keyLegal(source)) {
      std::cerr << "dest-source illegal";
      return false;
    }
    /* 
    if (operator()(source)[dest]!=operator()(dest)[source]) { 
      std::cerr << "Asymm:";
      std::cerr << operator[](source)[dest] << "," 
      << operator()(dest)[source];
      return false;
    }
    */
    return true;
  }    

  bool localLegal(const size_t source, const size_t dest) const {
    if (!(this->keyLegal(source))) {
      std::cerr << "Source not legal\n";
      return false;
    }
    if (!(this->keyLegal(dest))) {
      std::cerr << "Dest not legal\n";
      return false;
    } 
    return edgesLegal(source, dest);
  }

public:

  /**
   * Returns the virtual size of the network. To access the size of
   * the underlying data structure, use super::size().
   */
  size_t size() const {
    return virtual_size;
  }

  /**
   * Explicit resize of the network. Also changes the virtual size to
   * the new size, since this is what the user knows.
   */
  void resize(const size_t newSize) {
    this->virtualSize = newSize;
    super::resize(newSize);
  }	       

private:

  /**
   * Returns the true size of the underlying structure, not the
   * virtual size shown to the user.
   */
  size_t structure_size() const {
    return super::size();
  }

  /** 
   * Increase size to max{2*old_size, i+1}. Called only when the new
   * index does not fit in, and therefore we always have 
   * i+1 > this->virtual_size.
   */
  size_t increase_size(const size_t i) {
    const size_t newSize = (2*super::size() > i ? 2*super::size() : i+1);
    super::resize(newSize);
    //std::cerr << "Resized to " << newSize << ", virtual size: " << this->virtual_size << "\n";
    return newSize;
  }

public:

  const MyType & c() const {
    return *(const_cast<const MyType *>(this));
  }

  const EdgeMap & operator()(const size_t i) const {
    // Returns an empty EdgeMap if i does not exist in the net.
    if (i >= super::size()) return MyType::emptyEdgeMap;
    return super::operator[](i);
  }

  const EdgePair & operator()(const size_t i, const size_t j) const {
    // Returns an empty EdgePair if i or j does not exist in the net.
    if (i >= super::size() || j >= super::size()) return MyType::emptyEdgePair;
    return super::operator[](i)[j];
  }

  class Node;
 
  class Edge:public EdgeMap::value_stub {
    friend class Node;
    Node & node;
    const size_t dest;
  protected:
    Edge(Node & source, EdgeMap & myRef, const size_t dst):
      EdgeMap::value_stub(myRef, dst), node(source), dest(dst) {
      //std::cerr << "Edge stub init";
      assert(source.myIndex!=dst);
    }
  public:

    ~Edge() {
      //std::cerr << "Deleting edge, value:" << (*this) << "\n"; //EdgeMap::value_stub::ref() << "\n";
      /* We write tha data on the second end of the edge...*/
      
      node.getNet().int_Set(dest,node.myIndex,*this);
      //std::cerr << "Other end set, value:" 
      //<< node.getConstNet()[dest][node.myIndex] 
      //<< "\n";
      /* ...using the superclass's methods: we do not want 
       * an eternal loop. */ 
      EdgeMap::value_stub::_lce_finalize();

      /* The node from which this was called should be finalized OK. */
      /* Cannot assert fully here, as teh nodes's stub is yet 
       * to be finalized. */
      assert(node.getConstNet().edgesLegal(node.myIndex, dest));
      //std::cerr << "Edge " << node.myIndex << "->" << dest << " destroyed.\n";
    }

    /* Set edge weights in both directions. */
    Edge & operator=(const EdgePair & src) {
      EdgeMap::value_stub::operator=(src);
      return (*this);
    }

    /* Set only outgoing edge weight. */
    Edge & operator=(const EdgeData & src) {
      EdgeMap::value_stub::ref().out() = src;
      return (*this);
    }

    /* Implicit conversion to EdgeData gives the outgoing edge.  This
     * is used (implicitely) for instance when streaming edges, and
     * also allows using edges in calculations with variables of type EdgeData.
     * Again, this adds to the transparency of WeightPair (only
     * the edge i->j is visible upon a call to net[i][j]).
     **/
    operator EdgeData() {
      //std::cerr << "Converting ...\n";
      return EdgeMap::value_stub::ref().out();
    }

  };
  
  friend class Edge;

  /* The EdgeIter does not update the container 
   * until destructed. container on the fly. Easy to change 
   * the default behaviour. */

  class EdgeIter:public EdgeMap::iterator {
  private:
    friend class Node;
    Node & node;
  protected:
    EdgeIter(Node & source): 
      EdgeMap::iterator(&(source.ref())),
      node(source) {
      assert(this->finished() || 
	     node.getConstNet().edgesLegal(**this, node.myIndex));
    }

  public:    
    typedef typename EdgeMap::iterator::reference reference;
    EdgeIter & operator++() {
      //std::cerr << "Setting the edge from:" << (**this) << " to:"; 
      //std::cerr << node.myIndex << " to value:";
      //std::cerr << this->constValue() << "\n";
      node.getNet().int_Set(**this, node.myIndex, this->constValue());
      assert(node.getConstNet()[**this].isLegal());
      node._lce_update();
      //assert(node.getConstNet()[**this][node.myIndex]
      //	     ==node.getConstNet()[node.myIndex][**this]);
      EdgeMap::iterator::operator++();
      assert(this->finished() || 
	     node.getConstNet().localLegal(**this, node.myIndex)); 
      /* this would update the container: */
      /* node._lce_update();*/
      return *this;
    }
  };

  class Node:public NodeMap::value_stub {
    friend class 
    DirNet<EdgeData, EdgeTable, NodeTable, EdgeIndex, NodeIndex>;
    typedef typename NodeMap::value_stub SuperStub;
    friend class Edge;
    friend class EdgeIter;
    friend class NodeIter;
    const size_t myIndex;
  private:
    MyType & getNet() {return (MyType &) SuperStub::target;}
    const MyType & getConstNet() {return getNet().c();}
  protected:
    Node(MyType & net, size_t index): NodeMap::value_stub(net, index),
				      myIndex(index) {
      //std::cerr << "Node stub inited";
    }
  public:
    Edge operator[](size_t i) {
      // Resize if necessary.
      if ( i >= getNet().size() ) {
	if (i >= getNet().structure_size()) getNet().increase_size(i);
	getNet().virtual_size = i+1;
      }
      //assert(getConstNet().localLegal(myIndex, i));
      /* This would be the place to prefetch. */
      //std::cerr << "Gettin' edge\n";
      return Edge(*this, SuperStub::ref(), i);
    }
    ~Node() {
      //std::cerr << "Finalizing node stub";
      NodeMap::value_stub::_lce_finalize();
      assert(getConstNet().keyLegal(myIndex));
    }

    EdgeIter begin() {
      return EdgeIter(*this);
    } 

    template<typename SourceType>
    Node & operator=(const SourceType & src) {
      for (typename SourceType::const_iterator i=src.begin(); 
	   !i.finished(); 
	   ++i) {
	this->operator[](*i)=i.value();
      }
      return *this;
    }
  };
  
  friend class Node;

  class NodeIter:public NodeMap::iterator {
  protected:
    friend class 
    DirNet<EdgeData, EdgeTable, NodeTable, EdgeIndex, NodeIndex>;
    typedef typename NodeMap::iterator SuperIter;
    NodeIter(MyType * target):NodeMap::iterator(target)  {}
  public:
    Node value() {
      return Node(*((MyType *) SuperIter::target), *(*this));
    }
    /* All else OK. */
  };

  typedef NodeIter iterator;
  typedef typename super::const_iterator const_iterator;
  typedef EdgeIter edge_iterator;

  iterator begin() {return NodeIter(this);}
  
  typedef typename EdgeMap::const_iterator const_edge_iterator;

  Node operator[](const size_t i) {
    // Resize net if necessary, initializing with empty values.
    if ( i >= size() ) {
      if (i >= structure_size()) increase_size(i);
      this->virtual_size = i+1;
    }
    assert(this->keyLegal(i));
    return Node(*this, i);
  }

  typename NodeMap::const_reference operator[](const size_t i) const {
    //assert(this->keyLegal(i));
    return NodeMap::operator[](i);
  }

  bool isLegal() const {
    if (!super::isLegal()) {return false;}
    for (const_iterator i=super::begin(); !i.finished(); ++i) {
      if (!i.value().isLegal()) return false;
      /* 
      for (const_edge_iterator j=i.value().begin();
	   !j.finished(); ++j) {
	if (j.value() != (*this)[*j][*i]) {
	  std::cerr << "Bummer!\n";
	  return false;
	}
      }
      */
    }
    return true;
  }

};


// Definitions for the static template variables. Yes, they really
// need to be that complicated.
template<typename _EdgeData,
	 template<typename, typename, typename, typename, typename
		  > class EdgeTable,
	 template<typename, typename, typename, typename, typename
		  > class NodeTable,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table > class EdgeIndex,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table> class NodeIndex> 
const AutoMap<size_t, WeightPair<_EdgeData>, EdgeIndex, EdgeTable, 
				 NetEdgePolicy<WeightPair<_EdgeData> >, NetEdgeParams>
DirNet<_EdgeData, EdgeTable, NodeTable, EdgeIndex, NodeIndex>::emptyEdgeMap;

template<typename _EdgeData,
	 template<typename, typename, typename, typename, typename
		  > class EdgeTable,
	 template<typename, typename, typename, typename, typename
		  > class NodeTable,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table > class EdgeIndex,
	 template<typename, typename, typename, typename, 
		  template<typename, typename, typename, typename, typename
			   > class Table> class NodeIndex>
const WeightPair<_EdgeData>
DirNet<_EdgeData, EdgeTable, NodeTable, EdgeIndex, NodeIndex>::emptyEdgePair;

#endif
