/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/


#include <stdio.h>
#include <string.h>

#include "alloc.h"
#include "heap.h"
#include "types.h"
#include "err.h"


mcxHeap* mcxHeapInit
(  void* h
)
   {  mcxHeap* heap  =     h

   ;  if (!heap && !(heap = mcxAlloc(sizeof(mcxHeap), RETURN_ON_FAIL)))
      return NULL

   ;  heap->base     =     NULL
   ;  heap->heapSize =     0
   ;  heap->elemSize =     0
   ;  heap->cmp      =     NULL
   ;  heap->n_inserted =   0
   ;  return heap
;  }


mcxHeap* mcxHeapNew
(  mcxHeap*    h
,  dim         heapSize
,  dim         elemSize
,  int (*cmp)  (const void* lft, const void* rgt)
)
   {  mcxHeap* heap     =  mcxHeapInit(h)
   ;  mcxstatus status  =  STATUS_FAIL
   ;  char*    base

   ;  do
      {  if (!heap)
         break
      ;  if (!(heap->base = mcxAlloc (heapSize*elemSize, RETURN_ON_FAIL)))
         break
      ;  status = STATUS_OK
   ;  }
      while (0)

   ;  if (status)
      {  mcxHeapFree(&heap)
      ;  return NULL
   ;  }
      heap->heapSize    =  heapSize
   ;  heap->elemSize    =  elemSize
   ;  heap->cmp         =  cmp
   ;  heap->n_inserted  =  0
   ;  base              =  (char*) heap->base
   ;  return heap
;  }


void mcxHeapClean
(  mcxHeap*   heap
)  
   {  heap->n_inserted = 0
;  }


void mcxHeapRelease
(  void* heapv
)  
   {  mcxHeap* heap = (mcxHeap*) heapv

   ;  if (heap->base)
      mcxFree(heap->base)
   ;  heap->base     = NULL
   ;  heap->heapSize = 0
;  }


void mcxHeapFree
(  mcxHeap**   heap
)  
   {  if (*heap)
      {  if ((*heap)->base)
         mcxFree((*heap)->base)
         
      ;  mcxFree(*heap)
      ;  *heap       =  NULL
   ;  }
   }



void mcxHeapInsert
(  mcxHeap* heap
,  void*    elem
)  
   {  char* heapRoot =  heap->base
   ;  char* elemch   =  elem
   ;  dim   elsz     =  heap->elemSize
   ;  dim   hpsz     =  heap->heapSize

   ;  int (*cmp)(const void *, const void*) =  heap->cmp

   ;  if (heap->n_inserted  < hpsz)
      {  dim i =  heap->n_inserted

      ;  while (i != 0 && (cmp)(heapRoot+elsz*((i-1)/2), elemch) < 0)
         {  memcpy(heapRoot + i*elsz, heapRoot + elsz*((i-1)/2), elsz)
         ;  i = (i-1)/2
      ;  }
         memcpy(heapRoot + i*elsz, elemch, elsz)
      ;  heap->n_inserted++
   ;  }
      else if ((cmp)(elemch, heapRoot) < 0)
      {  dim root = 0
      ;  dim d

      ;  while ((d = 2*root+1) < hpsz)
         {  if
            (  (d+1<hpsz)
            && (cmp)(heapRoot + d*elsz, heapRoot + (d+1)*elsz) < 0
            )
            d++

         ;  if ((cmp)(elemch, heapRoot + d*elsz) < 0)
            {  memcpy(heapRoot+root*elsz, heapRoot+d*elsz, elsz)
            ;  root = d
         ;  }
            else
            break
      ;  }
         memcpy(heapRoot+root*elsz, elemch, elsz)
   ;  }
   }


