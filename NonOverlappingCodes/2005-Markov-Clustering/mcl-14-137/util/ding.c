/*   (C) Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009, 2010, 2011  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "ding.h"
#include "types.h"
#include "alloc.h"
#include "minmax.h"


char* mcxStrDup
(  const char* str
)
   {  return mcxStrNDup(str, strlen(str))
;  }


char* mcxStrNDup
(  const char* str
,  dim n
)
   {  char* rts = mcxAlloc(n+1, RETURN_ON_FAIL)
   ;  if (rts)
         memcpy(rts, str, n)
      ,  rts[n] = '\0'
   ;  return rts
;  }


dim mcxStrCountChar
(  const char*    p
,  char           c
,  ofs            len
)
   {  const char* z = p
   ;  dim      ct =  0

   ;  z += (len >= 0) ? (dim) len : strlen(p)

   ;  while (p<z)       /* ho hum will pass embedded nil bytes */
      if (*p++ == c)
      ct++

   ;  return ct
;  }


char* mcxStrChrIs
(  const char*    p
,  int          (*fbool)(int c)
,  ofs            len
)
   {  if (len)
      while (*p && !fbool((unsigned char) *p) && --len && ++p)
      ;
      return (char*) ((len && *p) ?  p : NULL)
;  }


char* mcxStrChrAint
(  const char*    p
,  int          (*fbool)(int c)
,  ofs            len
)
   {  if (len)
      while (*p && fbool((unsigned char) *p) && --len && ++p)
      ;
      return (char *) ((len && *p) ?  p : NULL)
;  }


char* mcxStrRChrIs
(  const char*    p
,  int          (*fbool)(int c)
,  ofs            offset
)
   {  const char* z =  offset >= 0 ? p+offset : p + strlen(p)
   ;  while (--z >= p && !fbool((unsigned char) *z))
      ;
      return (char*) (z >= p ? z : NULL)
;  }


char* mcxStrRChrAint
(  const char*    p
,  int          (*fbool)(int c)
,  ofs            offset
)
   {  const char* z  =  offset >= 0 ? p+offset : p + strlen(p)
   ;  while (--z >= p && fbool((unsigned char) *z))
      ;
      return (char*) (z >= p ? z : NULL)
;  }


