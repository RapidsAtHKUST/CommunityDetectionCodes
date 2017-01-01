/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_heap_h
#define tingea_heap_h

/*    Use a heap to compute the K largest elements according to some sortin
 *    criterion.
 *    Note that a new element only has to be compared with the root (when
 *    the heap is at full capacity) to know whether it should enter the heap
 *    and evict the root.
*/

typedef struct
{  void     *base
;  dim      heapSize
;  dim      elemSize
;  int      (*cmp)(const void* lft, const void* rgt)
;  dim      n_inserted
;
}  mcxHeap  ;


mcxHeap* mcxHeapInit
(  void*    heap
)  ;


mcxHeap* mcxHeapNew
(  mcxHeap* heap
,  dim      heapSize
,  dim      elemSize
,  int      (*cmp)(const void* lft, const void* rgt)
)  ;


void mcxHeapFree
(  mcxHeap**   heap
)  ;

void mcxHeapRelease
(  void* heapv
)  ;  

void mcxHeapClean
(  mcxHeap*   heap
)  ;  

void mcxHeapInsert
(  mcxHeap* heap
,  void*    elem
)  ;


#endif

