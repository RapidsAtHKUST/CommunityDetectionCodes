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

#include <cassert>
#include "../bits/FiboHeapCore.H"
#ifndef FIBO_HEAP
#define FIBO_HEAP

/* This trick probably should be generalized. */

struct EmptyType {};

template <typename T>
struct VoidMask {
  typedef T Type;
};

template <>
struct VoidMask<void> {
  typedef EmptyType Type;
};


template <typename KeyType, typename ValueType=void>
class FiboHeap {
public:
  typedef FiboHeap<KeyType, ValueType> MyType;
  typedef FiboHeapNode<KeyType, ValueType> NodeType;
  typedef typename VoidMask<ValueType>::Type ValType;
  typedef ValType & ValRefType;
  typedef const ValType & constValRefType;
private:
  FiboHeapCore<NodeType> heap;
  KeyType lastPopped;
public:
  /* Pushes a key-value pair to the heap. Returns a pointer 
   * to the node so generated, which can be used for decreasing key, 
   * deletion, setting values again etc. */
  NodeType * push(const KeyType key) {
    NodeType * retval=new NodeType(key);
    heap.push(retval);
    return retval;
  }
  
  NodeType * push(const KeyType key, constValRefType value) {
    NodeType * retval=push(key);
    retval->value()=value;
    return retval;
  }

  MyType & operator++() {
    assert(heap.getRoot()); /* !=0 */
    lastPopped=heap.getRoot()->getKey();
    delete heap.pop();
    return *this;
  }

  KeyType operator*() const {
    assert(heap.getRoot());
    return heap.getRoot()->getKey();
  }

  ValRefType value() {
    assert(heap.getRoot());
    return heap.getRoot()->value();
  }

  bool finished() {return heap.isEmpty();}
  
  void decreaseKey(NodeType * subject, KeyType newKey) {
    subject->decreaseKey(newKey, heap);
  } 

  void setKey(NodeType * subject, KeyType newKey) {
    subject->setKey(newKey, heap);
  }

  void deleteNode(NodeType * subject) {
    heap.deleteNode(subject);
  }
    
  unsigned size() {
    return heap.getNumElems();
  }

  bool isValid(bool justPopped=true) {
    return heap.rootListValid(true, justPopped); /* Check it all, 
						  * check degree list only 
						  * if just popped. */
  }
};

#endif




