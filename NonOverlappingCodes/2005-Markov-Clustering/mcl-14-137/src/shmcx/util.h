/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef mcx_util_h__
#define mcx_util_h__

#include "util/types.h"


/* fixme; make decent enum for mode */

void zmTell
(  int   mode
,  const char* fmt
,  ...
)
#ifdef __GNUC__
__attribute__ ((format (printf, 2, 3)))
#endif
   ;


void zmNotSupported1
(  const char* who
,  int utype1
)  ;


void zmNotSupported2
(  const char* who
,  int utype1
,  int utype2
)  ;

extern mcxbits  v_g;
extern int digits_g;

#define  V_STACK (1 << 0)
#define  V_HDL   (1 << 1)
#define  V_TRACE (1 << 2)

#endif

