/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_ding
#define tingea_ding

#include <string.h>
#include "types.h"
#include "ting.h"


char* mcxStrDup
(  const char* str
)  ;

char* mcxStrNDup
(  const char* str
,  dim n
)  ;

mcxstatus mcxStrTol
(  const char* s
,  long*    value
,  char**   end
)  ;

mcxstatus mcxStrToul
(  const char* s
,  ulong*   value
,  char**   end
)  ;


/*
 * if len < 0, strlen(p) is used.
*/

dim mcxStrCountChar
(  const char*    p
,  char           c
,  ofs            len         /* -1 for don't know */
)  ;

char* mcxStrChrIs
(  const char*    src
,  int          (*fbool)(int c)
,  ofs            len         /* -1 for don't know */
)  ;

char* mcxStrChrAint
(  const char*    src
,  int          (*fbool)(int c)
,  ofs            len         /* -1 for don't know */
)  ;

char* mcxStrRChrIs
(  const char*    src
,  int          (*fbool)(int c)
,  ofs            offset         /* -1 for don't know */
)  ;

char* mcxStrRChrAint
(  const char*    src
,  int          (*fbool)(int c)
,  ofs            offset         /* -1 for don't know */
)  ;


#define MCX_MEMPRINT_REVERSE  1
#define MCX_MEMPRINT_NOSPACE  2


/* Strides in char units, treats/truncates each unit
 * as/to an 8-bit entity.
*/

mcxTing* mcxMemPrint
(  mcxTing* ting
,  void*    p
,  dim      n_bytes
,  mcxbits  flags
)  ;


int mcxEditDistance
(  const char* s1
,  const char* s2
,  int* lcs
)  ;


int mcxSetenv
(  const char* kv
)  ;

#endif


