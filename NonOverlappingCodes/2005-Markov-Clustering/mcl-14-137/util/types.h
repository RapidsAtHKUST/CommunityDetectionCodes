/*   (C) Copyright 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef types_h
#define types_h

#include "inttypes.h"

typedef unsigned long mcxbits  ;
typedef unsigned long mcxmode  ;
typedef unsigned long mcxenum  ;
         /* mcxstatus   defined below */
         /* mcxbool     defined below */

#define BIT_ON(var,bits)   (var) |= (bits)
#define BIT_OFF(var, bits)  do { (var) |= (bits); (var) ^= (bits); } while (0)

#define ALL_BITS_OFF   0

#define VOID_TO_UINT (unsigned)
#define UINT_TO_VOID (void*)
#define VOID_TO_ULONG (unsigned long)
#define ULONG_TO_VOID (void*)

#define NOTHING   do { } while (0)


/*  **************************************************************************
 * *
 **            Implementation notes (a few).
 *
 *
 *  The intent is that routines that return mcxstatus can define their own
 *  values, so one must check the interface of a given routine as to what kind
 *  of symbolic values it may return.
 *
 *  For example, the opt library defines MCX_OPT_OK, MCX_OPT_NOARG, and
 *  MCX_OPT_UNKNOWN.
 *
 *  The intent is also that the OK value is always defined as zero -- it's
 *  not the zero that counts but the fact that all OK values (e.g. STATUS_OK,
 *  MCX_OPT_OK etc) are equal. One should be able to count on this.
 *  If multiple ok values are possible, use one of mcxenum, mcxbits, mcxmode.
 *
*/

typedef enum
{  STATUS_OK      =  0
,  STATUS_FAIL
,  STATUS_DONE             /* for iterator type interfaces (e.g. readLine) */
,  STATUS_IGNORE           /* for iterator type interfaces (line parser)   */
,  STATUS_NOMEM
,  STATUS_ABORT            /* e.g. user response                           */
,  STATUS_NEW              /* for cache type interfaces                    */
,  STATUS_UNUSED           /* use this as lower bound for new statuses     */
}  mcxstatus         ;

extern const char* mcx_status_list[];

#define MCXSTATUS(status)           \
      (  status <= STATUS_UNUSED    \
      ?  mcx_status_list[status]    \
      :  "NO_such_status!"          \
      )

#ifndef FALSE
   typedef enum
   {  FALSE       =  0
   ,  TRUE        =  1
   }  mcxbool        ;
#else
   typedef int mcxbool ;
#endif

typedef enum
{  RETURN_ON_FAIL =  1960
,  EXIT_ON_FAIL
,  SLEEP_ON_FAIL
,  ENQUIRE_ON_FAIL    /* user sets TINGEA_MEM_DO to e.g. exit or retry */
}  mcxOnFail         ;


#define  MCX_DATUM_THREADING  1
#define  MCX_DATUM_FIND       2
#define  MCX_DATUM_INSERT     4
#define  MCX_DATUM_DELETE     8

#endif

