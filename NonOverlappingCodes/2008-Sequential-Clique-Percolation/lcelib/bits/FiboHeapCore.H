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

//#define NDEBUG
#include <cassert>
#ifndef NDEBUG
#include <iostream>
#endif
#ifndef FIBO_HEAP_CORE
#define FIBO_HEAP_CORE

/**
 * The core templates for a Fibonacci heap as suggested by Tarjan et al. 
 * [*]. Supports all the usual operations for single heap. Heap merging
 * is not yet supported, albeit trivieal to implement. Will be, when needed.
 */

/* Traits for heap ordering. Either overload the < -operator or 
 * specialize this. You could not ask for more.  */

template <typename KeyType>
struct HeapKeyTraits {
  static bool precedes(KeyType first, KeyType second) {return (first<second);}
};

template <typename NodeType>
class FiboHeapCore {

private:
  /* Compiler wants to know that such exists: */
  NodeType ** degreeList; /* Array of pointers */
  unsigned degreeListSize;
  unsigned numElems;
  NodeType * root;

  /** 
   * Removes a node from a place. "Deep" means that the subject 
   * is not in the root list. Children remain attached. 
   */

  void removeDeep(NodeType * subject) {
    assert(subject->father); 
    if (subject->rightSister==subject) {
      assert (subject->leftSister==subject);
      subject->father->child=0;
    } else {
      subject->father->child=subject->rightSister;
      subject->rightSister->leftSister=subject->leftSister;
      subject->leftSister->rightSister=subject->rightSister;
    }
  } 

  /**
   * Adds a node to the root list. Children remain attached and 
   * the new minimum is not checked for. 
   */

  void addToRootList(NodeType * subject) {
    subject->rightSister=root->rightSister;
    subject->leftSister=root;
    root->rightSister->leftSister=subject;
    root->rightSister=subject;
    subject->father=0;
    degreeList[subject->degree]=subject;
  }
  
  /* Chain is run on the fathers of nodes cut. */

  void promoteChain(NodeType * subject) {
    assert(containsNode(subject)); /* Fathers automagically contained. */
    --subject->degree; /* Just lost a child. Why to decrease the degree? 
			* I don't know, but appears in the pseudocode. */
    NodeType * oldFather;
    while (subject->father) { 
      assert(!root->siblingTo(subject));
      if (!(subject->mark)) {
	subject->mark=true;
	return;
      }
      /* Not at root list. marked.
       * Remove from here... */
      oldFather=subject->father;
      removeDeep(subject);
      /* Add to root list. Marks the father as null. */
      addToRootList(subject); 
      subject=oldFather;
      --(subject->degree);
    }
    /* At the root list. Update it, the node has just lost a child. */
    degreeList[subject->degree]=subject;
    degreeList[(subject->degree)+1]=0;
  }

  /* The standard promote operation of fibonacci heap. 
   * Could also be a method of the nodes themselves, but this way,
   * we do not have to traverse the father links up to the root list. */

  void promoteNode(NodeType * subject) {
    assert (subject); /* Do not feed me nulls, please */
    assert (subject->father);
    NodeType * oldFather=subject->father;
    assert (containsNode(subject));
    assert(containsNode(oldFather));
    /* Only promote, if there is some sense in it. */
    removeDeep(subject);
    addToRootList(subject);
    if ((*subject) < (*root)) root=subject;
    assert(rootListValid(true, false));
    promoteChain(oldFather);
  }
  
