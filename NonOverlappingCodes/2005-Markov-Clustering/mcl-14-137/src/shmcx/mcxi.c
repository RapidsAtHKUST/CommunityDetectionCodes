/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

#include <stdio.h>

#include "stack.h"
#include "glob.h"
#include "ops.h"
#include "util.h"

#include "util/io.h"
#include "util/ting.h"
#include "util/err.h"

#include "impala/io.h"


int main
(  int               argc
,  const char*       argv[]
)
   {  int  a               =  0
   ;  mcxTing* argtxt      =  mcxTingEmpty(NULL, 10)

   ;  if (0)
      mclxIOsetQMode("MCLXIOVERBOSITY", MCL_APP_VB_NO)

   ;  opInitialize()       /* symtable etc */
   ;  globInitialize()     /* hdltable etc */

   ;  if (argc == 1)
      {  mcxTing* ops      =  mcxTingEmpty(NULL, 20)
      ;  mcxIO *xfin       =  mcxIOnew("-", "r")
      ;  mcxIOopen(xfin, EXIT_ON_FAIL)

#define TMPTST isatty(fileno(stdin))
      ;  if (TMPTST)
         fprintf
         (  stdout
         ,  "At your service: "
            "'[/<op>] help', '[/<str>] grep', 'ops', 'info', and 'quit'.\n"
         )

      ;  while (1)
         {  int ok
         ;  mcxTing* line  =  mcxTingEmpty(NULL, 30)

         ;  if (TMPTST)
               fprintf(stdout, "> ")
            ,  fflush(stdout)

         ;  if (STATUS_OK != mcxIOreadLine(xfin, line, MCX_READLINE_BSC))
            {  if (TMPTST)
               fprintf(stdout, "curtains!\n")
            ;  mcxTingFree(&line)
            ;  break
         ;  }
            mcxTingAppend(ops, line->str)
         ;  mcxTingFree(&line)

         ;  ok = zsDoSequence(ops->str)

         ;  if (ok && (v_g & V_STACK))
            zsList(0)

         ;  mcxTingEmpty(ops, 20)
      ;  }
      }
      else
      {  for (a=1;a<argc;a++)
         {  mcxTingWrite(argtxt, argv[a])
         ;  if (!zgUser(argtxt->str))
            mcxExit(1)
      ;  }
      }
      mcxTingFree(&argtxt)
   ;  opExit()
   ;  globExit()
   ;  return 0
;  }


