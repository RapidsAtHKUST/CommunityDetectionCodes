/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <stdarg.h>
#include <stdio.h>

#include "util.h"
#include "glob.h"

#include "util/types.h"

#include "impala/io.h"


mcxbits   v_g   =  1;
int digits_g    =  MCLXIO_VALUE_GETENV;


void zmTell
(  int   mode
,  const char* fmt
,  ...
)
   {  va_list  args
   ;  const char* band = "   "

   ;  if
      (  (mode == 'd' && !(v_g & V_HDL))
      || (mode == 't' && !(v_g & V_TRACE))
      || (mode == 'o' && !(v_g & V_TRACE))
      )
      return

   ;  switch(mode)
      {  case 'e' :  band = "###";  break  
      ;  case 'd' :  band = "---";  break  
      ;  case 't' :  band = "...";  break  
      ;  case 'o' :  band = "<->";  break  
      ;  case 'v' :  band = " * ";  break
      ;  case 'm' :  band = " @ ";  break
   ;  }

      va_start(args, fmt)
   ;  fprintf(stderr, "%s ", band)
   ;  vfprintf(stderr, fmt, args)
   ;  fprintf(stderr, "\n")
   ;  va_end(args)
;  }


void zmNotSupported1
(  const char* who
,  int utype
)
   {  zmTell('e', "<%s> [%s] not supported", zgGetTypeName(utype), who)
;  }


void zmNotSupported2
(  const char* who
,  int utype1
,  int utype2
)
   {  zmTell
      (  'e'
      ,  "<%s> <%s> [%s] not supported"
      ,  zgGetTypeName(utype1)
      ,  zgGetTypeName(utype2)
      ,  who
      )
;  }

