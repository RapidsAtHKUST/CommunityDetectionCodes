/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#ifndef mcx_stack_h__
#define mcx_stack_h__

#include <stdarg.h>

#include "glob.h"


typedef struct zscard_t *zscard_p;

int zsHaveNargs
(  int n
)  ;
zscard_p zsNew
(  zscard_p    prev
,  zgglob_p    glob
)  ;
void zsFree
(  zscard_p*    cardpp
)  ;
int zsDoSequence
(  char* seq
)  ;
int zsPop
(  void
)  ;
int zsPush
(  zgglob_p     glob
)  ;
int zsShift
(  int n
,  int time
)  ;
int zsRoll
(  int n
,  int j
)  ;
int zsClear
(  void
)  ;
int zsExch
(  void
)  ;
int zsList
(  int n
)  ;
int zsEmpty
(  void
)  ;
zgglob_p zsGetGlob
(  int depth
)  ;


zscard_p zsGetCard
(  int depth
)  ;
void* zsGetOb
(  int depth
,  int type
)  ;
void* zsGetMem
(  int depth
,  int class
)  ;
zgglob_p zsGetGlob
(  int depth
) ;

int zsGetType
(  int  depth
)  ;

#endif
