/*   (C) Copyright 2005, 2006, 2007, 2008, 2009 Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/


/* TODO
 * -  xtr_get_token does not check repeat pointer. crashable?
 * -  something that translates \012 to the byte value.
 * -  write regression test.
 * -  More error codes
 * -  Have a magic repeat operator that does not stop on boundaries.
 * -  Do sane things to implicit \0 (e.g. as result of complement) and
 *       characters outside ascii range.
 * -  [*a*256] leads to count 0.
 * #  tr a-z   A-C[*#*]X-Z    will not (now nor ever) lookahead
 * -  what about alpha tr(1) capabilities?
 *    6.  In the POSIX locale, to translate all ASCII characters that are not
 *        specified, enter:
 *             tr -c '[ -~]' '[A-_]' <textfile >newfile
*/

/* NOTE
   -  only room for 256 classes including sentinels C_CLASSSTART C_CLASSEND
   -  during tlt phase delete and squash bits are ignored and not respected
*/

/* randomly located documentation of intermediate and final tbl formats
 *
 *    css   class start sentinel
 *    ces   class end sentinel
 *    rps   repeat sentinel
 *    rss   range start sentinel
 *    res   range end sentinel
 *
 * Class:
 *    css | class , i1, i2, i3, .. , ces | class
 *          where i1 etc are to be mapped
 *
 * Repeat:
 *    rss | value, count
 *
 * Range
 *    rss | _ , i, i+1, i+2, i+3, .. , res | _
 *
 * Final tbl format: value stored in lower 8 bits.
 * Higher bits may contain MCX_TR_TRANSLATE, MCX_TR_DELETE MCX_TR_SQUASH
*/


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#include "tr.h"
#include "ting.h"
#include "types.h"
#include "ding.h"
#include "err.h"



const char*    X_REPEAT_UNEXPECTED  =  "repeat unexpected";
const char*    X_REPEAT_SYNTAX      =  "repeat syntax";
const char*    X_CLASS_UNKNOWN      =  "class unknown";
const char*    X_CLASS_UNEXPECTED   =  "class unexpected";
const char*    X_OCTAL_RANGE        =  "octal range";
const char*    X_OCTAL_SYNTAX       =  "octal syntax";
const char*    X_HEX_RANGE          =  "hex range";
const char*    X_HEX_SYNTAX         =  "hex syntax";
const char*    X_RANGE_INVERTED     =  "inverted range";

const char*    mcx_tr_err           =  NULL;
mcxbool        mcx_tr_debug         =  FALSE;


#define MAX_SPEC_INDEX 511

#define error_break  break

#define UC(c)    ((unsigned char) *(c))


static const char* us = "tr";


enum
{  C_CLASSSTART = 1u
,  C_ALNUM
,  C_ALPHA
,  C_CNTRL
,  C_DIGIT
,  C_GRAPH
,  C_LOWER
,  C_PRINT
,  C_PUNCT
,  C_SPACE
,  C_UPPER
,  C_XDIGIT
,  C_CLASSEND   /* 13 */

,  C_RANGESTART
,  C_RANGEEND

,  C_REPEAT    /* next token encodes repeat (stops on boundary)*/
,  C_FLOOD     /* next token encodeѕ flood  (floods all) */

,  C_EOF

}  ;


#define ISCLASS(cl) ((cl) > C_CLASSSTART && (cl) < C_CLASSEND)
#define ISREPEAT(cl) ((cl) == C_REPEAT || (cl) == C_FLOOD)

#define ISUCHARRANGE(i) \
   ( (i) >= 0 && (i) < 256 )

#define ISBOUNDARY(cl) \
  (    (cl) == C_CLASSSTART || (cl) == C_CLASSEND   \
    || (cl) == C_RANGESTART || (cl) == C_RANGEEND )

#define ISSTART(cl) \
  ( (cl) == C_CLASSSTART || (cl) == C_RANGESTART )

#define ISEND(cl) \
  ( (cl) == C_CLASSEND || (cl) == C_RANGEEND )

#define ISUPPERSTART(tok) \
   (  tok == ((C_CLASSSTART << 8) | C_UPPER)  )