  void compress() {
    NodeType * tempnode;
    NodeType * changed;
    NodeType ** tempnodes;

    tempnode=root; /* Main iterator */
    changed=root;  /* The last node manipualted during iteration,
		    * used for termination checking. Used also 
		    * when joining trees. */ 
    
    do { /* Loop joins all the nodes in the root list with the 
	  * same degree together. Heap order is maintained. 
	  * The loop is repeated until there is at most one 
	  * node with any given degree in the root list. 
	  * We know this when we go around the complete list without 
	  * finding anything to update. */
      /* When compressing after a pop op, we have several nodes with 
       * non-valid father links around. No reason to check this,
       * just mark: */
      tempnode->father=0;
      /* We have to find the minimum, also. 
       * This is the right place for that, as we necessarily go through
       * the ready degree list at least once: */
      if ((*tempnode) < (*root)) root=tempnode;
      
      assert(tempnode->localLegal());

      /* The loop below joins the tree rooted at tempnode together 
       * with other trees with the same degree. The check is repeated 
       * for the newly formed tree with increased degree until no joins have
       * to be made anymore. */
      while ((degreeList[tempnode->degree] != tempnode) &&
	     (degreeList[tempnode->degree] != 0)) {
	/* In case of the other condition, we will fix it next after
	 * bailing out. In the case of the first, we have to join 
	 * two trees. We temporally use the pointer "changed" to denote 
	 * the element leaving the root list. Tempnode will be the one
	 * remaining there. But first, let us assert: */
	assert(degreeList[tempnode->degree]->localLegal());

	if ((*tempnode) < *(degreeList[tempnode->degree])) {
	  changed=degreeList[tempnode->degree];
	} else {
	  changed=tempnode;
	  tempnode=degreeList[tempnode->degree];
	  /* We do not necessarily see this node again, so */
	  tempnode->father=0;
	  /* This bug took me almosta a couple of days to fix.
	   * It just might be so that tempnode and the root
	   * were exactly equal. In that case, we would 
	   * move the root up. So,  */
	  if (!((*root) < (*tempnode))) root=tempnode;
	}
	/* We might as well do this here: */
	changed->mark=false;
	/* In order not to forget...*/
	degreeList[tempnode->degree]=0;
	/* Then, let us remove the "changed" from the root list: */
	changed->leftSister->rightSister=changed->rightSister;
	changed->rightSister->leftSister=changed->leftSister;
	/*... and add it to the child list of the tempnode. */
	if (tempnode->child==0) {
	  tempnode->child=changed;
	  changed->rightSister=changed;
	  changed->leftSister=changed;
	} else {
	  changed->rightSister=tempnode->child->rightSister;
	  changed->leftSister=tempnode->child;
	  tempnode->child->rightSister->leftSister=changed;
	  tempnode->child->rightSister=changed;
	}
	changed->father=tempnode;
	tempnode->degree++;
	changed=tempnode; /* The last one changed during the loop... */


	/* We scale up the degree list. "vector" is so slow that
	 * we do this by hand. The factor 2 is hard-coded below
	 * (warning: in several places). No reason to change,
	 * as the size of the degree list is logarithmic to
	 * the data size. */
	if (tempnode->degree >= degreeListSize) {
	  tempnodes=new NodeType * [degreeListSize*2];
	  for (unsigned i=0; i< degreeListSize; i++) {
	    tempnodes[i+degreeListSize]=0;
	    tempnodes[i]=degreeList[i];
	  }
	  delete[] degreeList;
	  degreeList=tempnodes;
	  degreeListSize*=2;
	}
	assert (tempnode->localLegal());
      }
     
      /* Out of the recursive join loop. Now, either 
       * we did not run the join loop even once, or the place
       * for tempnode is marked null in the degree list. 
       * Why bother to check, as we can just: */
      degreeList[tempnode->degree]=tempnode;
      tempnode=tempnode->rightSister;
    } while (tempnode != changed);
  }
  
  
  /** 
   * Pops a single node from the root list
   */