const char* bit8[256]
=  {  "00000000"
   ,  "00000001"
   ,  "00000010"
   ,  "00000011"
   ,  "00000100"
   ,  "00000101"
   ,  "00000110"
   ,  "00000111"
   ,  "00001000"
   ,  "00001001"
   ,  "00001010"
   ,  "00001011"
   ,  "00001100"
   ,  "00001101"
   ,  "00001110"
   ,  "00001111"
   ,  "00010000"
   ,  "00010001"
   ,  "00010010"
   ,  "00010011"
   ,  "00010100"
   ,  "00010101"
   ,  "00010110"
   ,  "00010111"
   ,  "00011000"
   ,  "00011001"
   ,  "00011010"
   ,  "00011011"
   ,  "00011100"
   ,  "00011101"
   ,  "00011110"
   ,  "00011111"
   ,  "00100000"
   ,  "00100001"
   ,  "00100010"
   ,  "00100011"
   ,  "00100100"
   ,  "00100101"
   ,  "00100110"
   ,  "00100111"
   ,  "00101000"
   ,  "00101001"
   ,  "00101010"
   ,  "00101011"
   ,  "00101100"
   ,  "00101101"
   ,  "00101110"
   ,  "00101111"
   ,  "00110000"
   ,  "00110001"
   ,  "00110010"
   ,  "00110011"
   ,  "00110100"
   ,  "00110101"
   ,  "00110110"
   ,  "00110111"
   ,  "00111000"
   ,  "00111001"
   ,  "00111010"
   ,  "00111011"
   ,  "00111100"
   ,  "00111101"
   ,  "00111110"
   ,  "00111111"
   ,  "01000000"
   ,  "01000001"
   ,  "01000010"
   ,  "01000011"
   ,  "01000100"
   ,  "01000101"
   ,  "01000110"
   ,  "01000111"
   ,  "01001000"
   ,  "01001001"
   ,  "01001010"
   ,  "01001011"
   ,  "01001100"
   ,  "01001101"
   ,  "01001110"
   ,  "01001111"
   ,  "01010000"
   ,  "01010001"
   ,  "01010010"
   ,  "01010011"
   ,  "01010100"
   ,  "01010101"
   ,  "01010110"
   ,  "01010111"
   ,  "01011000"
   ,  "01011001"
   ,  "01011010"
   ,  "01011011"
   ,  "01011100"
   ,  "01011101"
   ,  "01011110"
   ,  "01011111"
   ,  "01100000"
   ,  "01100001"
   ,  "01100010"
   ,  "01100011"
   ,  "01100100"
   ,  "01100101"
   ,  "01100110"
   ,  "01100111"
   ,  "01101000"
   ,  "01101001"
   ,  "01101010"
   ,  "01101011"
   ,  "01101100"
   ,  "01101101"
   ,  "01101110"
   ,  "01101111"
   ,  "01110000"
   ,  "01110001"
   ,  "01110010"
   ,  "01110011"
   ,  "01110100"
   ,  "01110101"
   ,  "01110110"
   ,  "01110111"
   ,  "01111000"
   ,  "01111001"
   ,  "01111010"
   ,  "01111011"
   ,  "01111100"
   ,  "01111101"
   ,  "01111110"
   ,  "01111111"
   ,  "10000000"
   ,  "10000001"
   ,  "10000010"
   ,  "10000011"
   ,  "10000100"
   ,  "10000101"
   ,  "10000110"
   ,  "10000111"
   ,  "10001000"
   ,  "10001001"
   ,  "10001010"
   ,  "10001011"
   ,  "10001100"
   ,  "10001101"
   ,  "10001110"
   ,  "10001111"
   ,  "10010000"
   ,  "10010001"
   ,  "10010010"
   ,  "10010011"
   ,  "10010100"
   ,  "10010101"
   ,  "10010110"
   ,  "10010111"
   ,  "10011000"
   ,  "10011001"
   ,  "10011010"
   ,  "10011011"
   ,  "10011100"
   ,  "10011101"
   ,  "10011110"
   ,  "10011111"
   ,  "10100000"
   ,  "10100001"
   ,  "10100010"
   ,  "10100011"
   ,  "10100100"
   ,  "10100101"
   ,  "10100110"
   ,  "10100111"
   ,  "10101000"
   ,  "10101001"
   ,  "10101010"
   ,  "10101011"
   ,  "10101100"
   ,  "10101101"
   ,  "10101110"
   ,  "10101111"
   ,  "10110000"
   ,  "10110001"
   ,  "10110010"
   ,  "10110011"
   ,  "10110100"
   ,  "10110101"
   ,  "10110110"
   ,  "10110111"
   ,  "10111000"
   ,  "10111001"
   ,  "10111010"
   ,  "10111011"
   ,  "10111100"
   ,  "10111101"
   ,  "10111110"
   ,  "10111111"
   ,  "11000000"
   ,  "11000001"
   ,  "11000010"
   ,  "11000011"
   ,  "11000100"
   ,  "11000101"
   ,  "11000110"
   ,  "11000111"
   ,  "11001000"
   ,  "11001001"
   ,  "11001010"
   ,  "11001011"
   ,  "11001100"
   ,  "11001101"
   ,  "11001110"
   ,  "11001111"
   ,  "11010000"
   ,  "11010001"
   ,  "11010010"
   ,  "11010011"
   ,  "11010100"
   ,  "11010101"
   ,  "11010110"
   ,  "11010111"
   ,  "11011000"
   ,  "11011001"
   ,  "11011010"
   ,  "11011011"
   ,  "11011100"
   ,  "11011101"
   ,  "11011110"
   ,  "11011111"
   ,  "11100000"
   ,  "11100001"
   ,  "11100010"
   ,  "11100011"
   ,  "11100100"
   ,  "11100101"
   ,  "11100110"
   ,  "11100111"
   ,  "11101000"
   ,  "11101001"
   ,  "11101010"
   ,  "11101011"
   ,  "11101100"
   ,  "11101101"
   ,  "11101110"
   ,  "11101111"
   ,  "11110000"
   ,  "11110001"
   ,  "11110010"
   ,  "11110011"
   ,  "11110100"
   ,  "11110101"
   ,  "11110110"
   ,  "11110111"
   ,  "11111000"
   ,  "11111001"
   ,  "11111010"
   ,  "11111011"
   ,  "11111100"
   ,  "11111101"
   ,  "11111110"
   ,  "11111111"
   }  ;


