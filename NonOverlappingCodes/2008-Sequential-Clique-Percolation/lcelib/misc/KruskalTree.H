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

template<typename IndexType=unsigned, bool packTree=true>
class KruskalTree {
private:
  IndexType * fathers;
  IndexType * clusterSizes; /* Why to use the same type here? 
			     * Go figure. */
  KruskalTree() {}; /* Not allowed to be called: 
		     * only to be initialized to a specified size */ 
  
  IndexType size;

public:

  /* This should be self-documentary */

  KruskalTree(IndexType networkSize) {
    IndexType i;
    size=networkSize;
    fathers=new IndexType[size];
    clusterSizes=new IndexType[size];

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

  IndexType getClusterID(const IndexType nodeIndex) const {
    assert((nodeIndex >= 0) && (nodeIndex < size));
    if (fathers[nodeIndex] == nodeIndex) { /* The root of a subtree */
      return nodeIndex;
    } else { /* Recurse up the tree */
      IndexType theRoot=getClusterID(fathers[nodeIndex]);
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

  bool addEdge(const IndexType from, const IndexType to) {
    assert((from >= 0) && (from < size));
    assert((to >= 0) && (to < size));
    
    IndexType fromCluster=getClusterID(from);
    IndexType toCluster=getClusterID(to);
    
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

  //  IndexType getSubClusterSize(const IndexType clusterIndex) const {
  //  assert((clusterIndex >= 0) && (clusterIndex < size));
  //  return sizes[nodeIndex];
  // }
  // kommentoin taman funktion pois koska siina on jotain kummaa 
  // ja se estaa koko failin kaantamisen  - 6 Apr 2006 Riitta


  /**
   * Returns the size of the cluster the nodei in question belongs to
   * 
   * nodeIndex   the node...
   * returns     ...and the size of the cluster it belongs to.
   */

  IndexType getClusterSize(const IndexType nodeIndex) const {
    /* Assertion included below */
    return clusterSizes[getClusterID(nodeIndex)];
  }
};

 