  void popNode(NodeType * node) {
    assert(root->siblingTo(node));
    assert(rootListValid(false,  /* Do not check the whole tree */ 
			 false)); /* Do not assume to be just compressed */
       
    --numElems;
    
    if (node==node->leftSister) { /* The only element in the root list */
      assert (node==node->rightSister);
      root=node->child;
    } else {
      if (node->child != 0) {
	/* Add the descendants of the former root node to the root list */
	root=node->child;
	node->leftSister->rightSister=root;
	node->rightSister->leftSister=root->leftSister;
	root->leftSister->rightSister=node->rightSister;
	root->leftSister=node->leftSister;
	root->father=0;
      } else { /* no children, just remove  */
	node->leftSister->rightSister=node->rightSister;
	node->rightSister->leftSister=node->leftSister;
	root=node->rightSister;
      }
    }
    degreeList[node->degree]=0;
    assert(rootListValid(false,  /* FIXME:Do not check the whole tree */ 
			 false,  /* Do not assume to be just compressed */
			 true)); /* Do not check for the right root node
				  * nor father-links being null */

    if (root!=0) compress();

    assert (rootListValid()); 
    //assert ((!root) || (!((*root) < (*retval))));
  }


public:
  FiboHeapCore() {
    numElems=0;
    degreeListSize=10;
    degreeList=new NodeType * [degreeListSize];
    for (unsigned i=0; i<degreeListSize; i++) degreeList[i]=0;
    root=0; /* null, but this is the C Doublecross (tm) -style */
  }

  ~FiboHeapCore() {
    delete[] degreeList;
    if (root) {
      root->delNode(); /* This should recursively trigger the deletion of */
    } else {            /* the whole bastard */
      assert (numElems==0);
    }
  }

  void deleteNode(NodeType * subject) {
    assert(containsNode(subject));
    if (subject->father) {
      NodeType * oldFather=subject->father;
      removeDeep(subject);
      addToRootList(subject);
      promoteChain(oldFather);
    }
    popNode(subject); /* Decreases the number of elems. */
  }
  
  /**
   * Pushes a node to the heap, returning the number of elements in
   * it. 
   */

  unsigned push(NodeType * node) {
    /* A nice assertion below. Do not uncomment, at least
     * when doing heapsort. */
    // assert (rootListValid(false,  /* Do not check the whole tree */ 
    //		       false)); /* Do not assume to be just compressed */
    assert ((root!=0) || (numElems==0));
    assert ((root==0) || root->localLegal()); 
    assert (node!=0);
    assert ((node->father==0) && (node->leftSister==node) 
	    && (node->rightSister==node) && (node->child==0) && 
	    (node->mark==false) && (node->degree==0));
    if (root==0) {
      root=node;
    } else {
      addToRootList(node);
      if ((*node) < (*root)) {
	root=node;
      }
    }
    degreeList[0]=node;
    /* Same  goes here */
    //assert (rootListValid(false,  /* Do not check the whole tree */ 
    //		  false)); /* Do not assume to be just compressed */
    assert(root->localLegal() && node->localLegal());
    return ++numElems;
  }
  
  void justDecreased(NodeType * subject) {
    if (subject->father) {
      if (*subject < *(subject->father)) { 
	promoteNode(subject);
      }
    } else {
      if (*subject < *root) {
	root=subject;
      }
      
    }
  }

  void justIncreased(NodeType * subject) {
    deleteNode(subject);
    subject->init();
    push(subject);
  }

  unsigned getNumElems() {
    assert ((numElems==0) == (root==0));
    return numElems; 
  }

  bool isEmpty() {
    assert ((numElems==0) == (root==0));
    return (root==0);
  }
  

  NodeType * getRoot() const {return root;}

  /**
   * Pops the root node (the minimum one, or the one which no other node
   * precedes) from the heap. 
   *
   * NB. The root list is legal only after the compress op, which sets 
   * the father pointers to 0.
   */


  NodeType * pop() {
    NodeType * retval=root;
    popNode(root);
    return retval;
  }

  /**
   * Contains-op, mainly for assertions, as you should probably know
   * the result already. Follows the father-links up to the root list
   * level, and then checks whether the root of the heap is
   * in the same sibling list. Other sibling checks etc. are performed
   * if assertions enabled. the siblingTo-operation contains all the 
   * assertions needed.
   */

