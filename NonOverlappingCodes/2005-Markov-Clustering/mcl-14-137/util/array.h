/*   (C) Copyright 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_array_h
#define tingea_array_h

#include "types.h"

mcxstatus mcxSplice
(  void*           base1pp   /*  _address_ of pointer to elements       */
,  const void*     base2p    /*  pointer to elements                    */
,  dim             size      /*  size of base1 and base2 members        */
,  dim          *n_base1     /*  total length of elements after base1   */
,  dim          *N_base1     /*  number of alloc'ed elements for base1  */
,  ofs           o_base1     /*  splice relative to this ofset          */
,  dim           d_base1     /*  delete this number of elements         */
,  dim           c_base2     /*  number of elements to copy             */
)  ;


dim mcxDedup
(  void*          base     
,  dim            nmemb    
,  dim            size     
,  int            (*cmp)(const void *, const void *)
,  void           (*merge)(void *, const void *)
)  ;


mcxstatus mcxResize
(  void*          mempp
,  dim            size
,  dim*           ct
,  dim            newct
,  mcxOnFail      ON_FAIL
)  ;


/* Return largest element smaller than or equal to key.
 * return NULL if no element is smaller than key.
 * Returns rightmost element in case entries sort identically,
 * (note: mcxBsearchCeil will then return the leftmost element)
 *
 * base should be sorted according to cmp
*/

void* mcxBsearchFloor
(  const void *key
,  const void *base
,  dim nmemb
,  dim size
,  int (*cmp)(const void *, const void *)
)  ;


/* Return smallest element larger than or equal to key.
 * return NULL if no element is larger than key.
 * Returns leftmost element in case entries sort identically,
 * (note: mcxBsearchFloor will then return the rightmost element)
 *
 * base should be sorted according to cmp
*/

void* mcxBsearchCeil
(  const void *key
,  const void *base
,  dim nmemb
,  dim size
,  int (*cmp)(const void *, const void *)
)  ;


/* Uses weighted combinations of neighbours when the quartile
 * range does not fall perfectly on array offsets (i.e.
 * when the array size is not a multiple of 4.
*/
double mcxMedian
(  void* base
,  dim   n
,  dim   sz
,  double (*get)(const void*)
,  double* iqr
)  ;


/* Fisher Yates shuffle */

void mcxShuffle
(  void* datap
,  dim   nmem
,  dim   mem_size
,  char* mem_cell    /* should have mem_size size */
)  ;







typedef struct
{  void*       mempptr
;  dim         size
;  dim         n
;  dim         n_alloc
;  float       factor
;  mcxbool     bFinalized
;
}  mcxBuf      ;



/*    
 *    *mempptr should be peekable; NULL or valid memory pointer. 
*/

mcxstatus mcxBufInit
(  mcxBuf*     buf
,  void*       mempptr
,  dim         size
,  dim         n
)  ;


/*
 *    Extends the buffer by n_request unitialized chunks and returns a pointer
 *    to this space. It is the caller's responsibility to treat this space
 *    consistently. The counter buf->n is increased by n_request.
 *
 *    If we cannot extend (realloc), a NULL pointer is returned;
 *    the original space is left intact.
 *
 *    Returns NULL on (alloc) failure
*/

void* mcxBufExtend
(  mcxBuf*     buf
,  dim         n_request
,  mcxOnFail   ON_FAIL
)  ;


/*
 *    Make superfluous memory reclaimable by system,
 *    prepare for discarding buf (but not *(buf->memptr)!)
 *
 *    If for some bizarre reason we cannot shrink (realloc),
 *    errno is set to ENOMEM.
 *    the original space is left intact. Its size is in buf->n .
*/

dim mcxBufFinalize
(  mcxBuf*  buf
)  ;


/*
 *    Make buffer refer to a new variable. Size cannot be changed,
 *    so variable should be of same type as before.
*/

void mcxBufReset
(  mcxBuf*  buf
,  void*    mempptr
)  ;

#endif


