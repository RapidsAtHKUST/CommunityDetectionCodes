/*   (C) Copyright 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_compile_h
#define tingea_compile_h


#ifndef TINGEA__TYPED_MINMAX
#  define TINGEA__TYPED_MINMAX 0
#endif


#ifndef __GNUC__
#  define   MCX_GNUC_OK       0
#  define inline         /* no inline */
#  define cpl__pure         /* no pure */ 
#  define cpl__const        /* no const */
#  define cpl__noreturn     /* no noreturn */ 
#  define cpl__malloc       /* no malloc */
#  define cpl__must_check   /* no warn_unused_result */
#  define cpl__deprecated   /* no deprecated */ 
#  define cpl__used         /* no used */
#  define cpl__unused       /* no unused */
#  define cpl__packed       /* no packed */
#  define likely(x)      (x)
#  define unlikely(x)    (x)
#else
#define   MCX_GNUC_OK       1
#if __GNUC__  >= 3
#  define inline __inline__ __attribute__ ((always_inline))
#  define cpl__pure         __attribute__ ((pure))
#  define cpl__const        __attribute__ ((const))
#  define cpl__noreturn     __attribute__ ((noreturn))
#  define cpl__malloc       __attribute__ ((malloc))
#  define cpl__must_check   __attribute__ ((warn_unused_result))
#  define cpl__deprecated   __attribute__ ((deprecated))
#  define cpl__used         __attribute__ ((used))
#  define cpl__unused       __attribute__ ((unused))
#  define cpl__packed       __attribute__ ((packed))
#  define likely(x)      __builtin_expect (!!(x), 1)
#  define unlikely(x)    __builtin_expect (!!(x), 0)
#else
#  define inline         /* no inline */
#  define cpl__pure         /* no pure */ 
#  define cpl__const        /* no const */
#  define cpl__noreturn     /* no noreturn */ 
#  define cpl__malloc       /* no malloc */
#  define cpl__must_check   /* no warn_unused_result */
#  define cpl__deprecated   /* no deprecated */ 
#  define cpl__used         /* no used */
#  define cpl__unused       /* no unused */
#  define cpl__packed       /* no packed */
#  define likely(x)      (x)
#  define unlikely(x)    (x)
#endif
#endif

#endif

