/*   (C) Copyright 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

#include "err.h"
#include "types.h"
#include "compile.h"

/* TODO unify code.
*/

FILE*   mcxLogFILE         =  NULL;

static FILE*   mcx_fperr   =  NULL;
static FILE*   mcx_fpwarn  =  NULL;
static FILE*   mcx_fptell  =  NULL;

mcxbits mcxLogLevel = 0;


void  mcx_err_f
(  FILE*       fp
,  const char  *caller
,  const char  *fmt
,  va_list     *args
)
   {  if (caller)
      fprintf(fp, "___ [%s] ", caller)
   ;  else
      fprintf(fp, "___ ")

   ;  vfprintf(fp, fmt, *args)
   ;  fprintf(fp, "\n")

   ;  return
;  }


void mcxFail
(  void
)
   {  while(1) sleep(1000)
;  }


void mcxDie
(  int status
,  const char* caller
,  const char* fmt
,  ...
)
   {  va_list  args
   ;  va_start(args, fmt)
   ;  mcx_err_f(stderr, caller, fmt, &args)
   ;  va_end(args)
   ;  mcxExit(status)
;  }


void mcxExit
(  int val
)
   {  exit(val)
;  }


void mcxErrorFile
(  FILE* fp
)
   {  mcx_fperr = fp
;  }

void mcxTellFile
(  FILE* fp
)
   {  mcx_fptell = fp
;  }

void mcxLogSetFILE
(  FILE*    fp
,  mcxbool  ENV_LOG
)
   {  mcxLogFILE = fp
   ;  if (ENV_LOG)
      mcxLogLevelSetByString(getenv("TINGEA_LOG_TAG"))
;  }


FILE* mcxLogGetFILE
(  void
)
   {  return mcxLogFILE ? mcxLogFILE : stderr  
;  }


void mcxWarnFile
(  FILE* fp
)
   {  mcx_fpwarn = fp
;  }


void  mcxErrf
(  FILE*       fp
,  const char  *caller
,  const char  *fmt
,  ...
)
   {  va_list  args
   ;  va_start(args, fmt)
   ;  mcx_err_f(fp, caller, fmt, &args)
   ;  va_end(args)
;  }


void  mcxErr
(  const char  *caller
,  const char  *fmt
,  ...
)
   {  FILE* fp = mcx_fperr ? mcx_fperr : stderr  
   ;  va_list  args
   ;  va_start(args, fmt)
   ;  mcx_err_f(fp, caller, fmt, &args)
   ;  va_end(args)
;  }


void  mcxWarn
(  const char  *caller
,  const char  *fmt
,  ...
)
   {  va_list  args

   ;  if (caller)
      fprintf(stderr, "[%s] ", caller)

   ;  va_start(args, fmt)
   ;  vfprintf(stderr, fmt, args)
   ;  fprintf(stderr, "\n")
   ;  va_end(args)

   ;  return
;  }


static void  mcx_write_f
(  FILE*       fp
,  const char  *caller
,  const char  *fmt
,  va_list     *args
,  ...
)
   {  if (caller)
      fprintf(fp, "[%s] ", caller)

   ;  vfprintf(fp, fmt, *args)
   ;  fprintf(fp, "\n")

   ;  return
;  }


void  mcxTellf
(  FILE*       fp
,  const char  *caller
,  const char  *fmt
,  ...
)
   {  va_list  args
   ;  va_start(args, fmt)
   ;  mcx_write_f(fp, caller, fmt, &args)
   ;  va_end(args)
;  }


void  mcxTell
(  const char  *caller
,  const char  *fmt
,  ...
)
   {  FILE* fp = mcx_fptell ? mcx_fptell : stderr  
   ;  va_list  args
   ;  va_start(args, fmt)
   ;  mcx_write_f(fp, caller, fmt, &args)
   ;  va_end(args)
;  }


struct mcx_log_class_annotated
{  int      type
;  mcxbits  class
;  mcxbits  low
;
}  ;

struct mcx_log_class_annotated mcx_log_class_list[] =
{  {  'd'   ,  MCX_LOG_DATA      ,  MCX_LOG_DATA0     }
,  {  'f'   ,  MCX_LOG_FUNC      ,  MCX_LOG_FUNC0     }
,  {  'g'   ,  MCX_LOG_GAUGE     ,  MCX_LOG_GAUGE     }
,  {  'i'   ,  MCX_LOG_IO        ,  MCX_LOG_IO        }
,  {  'm'   ,  MCX_LOG_MON       ,  MCX_LOG_MON0      }
,  {  'n'   ,  MCX_LOG_NETWORK   ,  MCX_LOG_NETWORK   }
,  {  'p'   ,  MCX_LOG_IP        ,  MCX_LOG_IP        }
,  {  't'   ,  MCX_LOG_THREAD    ,  MCX_LOG_THREAD    }
,  {  'A'   ,  MCX_LOG_SLOT1     ,  MCX_LOG_SLOT1   }
,  {  'B'   ,  MCX_LOG_SLOT2     ,  MCX_LOG_SLOT2   }
,  {  'C'   ,  MCX_LOG_SLOT3     ,  MCX_LOG_SLOT3   }
}  ;