#define ISLOWERSTART(tok) \
   (  tok == ((C_CLASSSTART << 8) | C_LOWER)  )


int (*isit[13])(int c) =
{  NULL           /* so C_CLASSVAL - C_CLASSSTART gives an index */
,  isalnum
,  isalpha
,  iscntrl
,  isdigit
,  isgraph
,  islower
,  isprint
,  ispunct
,  isspace
,  isupper
,  isxdigit
}  ;


mcxTing* mcxTRsplash
(  mcxTR*   tr
,  mcxbits  bits
)
   {  int i,j
   ;  u32* tbl = tr->tlt
   ;  mcxTing* splash = mcxTingEmpty(NULL, 256)

   ;  for (i=1,j=0;i<256;i++)          /* NOTE: \000 excluded */
      {  u32 tok     =  tbl[i]
      ;  u32 meta    =  tok >> 8

      ;  if
         (  ((bits & MCX_TR_SOURCE)     &&    (meta & MCX_TR_TRANSLATE))
         || ((bits & MCX_TR_SOURCE_C)   && !  (meta & MCX_TR_TRANSLATE))
         || ((bits & MCX_TR_SQUASH)     &&    (meta & MCX_TR_SQUASH))
         || ((bits & MCX_TR_SQUASH_C)   && !  (meta & MCX_TR_SQUASH))
         || ((bits & MCX_TR_DELETE)     &&    (meta & MCX_TR_DELETE))
         || ((bits & MCX_TR_DELETE_C)   && !  (meta & MCX_TR_DELETE))
         )
         splash->str[j++] = i
   ;  }
      splash->str[j] = '\0'
   ;  return splash
;  }


ofs mcxTRtranslate
(  char*    src
,  mcxTR*   tr
)
   {  dim i,j
   ;  int prev =  INT_MAX          /* only relevant for squash case */
   ;  dim len  =  strlen(src)
   ;  u32* tbl =  tr->tlt

   ;  for (i=0,j=0;i<len;i++)
      {  u8  idx     = *(src+i)
      ;  u32 tok     =  tbl[idx]
      ;  u8 idxmap   =  tok & 0377
      ;  u8 val      =  0

      ;  val = (tok >> 8) & MCX_TR_TRANSLATE ? idxmap : idx

      ;  if ((tbl[val] >> 8) & MCX_TR_DELETE)
         continue

      ;  if (!((tbl[val] >> 8) & MCX_TR_SQUASH) || val != prev)
         {  *(src+j++) =  (char) val
         ;  prev = val
      ;  }
      }
      *(src+j) = '\0'
   ;  return (ofs) j
;  }


ofs mcxTingTranslate
(  mcxTing* src
,  mcxTR*   tr
)
   {  src->len  =  mcxTRtranslate(src->str, tr)
   ;  return src->len
;  }



/* fixme conceivably mcxTRtranslate could return -1 as
 * error condition. hum ho.
*/
ofs mcxTingTr
(  mcxTing*       txt
,  const char*    src
,  const char*    dst
,  const char*    set_delete
,  const char*    set_squash
,  mcxbits        modes
)
   {  mcxTR tr
   ;  if (mcxTRloadTable(&tr, src, dst, set_delete, set_squash, modes))
      return -1
   ;  txt->len = mcxTRtranslate(txt->str, &tr)
   ;  return txt->len
;  }