mcxTing* mcxMemPrint
(  mcxTing* ting
,  void*    p
,  dim      n_bytes
,  mcxbits  flags
)
   {  char* m = p
   ;  mcxbool space = !(flags & MCX_MEMPRINT_NOSPACE)
   ;  mcxbool rev   =   flags & MCX_MEMPRINT_REVERSE
   ;  dim i

   ;  ting = mcxTingEmpty(ting, 32)

   ;  for (i=0;i<n_bytes;i++)
      mcxTingPrintAfter
      (  ting
      ,  "%s%s"
      ,  space && i ? " " : ""
      ,  bit8[0xff & (unsigned char) m[rev ? n_bytes-i-1 : i]]
      )
   ;  return ting
;  }


int mcxEditDistance
(  const char* s
,  const char* t
,  int* lcs
)
   {  dim ls = strlen(s)
   ;  dim lt = strlen(t)
   ;  int* tbl
   ;  dim i, j
   ;  int min_val = 0
   ;  *lcs = -1

#define TBL(i,j)  tbl[(i)*(ls+1)+(j)]

   ;  if (ls == 0 || lt == 0)
      return -999;

   ;  if (!(tbl = malloc((ls+1) * (lt+1) * sizeof (int))))
      return -1000;

   ;  for (i=0;i<=lt;i++)
      for (j=0;j<=ls;j++)
      TBL(i,j) = 0

   ;  for (i=0;i<=ls;i++)
      TBL(0,i) = i 

   ;  for (i=0;i<lt;i++)
      for (j=0;j<ls;j++)
         TBL(i+1,j+1) = MCX_MIN(1 + TBL(i,j+1), 1 + TBL(i+1,j))
      ,  TBL(i+1,j+1) = MCX_MIN(TBL(i+1,j+1), TBL(i,j) + (t[i] == s[j] ? 0 : 1))


   ;  min_val = TBL(lt, ls)

   ;  for (i=0;i<lt;i++)
      if (TBL(i,ls) < min_val)
      min_val = TBL(i,ls);

     /* 2. try to insert t into s */
   ;  for (i=0;i<=lt;i++)
      TBL(i,0) = i

   ;  for (i=0;i<=ls;i++)
      TBL(0,i) = 0 

   ;  for (i=0;i<lt;i++)
      for (j=0;j<ls;j++)
         TBL(i+1,j+1) = MCX_MIN(1 + TBL(i,j+1), 1 + TBL(i+1,j))
      ,  TBL(i+1,j+1) = MCX_MIN(TBL(i+1,j+1), TBL(i,j) + (t[i] == s[j] ? 0 : 1))


     /*  find best answer */

   ;  for (i=0;i<ls;i++)
      if (TBL(lt,i+1) < min_val)
      min_val = TBL(lt, i+1)

   ;  if (lcs)
      {  int best = 0
      ;  if (ls == 0 || lt == 0)
         return -999;

      ;  for (i=0;i<=lt;i++)
         TBL(i,0) = 0

      ;  for (i=0;i<=ls;i++)
         TBL(0,i) = 0

      ;  for (i=0;i<lt;i++)
         for (j=0;j<ls;j++)
         {  TBL(i+1,j+1) = (TBL(i,j) + 1) * (t[i] == s[j] ? 1 : 0)
         ;  if (TBL(i+1,j+1) > best)
            best = TBL(i+1,j+1)
      ;  }
         *lcs = best
   ;  }

      free(tbl)
   ;  return min_val
;  }


int mcxSetenv
(  const char* kv
)
   {  mcxTing* tv = mcxTingNew(kv)
   ;  if (!strchr(kv, '='))
      mcxTingAppend(tv, "=1")
   ;  return putenv(mcxTinguish(tv))
;  }



mcxstatus mcxStrTol
(  const    char* s
,  long*    value
,  char**   end
)
   {  int errno_sa = errno
   ;  char* e  =  NULL
   ;  mcxstatus status = STATUS_OK

   ;  errno    =  0
   ;  *value   =  strtol(s, &e, 10)

   ;  if (errno || e == s)
      status = STATUS_FAIL

   ;  errno = errno_sa
   ;  if (end)
      *end = e
   ;  return status
;  }


mcxstatus mcxStrToul
(  const    char* s
,  ulong*   value
,  char**   end
)
   {  int errno_sa = errno
   ;  mcxstatus status = STATUS_OK
   ;  char* e  =  NULL

   ;  errno    =  0
   ;  *value = strtoul(s, &e, 10)

   ;  if (errno || e == s)
      status = STATUS_FAIL

   ;  errno = errno_sa
   ;  if (end)
      *end = e
   ;  return status
;  }

