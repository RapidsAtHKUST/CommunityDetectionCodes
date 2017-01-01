/*   (C) Copyright 2005, 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <stdio.h>
#include <stdlib.h>

#include "app.h"
#include "io.h"
#include "version.h"
#include "iface.h"

#include "util/ting.h"
#include "util/err.h"

void app_report_version
(  const char* me
)
   {  fprintf
      (  stdout
      ,  "%s %s\n"
"Copyright (c) 1999-%s, Stijn van Dongen. mcl comes with NO WARRANTY\n"
"to the extent permitted by law. You may redistribute copies of mcl under\n"
"the terms of the GNU General Public License.\n"
      ,   me
      ,  mclDateTag
      ,  mclYear
      )
;  }


void mclxSetBinaryIO
(  void
)
   {  mcxTing* tmp = mcxTingPrint(NULL, "MCLXIOFORMAT=8")
   ;  char* str = mcxTinguish(tmp)
   ;  putenv(str)
;  }


void mclxSetInterchangeIO
(  void
)
   {  mcxTing* tmp = mcxTingPrint(NULL, "MCLXIOFORMAT=2")
   ;  char* str = mcxTinguish(tmp)
   ;  putenv(str)
;  }


void mclx_app_init
(  FILE* fp
)
   {  const char* envp  =  getenv("NU_MAGIC")
   ;  mcxLogSetFILE(fp, TRUE)
   ;  if (envp)
      nu_magic = atof(envp)
;  }



dim mclx_set_threads_or_die
(  const char* caller
,  dim n_thread_l
,  dim i_group_G
,  dim n_group_G
)
   {  if (!n_group_G)
      mcxDie(1, caller, "-t thread option requires reasonable -J and -j values")
   ;  if (i_group_G >= n_group_G)
      mcxDie(1, caller, "-j argument must be smaller than -J argument")
   ;  if (!n_thread_l)
      n_thread_l = 1
   ;  return n_thread_l
;  }