static const char* xtr_get_token
(  const char *p
,  const char *z
,  u8* classp
,  u8* valuep
,  int *repeat
)
#define MCX_HEX(c)   (  (c>='0' && c<='9') ? c-'0'\
                     :  (c>='a' && c<='f') ? c-'a'+10\
                     :  (c>='A' && c<='F') ? c-'A'+10\
                     :  0\
                     )
   {  int c = UC(p), d = 0, np = 0
   ;  mcxstatus status = STATUS_FAIL
   ;  i32 value = 0, class = 0         /* need to be able to check overflow */
   ;  const char* me = "xtr_" "get_token"

   ;  mcxbool class_ok  = classp ? TRUE : FALSE

   ;  if (classp)
      *classp = 0
   ;  if (valuep)
      *valuep = 0
   ;  if (repeat)
      *repeat = -1

   ;  while (1)
      {  if (p >= z)
         break

   /*************************************************************************/

      ;  if (c == '\\')
         {  if (p+1 >= z)                          error_break
         ;  d = UC(p+1)

         ;  if (strchr("0123", d))
            {  if
               (  p+3 >=z
               || !isdigit(UC(p+2))
               || !isdigit(UC(p+3))          /* fixme 009 */
               )
               {  mcx_tr_err = X_OCTAL_SYNTAX
               ;  error_break
            ;  }

               value = 64*(d-'0') + 8*(UC(p+2)-'0') + (UC(p+3)-'0')

            ;  if (value > 0377)
               {  mcx_tr_err = X_OCTAL_RANGE
               ;  error_break
            ;  }
               np = 4
         ;  }
            else if (d == 'x')
            {  if
               (  p+3 >= z
               || !isxdigit(UC(p+2))
               || !isxdigit(UC(p+3))
               )
               {  mcx_tr_err = X_HEX_SYNTAX
               ;  error_break
            ;  }

               value = 16 * MCX_HEX(UC(p+2)) +  MCX_HEX(UC(p+3))
            ;  np = 4
         ;  }
            else if (strchr("\\abfnrtv", d))
            {  np = 2
            ;  switch(d)
               {  case 'a' : value = 07  ;  break
               ;  case 'b' : value = 010 ;  break
               ;  case 'f' : value = 014 ;  break
               ;  case 'n' : value = 012 ;  break
               ;  case 'r' : value = 015 ;  break
               ;  case 't' : value = 011 ;  break
               ;  case 'v' : value = 013 ;  break
               ;  case '\\': value = 0134;  break
               ;  default  : value = '?' ;  break    /* impossible */
            ;  }
            }
            else
            {  value= d
            ;  np = 2
         ;  }
         }  /* case '\\' */

   /*************************************************************************/

         else if (c == '[')
         {  if (!class_ok)                /* fixme: could just accept '[' */
            {  mcx_tr_err = X_CLASS_UNEXPECTED
            ;  error_break
         ;  }

         /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

            else if (p+1 < z && UC(p+1) == ':')
            {  if (!strncmp(p+2, "alpha:]", 7))  class = C_ALPHA,  np = 4+ 5
      ;   else if (!strncmp(p+2, "alnum:]", 7))  class = C_ALNUM,  np = 4+ 5
      ;   else if (!strncmp(p+2, "digit:]", 7))  class = C_DIGIT,  np = 4+ 5
      ;   else if (!strncmp(p+2, "cntrl:]", 7))  class = C_CNTRL,  np = 4+ 5
      ;   else if (!strncmp(p+2, "graph:]", 7))  class = C_GRAPH,  np = 4+ 5
      ;   else if (!strncmp(p+2, "lower:]", 7))  class = C_LOWER,  np = 4+ 5
      ;   else if (!strncmp(p+2, "print:]", 7))  class = C_PRINT,  np = 4+ 5
      ;   else if (!strncmp(p+2, "punct:]", 7))  class = C_PUNCT,  np = 4+ 5
      ;   else if (!strncmp(p+2, "space:]", 7))  class = C_SPACE,  np = 4+ 5
      ;   else if (!strncmp(p+2, "upper:]", 7))  class = C_UPPER,  np = 4+ 5
      ;   else if (!strncmp(p+2, "xdigit:]", 8)) class = C_XDIGIT, np = 4+ 6
      ;        else
               {  mcx_tr_err = X_CLASS_UNKNOWN
               ;  break
            ;  }
            }

         /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

            else if (p+1 < z && UC(p+1) == '*')
            {  int num = 0
            ;  u8 value2 = 0
            ;  const char* q = NULL
            ;  if (p+2 >= z)
               {  mcx_tr_err = X_REPEAT_SYNTAX
               ;  error_break
            ;  }

               if (!(q = xtr_get_token(p+2, z, NULL, &value2, NULL)))
               error_break

            ;  if (UC(q) == '*' && UC(q+1) == ']')
               class = C_FLOOD
            ;  else if
               (  (UC(q) == '#' && UC(q+1) == ']')
               || (UC(q) == '*')
               )
               class = C_REPEAT
            ;  else
               {  mcx_tr_err = X_REPEAT_SYNTAX
               ;  error_break
            ;  }

               if (UC(q+1) == ']')
               {  *repeat = 0
               ;  q++
            ;  }
               else
               {  if (sscanf(q+1, "%d]", &num) != 1)  break
               ;  if (!(q = strchr(q, ']')))          break
               ;  if (num < 0 || num > 256)           break
               ;  *repeat = num
               ;  if (!num)
                  class = C_FLOOD
            ;  }
                  /* q[0] should be ']' now */

               value =  value2
            ;  np    =  (q+1) - p         /* truncintok */
         ;  }

         /* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

            else                                      break
      ;  }

   /*************************************************************************/

         else
         {  value = c
         ;  np  = 1
      ;  }

         status = STATUS_OK
      ;  break
   ;  }

      if (status)
      {  mcxErr(me, "error at char (%c)", UC(p))
      ;  return NULL
   ;  }
      
if (mcx_tr_debug)
fprintf(stdout, "xtr_get_token (%d | %d)\n", (int) value, (int) class)

   ;  if (classp)
      *classp = class
   ;  if (valuep)
      *valuep = value

   ;  return p + np
;  }



char* mcxStrEscapedValue
(  const char* p
,  const char* z
,  int   *value
)
   {  u8 val = p[0]
   ;  const char* q = NULL
   ;  if (val != '\\')
      {  *value = val
      ;  return (char*) (p+1)
   ;  }
      if (!(q = xtr_get_token(p, z, NULL, &val, NULL)))
      return NULL
   ;  *value = val
   ;  return (char*) q
;  }



/* Sets either
 *    value to something in the range 0-255 and length * to something >= 1
 * or
 *    value to one of the constants C_ALPHA etc and length to 0.
 *
 * Returns the next parsable character or NULL for failure.
*/

static const char* xtr_get_set
(  const char *p
,  const char *z
,  u8  *classp
,  u8  *valuep
,  int *countp
)
   {  const char* q  =  xtr_get_token(p, z, classp, valuep, countp)

   ;  if (!q)
      return NULL

   ;  if (!*classp && *q == '-')
      {  u8 range_top = 0
      ;  q = xtr_get_token(q+1, z, NULL, &range_top, NULL)

      ;  if (!q)
         return NULL

      ;  if (range_top < *valuep)
         {  mcx_tr_err = X_RANGE_INVERTED
         ;  return NULL
      ;  }
         *countp = range_top - *valuep + 1
   ;  }
      else if (!*classp)
      *countp = 1

   /* else it is a repeat class or .. */

   ;  return q
;  }


static void mcx_tr_enc_dump
(  u32* enc
,  const char* tag
)
   {  int i = 0
   ;  fprintf(stdout, "(dumping %s\n", tag)
   ;  while (i<MAX_SPEC_INDEX+1)
      {  int meta = enc[i] >> 8
      ;  int val  = enc[i] & 0377
      ;  fprintf(stdout, "%6d%6d%6d\n", i, meta, val)
      ;  if (meta == C_EOF)
         break
      ;  i++
   ;  }
      fprintf(stdout, ")\n")
;  }



static mcxstatus xtr_get_spec
(  const char* spec
,  u32*        tbl         /* size MAX_SPEC_INDEX+1 */
,  mcxbool     repeat_ok   /* doubles as true for dst, false for src, ugly! */
)
   {  const char* p  =  spec
   ;  const char* z  =  spec + strlen(spec)
   ;  int offset=0, i=0
   ;  mcxstatus status = STATUS_FAIL

   ;  while (1)
      {  u8 value    =  0
      ;  u8 class    =  0
      ;  int count   = -1

      ;  status      =  STATUS_OK
      ;  if (p >= z)
         break
      ;  status = STATUS_FAIL

                              /* space for 1 token; check anything longer.
                               * (reserve space for EOF token)
                              */
      ;  if (offset >= MAX_SPEC_INDEX)
         break
      ;  if (!(p = xtr_get_set(p, z, &class, &value, &count)))
         break

      ;  if (ISREPEAT(class) && count >= 0)
         {  if (!repeat_ok)
            {  mcx_tr_err = X_REPEAT_UNEXPECTED
            ;  break
         ;  }
            tbl[offset++] = (class << 8) | value
         ;  if (offset >= MAX_SPEC_INDEX)
            break
         ;  tbl[offset++] = count & 0377
      ;  }
         else if (ISCLASS(class))
         {  int (*innit)(int) =  isit[class - C_CLASSSTART]
         ;  tbl[offset++]        =  (C_CLASSSTART << 8 ) | class

         ;  for (i=0;i<256;i++)
            {  if (innit(i))
               {  if (offset >= MAX_SPEC_INDEX)
                  break
               ;  tbl[offset++] = i
            ;  }
            }
            if (i != 256)
            break
         ;  if (offset >= MAX_SPEC_INDEX)
            break
         ;  tbl[offset++] = (C_CLASSEND << 8) | class
      ;  }
         else if (count > 1)
         {  tbl[offset++] = (C_RANGESTART << 8)
         ;  for (i=value;i<value+count;i++)
            {  if (offset >= MAX_SPEC_INDEX)
               break
            ;  tbl[offset++] = i
         ;  }
            if (i != value+count)
            break
         ;  if (offset >= MAX_SPEC_INDEX)
            break
         ;  tbl[offset++] = C_RANGEEND << 8
      ;  }
         else
         tbl[offset++] = value

      ;  status = STATUS_OK
   ;  }


      tbl[offset++] = C_EOF << 8

   ;  if (status || p!=z)
      {  mcxErr(us, "error!")
      ;  return STATUS_FAIL
   ;  }

      return STATUS_OK
;  }


static mcxstatus mcx_tr_encode_boolean
(  mcxTR*   tr
,  u32*     enc
,  mcxbits  bit
)
   {  int i
   ;  for (i=0;i<MAX_SPEC_INDEX;i++)
      {  int class = enc[i] >> 8
      ;  int value = enc[i] & 0377

      ;  if (class == C_EOF)
         break
      ;  if (class)
         continue

      ;  tr->tlt[value] |= bit << 8
   ;  }
      return STATUS_OK
;  }


static mcxstatus mcx_tr_translate_encode
(  mcxTR* tr
,  u32*   srcenc
,  u32*   dstenc
)
   {  int X = -1, Y = -1
   ;  int s = 0, d = 0
   ;  int star_count = 0
   ;  mcxbool
         star_fill   =  FALSE
      ,  flood_fill  =  FALSE
      ,  src_end     =  FALSE
      ,  dst_end     =  FALSE
      ,  to_lower    =  FALSE
      ,  to_upper    =  FALSE
      ,  src_class_end = FALSE
 
   ;  while (1)
      {  u32 src_tok = 0 ;  u32 src_cls = 0 ;  u32 src_val = 0
      ;  u32 dst_tok = 0 ;  u32 dst_cls = 0 ;  u32 dst_val = 0
      ;  src_class_end = FALSE

      ;  if (s >= MAX_SPEC_INDEX || d >= MAX_SPEC_INDEX)
         {  mcxErr(us, "panic off the rails")
         ;  break
      ;  }

         if (X >= 0 && Y >= 0)
         {  tr->tlt[X] = Y | (MCX_TR_TRANSLATE << 8)
;if (mcx_tr_debug) fprintf(stdout, "map %d to %d\n", X, Y)
         ;  X = -1
         ;  if (star_count)
            star_count--
         ;  if (!star_count && !star_fill && !flood_fill)
            Y = -1
      ;  }
         else if (s || d)
         mcxErr(us, "fimbly feeling")

/* skip ends, unset star_fill if necessary */
      ;  src_tok = srcenc[s++]
      ;  src_cls = src_tok >> 8
      ;  src_val = src_tok & 0377

/* consider both
*    tring -x 'abcdef[:digit:]' -y '[*_#][*##]'   (magic ends with f)
*    tring -x '[:digit:]abcdef' -y '[*##][*_#]'     (magic ends before a)
*/
      ;  if (ISBOUNDARY(src_cls))
         star_fill = FALSE

      ;  if (ISEND(src_cls))
            src_tok =   srcenc[s++]
         ,  src_cls =   src_tok >> 8
         ,  src_val =   src_tok & 0377
         ,  src_class_end =  TRUE
      
      ;  if (!star_count && !star_fill && !flood_fill)
         {  dst_tok = dstenc[d++]
         ;  dst_cls = dst_tok >> 8
         ;  dst_val = dst_tok & 0377

         ;  if (ISEND(dst_cls))
               dst_tok = dstenc[d++]
            ,  dst_cls = dst_tok >> 8
            ,  dst_val = dst_tok & 0377
      ;  }

;if (mcx_tr_debug)
fprintf(stdout, "have %3d %3d   %3d %3d\n",
(int) src_cls, (int) src_val, (int) dst_cls, (int) dst_val);

/* check class/range conditions.  */
         if (ISLOWERSTART(src_tok) && ISUPPERSTART(dst_tok))
         to_upper = TRUE

      ;  else if (ISUPPERSTART(src_tok) && ISLOWERSTART(dst_tok))
         to_lower = TRUE

      ;  else if ((to_upper || to_lower) && src_class_end)
            to_lower = FALSE
         ,  to_upper = FALSE

/* check repeat */
      ;  if (ISREPEAT(dst_cls))
         {  Y = dst_val
         ;  star_count = dstenc[d++]

         ;  if (dst_cls == C_FLOOD)
            flood_fill = TRUE
         ;  else if (!star_count)
            star_fill = TRUE
;if (mcx_tr_debug)
fprintf(stdout, "star count/fill/flood %d %d %d\n", star_count, star_fill, flood_fill)
      ;  }

;if (mcx_tr_debug && (to_upper || to_lower)) fprintf(stdout, "case mapping\n")

      ;  if (!star_count && !star_fill && !flood_fill)
         {  if (ISSTART(dst_cls))
            Y = dstenc[d++]
         ;  else
            Y = dst_val
      ;  }

         if (ISSTART(src_cls))
         X = srcenc[s++]
      ;  else
         X = src_val

      ;  if (!ISUCHARRANGE(X) || !ISUCHARRANGE(Y))
            mcxErr(us, "panic %d %d", X, Y)
         ,  X = 0
         ,  Y = 0

      ;  if (to_lower)
         {  if (!isupper(X))
            {  mcxErr(us, "panic %d not lower", X)
            ;   X = 0, Y = 0
         ;  }
            else
            Y = tolower(X)
      ;  }
         else if (to_upper)
         {  if (!islower(X))
            {  mcxErr(us, "panic %d not upper", X)
            ;  X = 0, Y = 0
         ;  }
            else
            Y = toupper(X)
      ;  }

         src_end = src_cls == C_EOF
      ;  dst_end = dst_cls == C_EOF

      ;  if (src_end || dst_end)
         break
   ;  }

      if (!src_end)
      mcxErr(us, "trailing fluff in src")
   ;  if (!dst_end && !((star_fill || flood_fill) && dstenc[d] >> 8 == C_EOF))
      mcxErr(us, "trailing fluff in dst")

   ;  return STATUS_OK        /* specs parsed alright, just ignore spurious stuff */
;  }


static void mcx_tr_complement
(  u32* enc
)
   {  u32 tmp[256]
   ;  int i = 0
   ;  int j = 0
   ;  int delta = 1

   ;  for (i=0;i<256;i++)
      tmp[i] = 0

   ;  for (i=0;i<=MAX_SPEC_INDEX;i+=delta)
      {  int class = enc[i] >> 8
      ;  int value = enc[i] & 0377
      ;  if (ISREPEAT(class))
         {  delta = 2
         ;  mcxErr(us, "value taken but repeat ignored in complement")
         ;  tmp[value] = 1
         ;  continue
      ;  }
         else if (ISBOUNDARY(class))
      /* nada
      */
      ;  else
         tmp[value] = 1
      ;  delta = 1
   ;  }

      for (i=0;i<256;i++)
      if (!tmp[i])
      enc[j++] = i

   ;  enc[j++] = C_EOF << 8

   ;  while (j<=MAX_SPEC_INDEX)
      enc[j++] = C_EOF << 8
;  }



/*
 *    the enc arrays use stop and end tokens for ranges and classes.  both are
 *    required to have more than 1 element.  No more than MAX_SPEC_INDEX+1
 *    positions are allocated for either specification, which also cater for
 *    start and end and EOF tokens - the +1 is for the EOF token.
 *
 *    carryon
*/

static mcxstatus mcx_tr_encode
(  mcxTR* tr
,  const char* src
,  const char* dst
,  const char* set_delete
,  const char* set_squash
)
   {  u32 srcenc[MAX_SPEC_INDEX+1]
   ;  u32 dstenc[MAX_SPEC_INDEX+1]
   ;  int i

   ;  if (src && dst)
      {  for (i=0;i<=MAX_SPEC_INDEX;i++)
            srcenc[i] = 0
         ,  dstenc[i] = 0

      ;  if (xtr_get_spec(src, srcenc, FALSE))
         return STATUS_FAIL

      ;  if (mcx_tr_debug)
         mcx_tr_enc_dump(srcenc, "source")

      ;  if (tr->modes & MCX_TR_SOURCE_C)
         mcx_tr_complement(srcenc)

      ;  if (xtr_get_spec(dst, dstenc, TRUE))
         return STATUS_FAIL

      ;  if (mcx_tr_debug)
         mcx_tr_enc_dump(dstenc, "destination")

      ;  if (tr->modes & MCX_TR_DEST_C)
         mcx_tr_complement(dstenc)

      ;  if (mcx_tr_translate_encode(tr, srcenc, dstenc))
         return STATUS_FAIL
   ;  }

      if (set_delete)
      {  for (i=0;i<=MAX_SPEC_INDEX;i++)
         srcenc[i] = 0

      ;  if (xtr_get_spec(set_delete, srcenc, FALSE))
         return STATUS_FAIL

      ;  if (mcx_tr_debug)
         mcx_tr_enc_dump(srcenc, "delete")

      ;  if (tr->modes & MCX_TR_DELETE_C)
         mcx_tr_complement(srcenc)

      ;  mcx_tr_encode_boolean(tr, srcenc, MCX_TR_DELETE)
   ;  }

      if (set_squash)
      {  for (i=0;i<=MAX_SPEC_INDEX;i++)
         srcenc[i] = 0

      ;  if (xtr_get_spec(set_squash, srcenc, FALSE))
         return STATUS_FAIL

      ;  if (mcx_tr_debug)
         mcx_tr_enc_dump(srcenc, "squash")

      ;  if (tr->modes & MCX_TR_SQUASH_C)
         mcx_tr_complement(srcenc)

      ;  mcx_tr_encode_boolean(tr, srcenc, MCX_TR_SQUASH)
   ;  }

      return STATUS_OK
;  }



mcxstatus mcxTRloadTable
(  mcxTR*      tr
,  const char* src
,  const char* dst
,  const char* set_delete
,  const char* set_squash
,  mcxbits     modes
)
   {  const char* me =  "mcxTRloadTable"
   ;  int i
   ;  mcx_tr_err = NULL

   ;  if (src && UC(src) == '^')
         src++
      ,  modes |= MCX_TR_SOURCE_C

   ;  if (dst && UC(dst) == '^')
         dst++
      ,  modes |= MCX_TR_DEST_C

   ;  if (src && dst)
      modes |= MCX_TR_TRANSLATE

   ;  if (set_delete)
      {  if (UC(set_delete) == '^')
            set_delete++
         ,  modes |= MCX_TR_DELETE_C
      ;  modes |= MCX_TR_DELETE
   ;  }

   ;  if (set_squash)
      {  if (UC(set_squash) == '^')
            set_squash++
         ,  modes |= MCX_TR_SQUASH_C
      ;  modes |= MCX_TR_SQUASH
   ;  }

      tr->modes = modes

   ;  for (i=0;i<256;i++)
      tr->tlt[i] = 0

   ;  if (src && !dst)
      {  mcxErr(me, "src requires dst")
      ;  return STATUS_FAIL
   ;  }

      return mcx_tr_encode(tr, src, dst, set_delete, set_squash)
;  }