  bool containsNode(const NodeType * const subject) const {
    if (subject->father == 0) {
      return root->siblingTo(subject);
    } else {
      if (!(subject->siblingTo(subject->father->child))) return false;
      return (containsNode(subject->father));
    } /* Tail recursion above. A decent compiler will notice. */
  }

  /**
   * Checks the degree and root lists for validity after compressions. 
   */

  bool rootListValid(bool checkWholeTree=false, 
		     bool isCompressed=true,
		     bool justPopped=false) const { /* between pop and compr */
    NodeType * iter=root;
    //std::cerr << "[";
    bool passed=true;
    if (iter==0) {
      if (numElems!=0) return false;
    } else {
      do {
	if (!(iter->localLegal())) passed=false;;
	if (!justPopped) {
	  if (iter->father!=0) {
	    std::cerr << "Father not null in the root list for elem!:"
		      << iter->degree << "\n";
	    NodeType * secIter=root;
	    do {
	      std::cerr << secIter->degree << " ";
	      secIter=secIter->rightSister;
	    } while (secIter!=root);
	    std::cerr << "\n";
	    passed=false;;
	  }
	  if ((*iter) < (*root)) {
	    std::cerr << "Wrong root!\n";
	    passed=false;
	  }
	}
	if (isCompressed && (degreeList[iter->degree]!=iter)) {
	  std::cerr << "Wrong elem in deg list for deg:" << iter->degree 
		    << " \n" ;
	  passed=false;;
	}
	if (checkWholeTree && (!(*iter).subtreeValid())) {
	  std::cerr << "Subtree not valid in the root list!\n";
	  passed=false;
	}
	iter=iter->rightSister;
      } while (iter!=root);
      if (checkWholeTree) {
	unsigned count=root->countNodes();
	if (count != numElems) {
	  std::cerr << "Wrong count! Is:" << count << ", heap counter:" 
		    << numElems << "\n";
	  passed=false;
	}
      }
    } /* Empty? */
 
    if (isCompressed) {
      /* The reverse check is run for an empty heap, also */    
      for (unsigned i=0; i<degreeListSize; i++) {
	if (degreeList[i] != 0) {
	  if (degreeList[i]->degree != i) {
	    std::cerr << "Degree list invalid!\n";
	    passed=false;
	  }
	}
      }
    }
    //std::cerr << "]";
    return passed;
  }  
};

template<typename ValueType>
struct NodeDataContainer {
  ValueType val;
  typedef ValueType & ValueRefType;
  ValueRefType value() {return val;}
};
  
template<>
class NodeDataContainer<void> {
public:
  typedef void ValueRefType;

  ValueRefType value() const {}
};



template <typename KeyType, typename ValueType>
class FiboHeapNode:public NodeDataContainer<ValueType> {
  typedef FiboHeapNode<KeyType, ValueType> MyType;
  friend class FiboHeapCore<MyType>;  
  typedef HeapKeyTraits<KeyType> MyKeyTraits;
public:
  typedef typename NodeDataContainer<ValueType>::ValueRefType ValueRefType;
protected:
  /* Full sister linking: */
  MyType * leftSister;
  MyType * rightSister;
  MyType * father;
  MyType * child;
  unsigned degree;
  bool mark;

  // protected:
  KeyType key; /* Protected, for subclasses */
  
  /**
   * Iteratively deletes a right-linked (or doubly linked) list
   * of nodes, and recursively a tree of nodes. See below for rationale.
   */

  void delNode() {
    MyType * iter;
    MyType * temp;   
    iter=this;
   
    //std::cerr << "D";

    do {
      if (iter->child != 0) {
	iter->child->delNode();
      }
      temp=iter;
      iter=iter->rightSister;
      delete temp;
    } while ((iter!=0) && (iter != this));
  }
  
  void init() {
    father=0;
    child=0;
    degree=0;
    rightSister=this;
    leftSister=this;
    mark=false;
  }

public: 

