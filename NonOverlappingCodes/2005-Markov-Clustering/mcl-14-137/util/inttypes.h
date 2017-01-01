/*   (C) Copyright 2001, 2002, 2003, 2004, 2005, 2006 Stijn van Dongen
 *   (C) Copyright 2007, 2008, 2009, 2010, 2011       Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#ifndef tingea_inttypes_h
#define tingea_inttypes_h

#include <limits.h>
#include <sys/types.h>


#if UINT_MAX >= 4294967295
#  define MCX_UINT32 unsigned int
#  define MCX_INT32  int
#else
#  define MCX_UINT32 unsigned long
#  define MCX_INT32  long
#endif


typedef  MCX_UINT32     u32 ;       /* at least 32 bits */
typedef  MCX_INT32      i32 ;       /* at least 32 bits */
typedef  unsigned char  u8  ;       /* at least  8 bits */

#define NO_BITS_SET 0

#ifndef ulong
#  define ulong unsigned long
#endif

#ifndef uchar
#  define uchar unsigned char
#endif

            /*  dim     is garantueed to be an unsigned type.
             *  ofs     is garantueed to be the corresponding signed type.
            */
#if 0
#  define  dim          size_t
#  define  ofs         ssize_t
#else
   typedef  size_t         dim;
   typedef  ssize_t        ofs;
#endif

#ifdef SIZE_MAX
#  define   DIM_MAX        SIZE_MAX
#else
#  define   DIM_MAX        ((size_t)-1)
#endif

#ifdef SSIZE_MAX
#  define   OFS_MAX        SSIZE_MAX
#else
#  define   OFS_MAX        LONG_MAX    /* lame, reasonable stopgap */
#endif

#ifdef SSIZE_MIN
#  define   OFS_MIN        SSIZE_MIN
#else
#  define   OFS_MIN        LONG_MIN    /* lame, reasonable stopgap */
#endif


                     /* annotate 'unsigned due to prototype'
                      * and related messages
                     */
#define different_sign
#define different_width


#endif


