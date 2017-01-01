/*   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include "mcx.h"

#include "impala/io.h"
#include "impala/matrix.h"
#include "impala/tab.h"

#include "util/types.h"
#include "util/io.h"
#include "util/err.h"
#include "util/opt.h"


static int cmp_keys
(  const void* kv1pp
,  const void* kv2pp
)
   {  const mcxTing* tg1 = ((mcxKV**) kv1pp)[0]->key
   ;  const mcxTing* tg2 = ((mcxKV**) kv2pp)[0]->key
   ;  int i = strcmp(tg1->str, tg2->str)
   ;  return i
;  }


enum
{  MY_OPT_MEET  = MCX_DISP_UNUSED
,  MY_OPT_MERGE
,  MY_OPT_LEFT
,  MY_OPT_CLEAN
}  ;


mcxOptAnchor tabOptions[] =
{  {  "--meet"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_MEET
   ,  NULL
   ,  "compute a tab file that is the meet of the tab files given"
   }
,  {  "--merge"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_MERGE
   ,  NULL
   ,  "compute a tab file that is the merge of the tab files given"
   }
,  {  "--left"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_LEFT
   ,  NULL
   ,  "compute a tab file with entries exclusive to the first file given"
   }
,  {  "--clean"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_CLEAN
   ,  NULL
   ,  "clean up tab file"
   }
,  {  NULL, 0, MCX_DISP_UNUSED, NULL, NULL }
}  ;


static  mcxmode mode  = -1;


static mcxstatus tabInit
(  void
)
   {  mode = 'j'
   ;  return STATUS_OK
;  }


static mcxstatus tabArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_MEET
      :  mode = 'i'        /* intersect */
      ;  break
      ;

         case MY_OPT_CLEAN
      :  mode = 'c'        /* join */
      ;  break
      ;

         case MY_OPT_MERGE
      :  mode = 'j'        /* join */
      ;  break
      ;

         case MY_OPT_LEFT
      :  mode = 'l'        /* join */
      ;  break
      ;

         default
      :  mcxExit(1) 
      ;
      }
   ;  return STATUS_OK
;  }



static mcxstatus tabMain
(  int                  argc
,  const char*          argv[]
)
   {  int a
   ;  mclTab* tab  = NULL
   ;  mcxIO* xftab = NULL
   ;  mcxHash* map = NULL
   ;  mcxbool meet = mode == 'i'
   ;  mcxbool merge= mode == 'j'
   ;  mcxbool left = mode == 'l'
   ;  mcxbool clean= mode == 'c'

   ;  if (argc < 1)
      mcxDie(1, "mcx tab", "no arguments!, I argue")

   ;  xftab = mcxIOnew(argv[0], "r")
   ;  mcxIOopen(xftab, EXIT_ON_FAIL)
   ;  tab = mclTabRead(xftab, NULL, EXIT_ON_FAIL)
   ;  mcxIOclose(xftab)

#if 0
   ;  if (clean)
      {  mcxIO* xfout = mcxIOnew("-", "w")
      ;  mclTabWrite(tab, xfout, NULL, EXIT_ON_FAIL)
      ;  mcxIOclose(xfout)
   ;  }
#endif

   ;  map = mclTabHash(tab)
   ;  mclTabHashSet(map, 1)

   ;  for (a=1;a<argc;a++)
      {  mclTab* tab2 = NULL
      ;  dim i
      ;  mcxIOrenew(xftab, argv[a], NULL)
      ;  mcxIOopen(xftab, EXIT_ON_FAIL)

      ;  tab2 = mclTabRead(xftab, NULL, EXIT_ON_FAIL)
      ;  mcxIOclose(xftab)
      ;  for (i=0;i<N_TAB(tab2);i++)
         {  mcxTing* tg = mcxTingNew(tab2->labels[i])
         ;  mcxKV* kv = mcxHashSearch(tg, map, meet || left ? MCX_DATUM_FIND : MCX_DATUM_INSERT)

         ;  if (meet || left)
            {  if (kv)
               kv->val = ULONG_TO_VOID (VOID_TO_ULONG kv->val + 1)
            ;  mcxTingFree(&tg)
         ;  }
            else if (merge)
            {  if (kv && kv->key != tg)      /* already there */
               mcxTingFree(&tg)
            ;  else if (kv && kv->key == tg)
               kv->val = ULONG_TO_VOID 1LU
         ;  }
         }
         mclTabFree(&tab2)
   ;  }

      {  dim rk = 0, n_entries = 0
      ;  void** kvs = mcxHashKVs(map, &n_entries, cmp_keys, 0), **kvp
      ;  for (kvp = kvs; kvp < kvs + n_entries; kvp++)
         {  mcxKV* kv = kvp[0]
         ;  ulong  u  = VOID_TO_ULONG kv->val
         ;  mcxTing* tg = kv->key
         ;  if (merge || (meet && u >= argc) || (left && u == 1))
               fprintf(stdout, "%lu\t%s\n", (ulong) rk, tg->str)
            ,  rk++
      ;  }
      }
      return STATUS_OK
;  }


static mcxDispHook tabEntry
=  {  "tab"
   ,  "tab [files]"
   ,  tabOptions
   ,  sizeof(tabOptions)/sizeof(mcxOptAnchor) - 1

   ,  tabArgHandle
   ,  tabInit
   ,  tabMain

   ,  1
   ,  -1
   ,  MCX_DISP_DEFAULT
   }
;


mcxDispHook* mcxDispHookTab
(  void
)
   {  return &tabEntry
;  }


