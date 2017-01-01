/*   (C) Copyright 2004, 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_gralloc_h
#define tingea_gralloc_h

#include "types.h"

/*
 * gralloc; grid memory allocation; allocation of equally sized chunks
*/

typedef struct mcxGrim mcxGrim;

#define MCX_GRIM_GEOMETRIC    1
#define MCX_GRIM_ARITHMETIC   2

mcxGrim* mcxGrimNew
(  dim       sz_unit
,  dim       n_units      /* initial capacity */
,  mcxbits  options
)  ;  

void* mcxGrimGet
(  mcxGrim*   src
)  ;

void mcxGrimLet
(  mcxGrim* src
,  void* mem
)  ;

dim mcxGrimCount
(  mcxGrim* src
)  ;

dim mcxGrimMemSize
(  mcxGrim* src
)  ;

void mcxGrimFree
(  mcxGrim** src
)  ;

#endif