mcxbool mcxLogGet
(  mcxbits level_programmer
)
   {  dim i
   ;  mcxbool ok = FALSE
   ;  for (i=0;i<sizeof mcx_log_class_list/sizeof mcx_log_class_list[0];i++)
      {  mcxbits code_bits = level_programmer & mcx_log_class_list[i].class

#if 0
;fprintf
( stderr, "code %d user %d class %d user&class %d\n"
, (int) code_bits
, (int) mcxLogLevel
, (int) mcx_log_class_list[i].class
, (int) (mcxLogLevel & mcx_log_class_list[i].class)
)
#endif
      ;  if (!code_bits)
         continue

      ;  ok
         =  (  (code_bits && (mcxLogLevel & mcx_log_class_list[i].class))
            && (code_bits >= (mcxLogLevel & mcx_log_class_list[i].class))
            )

      ;  if (mcxLogLevel & MCX_LOG_OR)
         {  if (ok)
            return TRUE
      ;  }
         else if (!ok)   /* AND, implicit or not */
         return FALSE
   ;  }
      return ok
;  }


void mcxLog2
(  const char* tag
,  const char* fmt
,  ...
)
   {  FILE* fp = mcxLogFILE ? mcxLogFILE : stderr  
   ;  va_list  args
   ;  va_start(args, fmt)
   ;  mcx_write_f(fp, tag, fmt, &args)
   ;  va_end(args)
;  }


void mcxLog
(  mcxbits level_programmer
,  const char* tag
,  const char* fmt
,  ...
)
   {  FILE* fp = mcxLogFILE ? mcxLogFILE : stderr  
   ;  va_list  args
   ;  mcxbool ok = FALSE

   ;  if (!mcxLogLevel || (mcxLogLevel & MCX_LOG_NULL))
      return

   ;  ok = mcxLogGet(level_programmer)

   ;  if (ok)
      {  va_start(args, fmt)
      ;  mcx_write_f(fp, tag, fmt, &args)
      ;  va_end(args)
   ;  }
   }


/* dependency with mcx_level_setnum */

static int mcx_level_parsenum
(  int T
,  int c
)
   {  int r = -1
   ;  if (c == 'x')
      r = 0
   ;  else if (c >= '1' && c <= '9')
      r = ((int) c) - '0'
   ;  else
      mcxErr
      (  "mcxLogLevelSetByString"
      ,  "%c axis level [%c] parse error"
      ,  (int) T
      ,  (int) c
      )
   ;  return r
;  }


/* covers val -1 mcx_level_parsenum case */

static int mcx_level_setnum
(  int val
,  mcxbits CLASS
,  mcxbits CLASS0
)
   {  mcxbits new = 0
   ;  if (val > 0)
      {  new = (1 << (val-1)) * CLASS0

               /* below we know that new is simply too high: e.g. new == 6
                * but there are only 5 classes.
                * We then set new to the highest class member
               */
      ;  if (!(new & CLASS))
         new = CLASS ^ (CLASS >> 1) ^ (CLASS0 >> 1)
   ;  }
      return new
;  }


volatile sig_atomic_t mcxLogSigGuard = 0;

void mcxLogSig
(  int sig
)
   {  mcxLogSigGuard = sig
;  }


void mcxLogLevelSetByString
(  const char* str
)
   {  dim i
   ;  u8 str0 = str ? str[0] : 0

   ;  if (!str)
      return

   ;  if (str0 == 'x')
      mcxLogLevel = 0

   ;  else if (str0 == '1')
      mcxLogLevel = MCX_LOG_VERBOSE

   ;  else if (str0 == '8')
      mcxLogLevel = MCX_LOG_TERSE

   ;  else if (str0 == '9')
      mcxLogLevel = MCX_LOG_TERSER


   ;  if (strchr(str, 'V'))
      mcxLogLevel |= MCX_LOG_OR

   ;  if (strchr(str, '#'))
      mcxLogLevel |= MCX_LOG_NULL

   ;  if (strchr(str, '%'))
      BIT_OFF(mcxLogLevel, MCX_LOG_NULL)


   ;  for (i=0;i<sizeof mcx_log_class_list / sizeof mcx_log_class_list[0];i++)
      {  char* a = strchr(str, mcx_log_class_list[i].type)
      ;  if (a)
         {  int val =   mcx_level_parsenum
                        (  mcx_log_class_list[i].type
                        , (u8) a[1]
                        )
         ;  int new =   mcx_level_setnum
                        (  val
                        ,  mcx_log_class_list[i].class
                        ,  mcx_log_class_list[i].low
                        )
         ;  if (!val || new)
            {  BIT_OFF(mcxLogLevel, mcx_log_class_list[i].class)
            ;  BIT_ON(mcxLogLevel, new)
         ;  }
         }
      }
   }


