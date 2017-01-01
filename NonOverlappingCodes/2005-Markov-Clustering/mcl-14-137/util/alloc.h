/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_alloc_h
#define tingea_alloc_h

#include <stdio.h>

/* ========================================================================= *
 *
 *    mcxRealloc is notable, for it *frees* memory and returns a NULL pointer
 *    if the newly requested block has size 0.  This must be so in order to
 *    allow routines to do their math and housekeeping. If they register the
 *    new block as having 0 bytes, then they need not and must not attempt to
 *    free it thereafter.
 *
 * ========================================================================= *
*/


#include "types.h"


void* mcxAlloc
(  dim               size
,  mcxOnFail         ON_FAIL
)  ;

void* mcxRealloc
(  void*             object
,  dim               new_size
,  mcxOnFail         ON_FAIL
)  ;

void mcxFree
(  void*             object
)  ;

void mcxNFree
(  void*             base
,  dim               n_elem
,  dim               elem_size
,  void            (*obRelease) (void *)
)  ;

void* mcxNAlloc
(  dim               n_elem
,  dim               elem_size
,  void*           (*obInit) (void *)
,  mcxOnFail         ON_FAIL
)  ;

void* mcxNRealloc
(  void*             mem
,  dim               n_elem
,  dim               n_elem_prev
,  dim               elem_size
,  void* (*obInit) (void *)
,  mcxOnFail         ON_FAIL
)  ;

void mcxMemDenied
(  FILE*             channel
,  const char*       requestee
,  const char*       unittype
,  dim               n
)  ;

void mcxAllocLimits
(  long  maxchunksize
,  long  maxtimes
)  ;


#endif