  FiboHeapNode(KeyType theKey=0) {
    init();
    key=theKey;
  }



  /* This is crucial: the standard destructor does not really 
   * delete anything linked in the node. This is left for the 
   * heap to do. For all other purposes, the delNode- method should
   * be handy. 
   * RATIONALE: We do want to destruct individual nodes, also, without
   * explicitly setting all pointers to null. Recursive or chained
   * deletions should only be needed for nodes either in a tree or
   * maybe cache lists. 
   */

  bool operator<(const MyType & rightOperand) const {
    return MyKeyTraits::precedes(key, rightOperand.key);
  }

  KeyType getKey() const {return key;}

  /**
   * The famous decrease key -operation for Tarjan's fibonacci heap. 
   * 
   * The reason for implementing it here in this form is that this way,
   * the heap itself does not have to know anything about keys: it
   * just uses the < -operator. On the other hand, the heap is included
   * as a parameter in order to facilitate fast merges to the root list.
   */

  void decreaseKey(const KeyType newKey, FiboHeapCore<MyType> & heap) {
    assert (heap.containsNode(this));
    /* We might as well allow decreasing a key to itself: */
    assert (!(MyKeyTraits::precedes(key, newKey)));
    key=newKey;
    heap.justDecreased(this);
  }

  void setKey(const KeyType newKey, FiboHeapCore<MyType> & heap) {
    assert (heap.containsNode(this));
    if (MyKeyTraits::precedes(key, newKey)) {
      key=newKey;
      heap.justIncreased(this);
    } else {
      key=newKey;
      heap.justDecreased(this);
    }
  } 
  
  bool localLegal() const {
    if (!((leftSister->rightSister==this) &&
	  (rightSister->leftSister==this))) {
      std::cerr << "Wrong sister links!\n";
      return false;
    }
    if (child!=0) { 
      if (child->father != this) {
	std::cerr << "Father-link wrong!\n";
	return false;
      }
      if (MyKeyTraits::precedes(child->key, key)) {
	std::cerr << "Heap order violated!\n";
	return false;
      }
      //  if (child->degree > degree) {
//  	std::cerr << "Recalc degrees, please!\n";
//  	//return false;
//        }
    }
    return true;
  }
  
  /* Validity for this node and its descendants */
  bool subtreeValid() {
    MyType * iter;
    unsigned largestDegFound=0;
    if (!localLegal()) return false;
    if (this->child != 0) {
      iter=this->child;
      do {
	if (iter->degree > largestDegFound) largestDegFound=iter->degree;
	if (!(iter->subtreeValid())) {
	  std::cerr << "Subtree not valid! Count these!\n";
	  return false;
	}
	if ((*iter) < (*this)) {
	  std::cerr << "Heap order violation!\n";
	  return false;
	}
	if (iter->father != this) {
	  std::cerr << "Father problem down the tree!\n";
	  return false;
	}
	iter=iter->rightSister;
      } while (iter!=this->child);
    }
    //  if (child) {
//        if (largestDegFound+1 != degree) {
//  	std::cerr << "Wrong degree down here! Largest found:"
//  		  << largestDegFound << ", mine is: " << degree << "\n";  
//        }
//      } else {
//        if (degree != 0) return false;
//      }
    return true;
  }

  /* No assertions in below. Called when the tree is not in a valid state. */

  bool siblingTo(const MyType * const subject) const {
    const MyType * iter;
    iter=this;
    while (iter!=subject) {
      iter=iter->rightSister;
      if (iter==this) return false; /* Went around without finding */
    }
    return true;
  }

  /* Counts nodes in subtree, including sister of this */

  unsigned countNodes() const {
    const MyType * iter;
    unsigned currCount=0;
    iter=this;
    do {
      currCount++;
      assert (iter->localLegal());
      if (iter->child) currCount+=iter->child->countNodes();
      iter=iter->rightSister;
    } while (iter!=this);
    return currCount;
  }
};

#endif





