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

#include<cassert>

/**
 * Template parameters:
 *
 * IndexType   is the integral type of node indices
 * packTree    Whether to link the nodes found on the 
 *             path to the root node of a cluster tree
 *             directly to the root. Breaks the shape of the tree 
 *             AND the node counts of subtrees, but speeds up the 
 *             futher operations considerably
 */

template<bool packTree=true>
class KruskalTree {
private:
  size_t * fathers;
  size_t * clusterSizes; /* Why to use the same type here? 
			     * Go figure. */
  KruskalTree() {}; /* Not allowed to be called: 
		     * only to be initialized to a specified size */ 
  
  size_t size;

public:

  /* This should be self-documentary */

  KruskalTree(size_t networkSize) {
    size_t i;
    size=networkSize;
    fathers=new size_t[size];
    clusterSizes=new size_t[size];

    for (i=0; i<size; i++) {
      fathers[i]=i; /* Every node is it's own cluster */
      clusterSizes[i]=1;
    }
  }

  ~KruskalTree() {
    delete[] fathers;
    delete[] clusterSizes;
  }

  /* The basic, recursive method for traversing a tree toward the 
   * root. The packing operation becomes very elegant with the 
   * recursive implementation. If packing is not to be used,
   * the compiler should be able to eliminate the recursion 
   * as a tail recursion. 
   *
   * Params: 
   * 
   * nodeIndex   the node...
   * returns     ...and where it belongs to. */

  size_t getClusterID(const size_t nodeIndex) const {
    assert((nodeIndex >= 0) && (nodeIndex < size));
    if (fathers[nodeIndex] == nodeIndex) { /* The root of a subtree */
      return nodeIndex;
    } else { /* Recurse up the tree */
      size_t theRoot=getClusterID(fathers[nodeIndex]);
      if (packTree) { /* The compiler eliminates this clause based on
		       * the template parameter. */
	fathers[nodeIndex]=theRoot; /* A direct link to the root */
      }
      return theRoot;
    }
  }

  /* Adds an edge to the network, joining clusters if necessary. 
   *
   * If clusters (trees) are joined, the one corresponding to the
   * first parameter becomes the "root", that is, the cluster ID
   * of that will be the cluster ID of the resulting cluster.
   * Feel free to make use of the feature. 
   *
   * Is the above order sensible? Dunno, we'll find out when 
   * using this with directed graphs.
   *
   * Params: 
   *
   * from, to    The nodes associated with the edge 
   * returns     Whether a new cluster was formed */

  bool addEdge(const size_t from, const size_t to) {
    assert((from >= 0) && (from < size));
    assert((to >= 0) && (to < size));
    
    size_t fromCluster=getClusterID(from);
    size_t toCluster=getClusterID(to);
    
    if (fromCluster==toCluster) {
      return false;
    } else {
      fathers[toCluster]=fromCluster; /* The linking op */
      clusterSizes[fromCluster]+=clusterSizes[toCluster];
      return true;
    }

  }

  /**
   * When called with a proper cluster index, returns the size of the 
   * cluster. This might be slightly faster than using the next method.
   * Otherwise, returns the size of the sub-cluster, which DOES NOT
   * make sense in the case of a packed tree.
   *
   * Params:
   *
   * clusterIndex   The subcluster (root node at the root of the subtree)...
   * returns        ...and it's size
   */
  /*
  size_t getSubClusterSize(const size_t clusterIndex) const {
    assert((clusterIndex >= 0) && (clusterIndex < size));
    return sizes[nodeIndex];
  }
  */

  /**
   * Returns the size of the cluster the nodei in question belongs to
   * 
   * nodeIndex   the node...
   * returns     ...and the size of the cluster it belongs to.
   */

  size_t getClusterSize(const size_t nodeIndex) const {
    /* Assertion included below */
    return clusterSizes[getClusterID(nodeIndex)];
  }
};  

template<bool packTree=true>
class BiDirKruskalTree: public KruskalTree<packTree> {
public:
  typedef KruskalTree<packTree> super;
  typedef BiDirKruskalTree<packTree> MyType;
private:
  bool * InTheNet;
  BiDirKruskalTree() {}
public:
  BiDirKruskalTree(size_t size): super(size) {
    InTheNet=new bool[size];
  }

  bool addEdge(const size_t from, const size_t to) { 
    InTheNet[from]=true;
    if (InTheNet[to]) return super::addEdge(from,to);
    return false;
  }

  ~BiDirKruskalTree() {
    delete[] InTheNet;
  }
};
  


