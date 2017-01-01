/*   (C) Copyright 2005, 2006, 2007, 2008, 2009, 2010, 2011 Stijn van Dongen
 *   (C) Copyright 2012, 2013 Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/* TODO
 *    This is tricky code, way too sprawling, and too much interweaving
 *    of string-offset arithmetic and string parsing with label handling,
 *    domain checks, and flow control (stemming from e.g. IGNORE modes)
 *    from the restrict cases.
 *    
 *    Has to support many modes
 *    in terms of abc|etc|123|235 modes, extend|strict|restrict,
 *    cmax|rmax symmetric|directed.
 *    With -restrict-tabr?, what are the domains of the resulting matrix?
 *
 *    err if *ai and tab_col_in both present.
 *    _AI_ map logic was a bit bolted on. go over it and enforce clear logic.
 *
 *    handle_label could first look for labels. only insert if
 *    necessary (so no deletion is needed) [but I might have forgotten
 *    a particular snag with this].
 *
 *    max_seen update could be factored out of read_abc, read_etc,
 *    and handle_label.
 *
 *    with 123 data, one might want to enable domains.
 *
 *    mclxIOstreamIn and its descendants should not die
 *    but respect ON_FAIL
 *
 *    Code not good, but has not failed in a long time.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>

#include "stream.h"
#include "vector.h"
#include "iface.h"

#include "util/compile.h"
#include "util/types.h"
#include "util/err.h"
#include "util/minmax.h"
#include "util/alloc.h"
#include "util/ting.h"
#include "util/ding.h"
#include "util/io.h"
#include "util/hash.h"
#include "util/array.h"

const char* module = "mclxIOstreamIn";

#define MCLXIO_STREAM_CTAB (MCLXIO_STREAM_CTAB_EXTEND | MCLXIO_STREAM_CTAB_STRICT | MCLXIO_STREAM_CTAB_RESTRICT)
#define MCLXIO_STREAM_RTAB (MCLXIO_STREAM_RTAB_EXTEND | MCLXIO_STREAM_RTAB_STRICT | MCLXIO_STREAM_RTAB_RESTRICT)

#define MCLXIO_STREAM_CTAB_RO (MCLXIO_STREAM_CTAB_STRICT | MCLXIO_STREAM_CTAB_RESTRICT)
#define MCLXIO_STREAM_RTAB_RO (MCLXIO_STREAM_RTAB_STRICT | MCLXIO_STREAM_RTAB_RESTRICT)

#define MCLXIO_STREAM_ETCANY (MCLXIO_STREAM_ETC | MCLXIO_STREAM_ETC_AI | MCLXIO_STREAM_SIF)
#define MCLXIO_STREAM_235ANY (MCLXIO_STREAM_235 | MCLXIO_STREAM_235_AI)

#define DEBUG  0
#define DEBUG2 0
#define DEBUG3 0


#if 0                   /* for packed machine format, one day */
static mcxstatus write_u32_be
(  unsigned long l
,  FILE* fp
)
   {  int i = 0
   ;  u8 bytes[4]

   ;  for (i=0;i<4;i++)
      {  bytes[3-i] = l & 0377   /* least significant last */
      ;  l >>= 8
   ;  }

      fwrite(&bytes, sizeof bytes[0], 4, fp)
   ;  return STATUS_OK
;  }


static mcxstatus read_u32_be
(  unsigned long *lp
,  FILE* fp
)
   {  int i = 0
   ;  u8 bytes[4]
   ;  unsigned long idx = 0

   ;  *lp = 0
   ;
      {  if (fread(bytes, sizeof bytes[0], 4, fp) != 4)
         return STATUS_FAIL
      ;  for (i=0;i<4;i++)
         idx += bytes[3-i] << (8*i) 
   ;  }

      *lp = idx
   ;  return STATUS_OK
;  }
#endif



/*
 *    fixme: improve documentation, organization
 *    possibly implement ':value'
 *
 *    Returns IGNORE, FAIL or DONE
*/

typedef struct
{  mcxTing* etcbuf
;  dim      etcbuf_check
;  dim      etcbuf_ofs
;  unsigned long x_prev       /* NOTE we depend on unsigned wrap-around */
;  dim      n_y
;
}  etc_state;


typedef struct
{  mcxHash*    map
;  mclTab*     tab
;  long        max_seen
;  ulong       n_seen
;
}  map_state  ;


typedef struct
{  map_state* map_c
;  map_state* map_r
;  ulong       x
;  ulong       y              /* note: can be same as x  */
;  mcxstatus   statusx
;  mcxstatus   statusy
;  mcxbits     bits

;  mclpAR*     pars
;  dim         pars_n_alloc
;  dim         pars_n_used
;
}  stream_state   ;


/* *keypp; if we free it caller can still free it as well
 * (because *keypp is set to NULL).
 *
 * If handle_label fills z, this means (as a programmer's contract) that *z
 * should always be used by the caller. This is a tight coupling necessitated
 * by the internal implementation of the stream interface.
 * may return:

 *    STATUS_OK
      STATUS_NEW
      STATUS_IGNORE
      STATUS_FAIL
*/

static mcxstatus handle_label
(  mcxTing**      keypp
,  unsigned long* z
,  map_state*     map_z
,  mcxbits        bits
,  const char*    mode
)
   {  mcxbool strict =  bits & MCLXIO_STREAM_GTAB_STRICT
   ;  mcxbool warn   =  bits & MCLXIO_STREAM_WARN
   ;  mcxbool ro     =  bits & (MCLXIO_STREAM_GTAB_STRICT | MCLXIO_STREAM_GTAB_RESTRICT)
   ;  mcxbool debug  =  bits & MCLXIO_STREAM_DEBUG

   ;  mcxstatus status = STATUS_OK
   ;  mcxKV* kv = mcxHashSearch(*keypp, map_z->map, ro ? MCX_DATUM_FIND : MCX_DATUM_INSERT)

   ;  if (!kv)      /* ro and not found */
      {  if (strict)
         {  mcxErr
            (module, "label <%s> not found (%s strict)", (*keypp)->str, mode)
         ;  status = STATUS_FAIL
      ;  }
         else
         {  if (warn)  /* restrict */
            mcxTell
            (module, "label <%s> not found (%s restrict)", (*keypp)->str, mode)
         ;  status = STATUS_IGNORE
      ;  }
      }
      else if (kv->key != *keypp)         /* seen */
      {  mcxTingFree(keypp)
      ;  *z = VOID_TO_ULONG kv->val     /* fixme theoretical signedness issue */
   ;  }
      else                             /* INSERTed */
      {  if (debug)
         mcxTell
         (  module
         ,  "label %s not found (%s extend %lu)"
         ,  (*keypp)->str
         ,  mode
         ,  (map_z->max_seen + 1)
         )
      ;  map_z->n_seen++
      ;  map_z->max_seen++
      ;  kv->val = ULONG_TO_VOID map_z->max_seen
      ;  *z = map_z->max_seen           /* fixme theoretical signedness issue */
;if(DEBUG3)fprintf(stderr, "hl insert %s <%s> to %d\n", mode, keypp[0]->str, (int) *z)
      ;  status = STATUS_NEW
   ;  }

;if(DEBUG2)fprintf(stderr, "hl final map to %d\n", (int) *z)
   ;  return status
;  }


   /* Purpose: read a single x/y combination. The x may be cached
    * due to the etc format, where a single line always refers to the same x
    * and that x is listed only at the start or line, or omitted with
    * the etc-ai and 235-ai formats.
    *
    * state->x_prev may be used by read_etc in order to obtain the
    * current x index.
   */
static mcxstatus read_etc
(  mcxIO*         xf
,  stream_state  *iface
,  etc_state     *state
,  double*        value
)
   {  mcxbits bits      =  iface->bits
   ;  FILE* stdbug      =  stdout

   ;  mcxstatus status  =  STATUS_OK
   ;  mcxTing* ykey     =  NULL
   ;  mcxTing* xkey     =  NULL
   ;  const char* printable

   ;  mcxbool label_cbits = bits & (MCLXIO_STREAM_CTAB_STRICT | MCLXIO_STREAM_CTAB_RESTRICT)
   ;  mcxbool label_rbits = bits & (MCLXIO_STREAM_RTAB_STRICT | MCLXIO_STREAM_RTAB_RESTRICT)
   ;  mcxbool label_dbits = bits & (MCLXIO_STREAM_WARN | MCLXIO_STREAM_DEBUG)
   ;  mcxbool tryvalue    = bits & MCLXIO_STREAM_EXPECT_VALUE

   ;  iface->statusx    =  STATUS_OK
   ;  iface->statusy    =  STATUS_OK

   ;  iface->x =  state->x_prev
   ;  *value   =  1.0

;if(DEBUG)fprintf(stdbug, "read_etc initially set x to %d\n", (int) iface->x)

   ;  if (!state->etcbuf)
      state->etcbuf = mcxTingEmpty(NULL, 100)

   ;  do
      {  int n_char_read = 0
      ;  if (state->etcbuf->len != state->etcbuf_check)
         {  mcxErr
            (  module
            ,  "read_etc sanity check failed %ld %ld"
            ,  (long) state->etcbuf->len
            ,  (long) state->etcbuf_check
            )
         ;  status = STATUS_FAIL
         ;  break
      ;  }

               /* we need to read */
                     /* do we need to read a line ?   */
                     /* -> then set iface->x          */
               /* fixmefixme: funcify this */
               /* iface->x can only be changed in this branch */
/* ************************************************************************** */
         if (state->etcbuf_ofs >= state->etcbuf->len)
         {  state->etcbuf_ofs  = 0
         ;  state->n_y = 0
         ;  if ((status = mcxIOreadLine(xf, state->etcbuf, MCX_READLINE_CHOMP)))
            break
         ;  state->etcbuf_check = state->etcbuf->len

         ;  if
            (  !(printable = mcxStrChrAint(state->etcbuf->str, isspace, -1))
            || (unsigned char) *printable == '#'
            )
            {  state->etcbuf_ofs = state->etcbuf->len
            ;  iface->statusy = STATUS_IGNORE
            ;  break    /* fixme: ^ statusx seems to work as well. cleanify design */
         ;  }

         ;  if (bits & (MCLXIO_STREAM_ETC_AI | MCLXIO_STREAM_235_AI))
            {
            }
                     /* In this branch we do not issue handle_label, so we take care of max_seen.
                     */
            else if (bits & MCLXIO_STREAM_235)
            {  if (1 != sscanf(state->etcbuf->str, "%lu%n", &(iface->x), &n_char_read))
               {  iface->statusx = STATUS_FAIL  
               ;  break
            ;  }
               state->etcbuf_ofs += n_char_read
            ;  if (iface->map_c->max_seen+1 < iface->x+1)      /* note mixed-sign comparison */
               iface->map_c->max_seen = iface->x
            ;  state->x_prev = iface->x
         ;  }
            else if (bits & (MCLXIO_STREAM_ETC | MCLXIO_STREAM_SIF))
            {  xkey = mcxTingEmpty(NULL, state->etcbuf->len)
            ;  if (1 != sscanf(state->etcbuf->str, "%s%n", xkey->str, &n_char_read))
               break
            ;  state->etcbuf_ofs += n_char_read
            ;  xkey->len = strlen(xkey->str)
            ;  xkey->str[xkey->len] = '\0'
;if(DEBUG3)fprintf(stderr, "max %lu\n", (ulong) iface->map_c->max_seen)
            ;  iface->statusx
                = handle_label(&xkey, &(iface->x), iface->map_c, label_cbits | label_dbits, "col")
;if(DEBUG3)fprintf(stderr, "max %lu x %lu\n", (ulong) iface->map_c->max_seen, (ulong) iface->x)
            ;  if (iface->statusx == STATUS_IGNORE || iface->statusx == STATUS_FAIL)
               {  /* iface->x = 141414  recentlyadded */
;if(DEBUG3)fprintf(stderr, "max %lu\n", (ulong) iface->map_c->max_seen)
               ;  break
            ;  }
                     /* ^ Consider what happens when we break here (x label not
                      * accepted) with map_c->max_seen.  Basically x label is
                      * indepedent of y, so we never need to undo the
                      * handle_label action.
                     */
               state->x_prev = iface->x
         ;  }
            else
            mcxDie(1, module, "strange, really")
      ;  }
/* ************************************************************************** */

         if
         ( !(  printable
            =  mcxStrChrAint(state->etcbuf->str+state->etcbuf_ofs, isspace, -1)
            )
         || (uchar) *printable == '#'
         )
         {  state->etcbuf_ofs = state->etcbuf->len
         ;  /* iface->y = 141414 recentlyadded */
         ;  iface->statusy = STATUS_IGNORE
         ;  break
      ;  }

         if (bits & (MCLXIO_STREAM_235_AI | MCLXIO_STREAM_235))
         {  if
            (  (  tryvalue
               && 2 != sscanf(state->etcbuf->str+state->etcbuf_ofs, "%lu:%lf%n", &(iface->y), value, &n_char_read)
               )
            || 1 != sscanf(state->etcbuf->str+state->etcbuf_ofs, "%lu%n", &(iface->y), &n_char_read)
            )
            {  char* s = state->etcbuf->str+state->etcbuf_ofs
            ;  while(isspace((uchar) s[0]))
               s++
            ;  mcxErr
               (  module
               ,  "unexpected string starting with <%c> on line %lu"
               ,  (int) ((uchar) s[0])
               ,  xf->lc
               )
            ;  iface->statusy = STATUS_FAIL
         ;  }
            else
            {
;if(DEBUG3)fprintf(stdbug, "hit at %d\n", (int) state->etcbuf_ofs);
               state->etcbuf_ofs += n_char_read
            ;  if (iface->map_r->max_seen+1 < iface->y+1)      /* note mixed-sign comparison */
               iface->map_r->max_seen = iface->y
         ;  }
         }
         else  /* ETCANY, including SIF */
         {  ykey = mcxTingEmpty(NULL, state->etcbuf->len)
         ;  if
            (  (  !tryvalue
               || 2 != sscanf(state->etcbuf->str+state->etcbuf_ofs, " %[^:\t ]:%lf%n", ykey->str, value, &n_char_read)
               )     /* ^ failure is true if !tryvalue or failed sscanf */
              && 1 != sscanf(state->etcbuf->str+state->etcbuf_ofs, "%s%n", ykey->str, &n_char_read)
            )
            {  mcxErr(module, "unexpected input [%s] on line %lu", state->etcbuf->str+state->etcbuf_ofs, xf->lc)
            ;  iface->statusy = STATUS_FAIL
            ;  break
         ;  }

            ykey->len = strlen(ykey->str)
         ;  ykey->str[ykey->len] = '\0'
         ;  state->etcbuf_ofs += n_char_read
         ;  if (state->n_y == 0 && (bits & MCLXIO_STREAM_SIF))
               iface->statusy = STATUS_IGNORE
            ,  state->n_y++
         ;  else
            iface->statusy =  handle_label(&ykey, &(iface->y), iface->map_r, label_rbits | label_dbits, "row")
      ;  }

                  /* this won't scale well in terms of organisation if  and when
                   * tabs are allowed with 235 mode, because in that case,
                   * with 235-ai and restrict-tabr and extend-tabc we will
                   * need the stuff below duplicated in the 235 branch above.

                   * what happens here is that we only decide now whether
                   * the auto-increment is actually happening. It depends
                   * on there being at least one y that was not rejected.
                   * ^ that's a design decision. the right one? dunno, probably not :-(
                  */
      ;  if
         (  (bits & (MCLXIO_STREAM_ETC_AI | MCLXIO_STREAM_235_AI))
         && (iface->statusy == STATUS_OK || iface->statusy == STATUS_NEW)
         && !state->n_y
         )
         {  iface->x = state->x_prev + 1         /* works first time around */
         ;  iface->map_c->max_seen = state->x_prev + 1
         ;  state->n_y++
         ;  state->x_prev = iface->x
      ;  }

;if(DEBUG2)fprintf(stdbug, "etc handle label we have y %d status %s\n", (int) iface->y, MCXSTATUS(iface->statusy))
;     }
      while (0)

;if(DEBUG2)fprintf(stdbug, "status %s\n", MCXSTATUS(status))
   ;  do
      {  if (status)    /* e.g. STATUS_DONE (readline) [or STATUS_IGNORE (#)]*/
         break

                  /* below iface->statusy == STATUS_NEW should be impossible
                   * given this clause and the code sequence earlier.
                  */
      ;  if (iface->statusx == STATUS_FAIL || iface->statusx == STATUS_IGNORE)
         {  mcxTingFree(&xkey)
         ;  status = iface->statusx
         ;  break
      ;  }

                  /* case iface->statusx == STATUS_NEW is *always* honored
                  */
      ;  if (iface->statusy == STATUS_FAIL || iface->statusy == STATUS_IGNORE)
         {  mcxTingFree(&ykey)
         ;  status = iface->statusy
         ;  break
      ;  }
      }
      while (0)

   ;  if (status == STATUS_IGNORE || status == STATUS_FAIL)
      mcxTingFree(&ykey)

                  /* fixme, the action in this branch is done in other places too. cleanify design */
   ;  if
      (  iface->statusx == STATUS_IGNORE
      || !mcxStrChrAint(state->etcbuf->str+state->etcbuf_ofs, isspace, -1)
      )
      state->etcbuf_ofs = state->etcbuf->len

;if(DEBUG3)fprintf
( stdbug, "read_etc %s return x(%s -> %d stat=%s) y(%s -> %d stat=%s) status %s buf %d %d c_max_seen %lu\n"
,  MCXSTATUS(status)
, (xkey ? xkey->str : "-"), (int) iface->x, MCXSTATUS(iface->statusx)
, (ykey ? ykey->str : "-"), (int) iface->y, MCXSTATUS(iface->statusy)
, MCXSTATUS(status), (int) state->etcbuf->len, (int) state->etcbuf_ofs
,  (ulong) iface->map_c->max_seen
)
   ;  return status
;  }


static mcxstatus read_abc
(  mcxIO* xf
,  mcxTing* buf
,  stream_state *iface
,  double* value
)
   {  mcxstatus status  =  mcxIOreadLine(xf, buf, MCX_READLINE_CHOMP)
   ;  mcxTing* xkey     =  mcxTingEmpty(NULL, buf->len)
   ;  mcxTing* ykey     =  mcxTingEmpty(NULL, buf->len)
   ;  mcxbits bits      =  iface->bits

   ;  mcxbool strict    =  bits & MCLXIO_STREAM_STRICT
   ;  mcxbool warn      =  bits & MCLXIO_STREAM_WARN

   ;  mcxbool label_cbits = bits & (MCLXIO_STREAM_CTAB_STRICT | MCLXIO_STREAM_CTAB_RESTRICT)
   ;  mcxbool label_rbits = bits & (MCLXIO_STREAM_RTAB_STRICT | MCLXIO_STREAM_RTAB_RESTRICT)
   ;  mcxbool label_dbits = bits & (MCLXIO_STREAM_WARN | MCLXIO_STREAM_DEBUG)

   ;  const char* printable
   ;  int cv = 0

   ;  iface->statusx =  STATUS_OK
   ;  iface->statusy =  STATUS_OK

   ;  do
      {  int xlen = 0
      ;  int ylen = 0

      ;  if (status)
         break

      ;  printable = mcxStrChrAint(buf->str, isspace, buf->len)
      ;  if (printable && (uchar) printable[0] == '#')
         {  status = mcxIOreadLine(xf, buf, MCX_READLINE_CHOMP)
         ;  continue
      ;  }

         mcxTingEnsure(xkey, buf->len)    /* fixme, bit wasteful */
      ;  mcxTingEnsure(ykey, buf->len)    /* fixme, bit wasteful */

      ;  cv =     strchr(buf->str, '\t')
               ?  sscanf(buf->str, "%[^\t]\t%[^\t]%lf", xkey->str, ykey->str, value)
               :  sscanf(buf->str, "%s%s%lf", xkey->str, ykey->str, value)

      /* WARNING: [xy]key->len have to be set.
       * we first check sscanf return value
      */

      ;  if (cv == 2)
         *value = 1.0

      ;  else if (cv != 3)
         {  if (warn || strict)
            mcxErr
            (  module
            ,  "abc-parser chokes at line %ld [%s]"
            ,  (long) xf->lc
            ,  buf->str
            )
         ;  if (strict)
            {  status = STATUS_FAIL
            ;  break
         ;  }
            status = mcxIOreadLine(xf, buf, MCX_READLINE_CHOMP)
         ;  continue
      ;  }
         else if (!(*value <= FLT_MAX))  /* should catch nan, inf */
         *value = 1.0

      ;  xlen = strlen(xkey->str)
      ;  ylen = strlen(ykey->str)

      ;  xkey->len = xlen
      ;  ykey->len = ylen

      ;     status
         =  iface->statusx
         =  handle_label(&xkey, &(iface->x), iface->map_c, label_cbits | label_dbits, "col")

      ;  if (status == STATUS_FAIL || status == STATUS_IGNORE)
         break

      ;     status
         =  iface->statusy
         =  handle_label(&ykey, &(iface->y), iface->map_r, label_rbits | label_dbits, "row")
      ;  if (status == STATUS_FAIL || status == STATUS_IGNORE)
         break

      ;  status = STATUS_OK     /* Note: status can never be STATUS_NEW */
      ;  break
   ;  }
      while (1)

;if(DEBUG2) fprintf(stderr, "read_abc status %s\n", MCXSTATUS(status))
   ;  if (status == STATUS_NEW)
      mcxErr(module, "read_abc panic, because status == STATUS_NEW")

               /* Below we remove the key from the map if it should be
                * ignored. It will be freed in the block following this one.
               */
   ;  if
      (   iface->statusx == STATUS_NEW
      && (iface->statusy == STATUS_FAIL || iface->statusy == STATUS_IGNORE)
      )
      {  mcxHashSearch(xkey, iface->map_c->map, MCX_DATUM_DELETE)
      ;  iface->map_c->max_seen--
      ;  iface->statusx = STATUS_IGNORE
   ;  }
      else if   /* Impossible (given that we break when iface->statusx) but defensive */
      (   iface->statusy == STATUS_NEW
      && (iface->statusx == STATUS_FAIL || iface->statusx == STATUS_IGNORE)
      )
      {  mcxHashSearch(ykey, iface->map_r->map, MCX_DATUM_DELETE)
      ;  iface->map_r->max_seen--
      ;  iface->statusy = STATUS_IGNORE
   ;  }

         /* NOTE handle_label might have set either to NULL but
          * that's OK.  This is needed because handle_label(&xkey)
          * might succeed and free xkey (because already present in
          * map_c->map); then when handle_label(&ykey) fails we need to
          * clean up.
         */
   ;  if (status)
      {  mcxTingFree(&xkey)   /* kv deleted if iface->statusx == STATUS_NEW */
      ;  mcxTingFree(&ykey)   /* kv deleted if iface->statusy == STATUS_NEW */
   ;  }

      return status
;  }



static mcxstatus read_123
(  mcxIO* xf
,  mcxTing* buf
,  stream_state* iface
,  mclxIOstreamer* streamer
,  double* value
,  mcxbits bits
)
   {  mcxstatus status = mcxIOreadLine(xf, buf, MCX_READLINE_CHOMP)
   ;  int cv = 0
   ;  const char* printable
   ;  const char* me = module
   ;  mcxbool strict = bits & MCLXIO_STREAM_STRICT
   ;  mcxbool warn   = bits & MCLXIO_STREAM_WARN
   ;  unsigned long x = 0, y = 0

   ;  while (1)
      {  if (status)
         break

      ;  status = STATUS_FAIL

      ;  printable = mcxStrChrAint(buf->str, isspace, buf->len)
      ;  if (printable && (unsigned char) printable[0] == '#')
         {  status = mcxIOreadLine(xf, buf, MCX_READLINE_CHOMP)
         ;  continue
      ;  }

         cv = sscanf(buf->str, "%lu%lu%lf", &x, &y, value)

      ;  if (x > LONG_MAX || y > LONG_MAX)
         {  mcxErr
            (me, "negative values in input stream? unsigned %lu %lu", x, y)
         ;  break
      ;  }

         if (cv == 2)
         *value = 1.0

      ;  else if (cv != 3)
         {  if (strict || warn)
            mcxErr
            (  module
            ,  "123-parser chokes at line %ld [%s]"
            ,  (long) xf->lc
            ,  buf->str
            )
         ;  if (strict)
            break

         ;  status = mcxIOreadLine(xf, buf, MCX_READLINE_CHOMP)
         ;  continue
      ;  }
         else if (!(*value < FLT_MAX))
         *value = 1.0

      ;  if
         (  (streamer->cmax_123 && x >= streamer->cmax_123)
         || (streamer->rmax_123 && y >= streamer->rmax_123)
         )
         {  status = STATUS_IGNORE
         ;  break
      ;  }

         status = STATUS_OK
      ;  break
   ;  }

      if (!status)
      {  iface->x = x
      ;  iface->y = y
      ;  if (iface->map_c->max_seen+1 < x+1)    /* note mixed-sign comparison */
         iface->map_c->max_seen = x
      ;  if (iface->map_r->max_seen+1 < y+1)    /* note mixed-sign comparison */
         iface->map_r->max_seen = y
   ;  }

      return status
;  }


static void* my_par_init_v
(  void* arp_v
)
   {  mclpAR* ar = arp_v
   ;  mclpARinit(ar)
   ;  mclpARensure(ar, 10)
   ;  return ar
;  }


static mcxstatus pars_realloc
(  stream_state* iface
,  dim n_needed
)
   {  dim n_alloc = MCX_MAX(n_needed+8, iface->pars_n_alloc * 1.2)
   ;  mclpAR* p

   ;  if (n_needed <= iface->pars_n_alloc)
      {  if (n_needed > iface->pars_n_used)
         iface->pars_n_used = n_needed
      ;  return STATUS_OK
   ;  }

      p
      =  mcxNRealloc
         (  iface->pars
         ,  n_alloc
         ,  iface->pars_n_alloc
         ,  sizeof p[0]
         ,  my_par_init_v
         ,  RETURN_ON_FAIL
         )
;if(DEBUG3)fprintf(stderr, "realloc pars %lu (requested = %lu)\n", (ulong) n_alloc, (ulong) n_needed)
   ;  if (!p)
      {  mcxErr(module, "failure allocing p array (%lu units)", (ulong) n_alloc)
      ;  return STATUS_FAIL
   ;  }

      iface->pars = p
   ;  iface->pars_n_used  = n_needed
   ;  iface->pars_n_alloc = n_alloc
   ;  return STATUS_OK
;  }



               /* sets iface.map_c and iface.map_r
                * This includes
                *    map (hash)
                *    tab
                *    max_seen
               */

static void stream_state_set_map
(  mcxbool symmetric
,  stream_state *iface
,  mclxIOstreamer* streamer
,  mcxbits* bitsp
)
   {  mcxbits newbits = 0

   ;  if (symmetric)       /* work from input tab */
      {  iface->map_c->tab = streamer->tab_sym_in
      ;  if (iface->map_c->tab != NULL)
         {  iface->map_c->map = mclTabHash(iface->map_c->tab)
         ;  if (!(bitsp[0] & (MCLXIO_STREAM_CTAB | MCLXIO_STREAM_RTAB)))
            {  mcxErr(module, "PBD suggest explicit tab mode (now extending)")
            ;  newbits |= (MCLXIO_STREAM_CTAB_EXTEND | MCLXIO_STREAM_RTAB_EXTEND)
         ;  }
            iface->map_c->max_seen = MCLV_MAXID(iface->map_c->tab->domain)
      ;  }
         else
         {  iface->map_c->map = mcxHashNew(1024, mcxTingDPhash, mcxTingCmp)
      ;  }
         iface->map_r = iface->map_c
   ;  }
      else
      {  iface->map_c->tab = streamer->tab_col_in
      ;  if (streamer->tab_col_in != NULL)
         {  iface->map_c->map = mclTabHash(iface->map_c->tab)
         ;  if (!(bitsp[0] & MCLXIO_STREAM_CTAB))
            {  mcxErr(module, "PBD suggest explicit ctab mode (now extending)")
            ;  newbits |= MCLXIO_STREAM_CTAB_EXTEND
         ;  }
            iface->map_c->max_seen = MCLV_MAXID(iface->map_c->tab->domain)
      ;  }
         else if (!(bitsp[0] & (MCLXIO_STREAM_ETC_AI | MCLXIO_STREAM_235_AI)))
         iface->map_c->map = mcxHashNew(1024, mcxTingDPhash, mcxTingCmp)

      ;  iface->map_r->tab = streamer->tab_row_in
      ;  if (streamer->tab_row_in != NULL)
         {  iface->map_r->map = mclTabHash(iface->map_r->tab)
         ;  if (!(bitsp[0] & MCLXIO_STREAM_RTAB))
            {  mcxErr(module, "PBD suggest explicit rtab mode (now extending)")
            ;  newbits |= MCLXIO_STREAM_RTAB_EXTEND
         ;  }
            iface->map_r->max_seen = MCLV_MAXID(iface->map_r->tab->domain)
      ;  }
         else
         iface->map_r->map = mcxHashNew(1024, mcxTingDPhash, mcxTingCmp)
   ;  }

         /* alloc number below is some positive number if a restrict or extend tab was given
          * Otherwise it is 0.
         */
      pars_realloc(iface, (iface->map_c->max_seen + 1))
   ;  bitsp[0] |= newbits
;  }


static mclx* make_mx_from_pars
(  mclxIOstreamer* streamer
,  stream_state* iface
,  void  (*ivpmerge)(void* ivp1, const void* ivp2)
,  mcxbits bits
)
   {  mclpAR* pars = iface->pars
   ;  long dc_max_seen = iface->map_c->max_seen
   ;  long dr_max_seen = iface->map_r->max_seen
   ;  mclx* mx = NULL
   ;  mclv* domc, *domr
   ;  dim i

   ;  if (bits & MCLXIO_STREAM_235ANY)
      {  if (streamer->cmax_235 > 0 && dc_max_seen < streamer->cmax_235 - 1)
         dc_max_seen = streamer->cmax_235-1
   ;  }
      else if (bits & MCLXIO_STREAM_123)
      {  if (streamer->cmax_123 > 0 && dc_max_seen+1 < streamer->cmax_123)
         dc_max_seen = streamer->cmax_123-1
      ;  if (streamer->rmax_123 > 0 && dr_max_seen+1 < streamer->rmax_123)
         dr_max_seen = streamer->rmax_123-1
   ;  }

if(0)mcxTell("stream", "maxc=%d maxr=%d", (int) dc_max_seen, (int) dr_max_seen)

   ;  if (iface->pars_n_used != iface->map_c->max_seen+1)
      mcxDie
      (  1
      ,  module
      ,  "internal discrepancy: n_pars=%lu max_seen+1=%lu"
      ,  (ulong) iface->pars_n_used
      ,  (ulong) (iface->map_c->max_seen+1)
      )

   ;  if (dc_max_seen < 0 || dr_max_seen < 0)
      {  if (dc_max_seen < -1 || dr_max_seen < -1)
         {  mcxErr(module, "bad apples %ld %ld", dc_max_seen, dr_max_seen)
         ;  return NULL
      ;  }
         else
         mcxTell(module, "no assignments yield void/empty matrix")
   ;  }

                           /* fixme: with extend and same tab, should still copy.
                            * then, there are still occasions where one would
                            * want to go the sparse route
                           */
      domc  =     iface->map_c->tab && (iface->bits & MCLXIO_STREAM_CTAB_RO)
               ?  mclvClone(iface->map_c->tab->domain)
               :  mclvCanonical(NULL, dc_max_seen+1, 1.0)
   ;  domr  =     iface->map_r->tab && (iface->bits & MCLXIO_STREAM_RTAB_RO)
               ?  mclvClone(iface->map_r->tab->domain)
               :  mclvCanonical(NULL, dr_max_seen+1, 1.0)
      
   ;  if (! (mx = mclxAllocZero(domc, domr)))
      {  mclvFree(&domc)
      ;  mclvFree(&domr)
   ;  }
      else
      for (i=0;i<iface->pars_n_used;i++)  /* careful with signedness */
      {  long d = domc->ivps[i].idx
;if(DEBUG3)fprintf(stderr, "column %d alloc %d\n", (int) d, (int) iface->pars_n_alloc);
      ;  mclvFromPAR(mx->cols+i, pars+d, 0, ivpmerge, NULL)
   ;  }
      return mx
;  }


/* current, debatable, interface: if tab did not change return NULL.
*/

static mclTab* make_tab
(  map_state* map
)
   {  mclTab* tab = NULL
   ;  if (!map->tab || map->tab->domain->n_ivps < (dim) (map->max_seen+1))
      tab = mclTabFromMap(map->map)
   ;  else
      tab = map->tab
   ;  return tab
;  }


static void free_pars
(  stream_state* iface
)
   {  dim i
   ;  dim n_mem = 0, n_used = 0
   ;  for (i=0; i<iface->pars_n_alloc; i++)
         n_mem += iface->pars[i].n_alloc
      ,  n_used += iface->pars[i].n_ivps
      ,  mcxFree(iface->pars[i].ivps)
   ;  mcxFree(iface->pars)
   ;  iface->pars = NULL
;if(0)fprintf(stderr, "par allocated/used %u/%u ivps in %d buckets\n", (unsigned) n_mem, (unsigned) n_used, (int) iface->pars_n_alloc)
;  }


mclx* mclxIOstreamIn
(  mcxIO*   xf
,  mcxbits  bits
,  mclpAR*  transform
,  void    (*ivpmerge)(void* ivp1, const void* ivp2)
,  mclxIOstreamer* streamer
,  mcxOnFail ON_FAIL
)
   {  mcxstatus status  =  STATUS_FAIL
   ;  const char* me    =  module

   ;  mcxbool symmetric =  bits & MCLXIO_STREAM_SYMMETRIC
   ;  mcxbool mirror    =  bits & MCLXIO_STREAM_MIRROR

   ;  mcxbool abc       =  bits & MCLXIO_STREAM_ABC    ? TRUE : FALSE
   ;  mcxbool one23     =  bits & MCLXIO_STREAM_123    ? TRUE : FALSE
   ;  mcxbool etc       =  bits & (MCLXIO_STREAM_ETC | MCLXIO_STREAM_ETC_AI | MCLXIO_STREAM_SIF) ? TRUE : FALSE

   ;  mcxbool longlist  =  bits & (MCLXIO_STREAM_ETCANY | MCLXIO_STREAM_235ANY) ? TRUE : FALSE

   ;  mcxTing* linebuf  =  mcxTingEmpty(NULL, 100)

   ;  map_state  map_c  =  { NULL, NULL, -1 , 0}
   ;  map_state  map_r  =  { NULL, NULL, -1 , 0}

   ;  stream_state   iface
   ;  etc_state      etcstate

   ;  unsigned long n_ite = 0
   ;  mclx* mx = NULL

   ;  if (!ivpmerge)
      ivpmerge = mclpMergeMax

   ;  if (symmetric)
         iface.map_c = &map_c    /* this bit of hidgery-pokery       */
      ,  iface.map_r = &map_c    /* is a crucial interfacummathingy  */
   ;  else
         iface.map_c = &map_c
      ,  iface.map_r = &map_r

;if(DEBUG2)fprintf(stderr, "%s abc\n", abc ? "yes" : "no")
   ;  etcstate.etcbuf = NULL
   ;  etcstate.etcbuf_ofs = 0
   ;  etcstate.etcbuf_check = 0
   ;  etcstate.x_prev   =  ULONG_MAX      /* note we depend on ULONG_MAX + 1 == 0 */
   ;  etcstate.n_y   =  0

         /* fixme incomplete and distributed initialization of iface */
   ;  iface.pars = NULL
   ;  iface.pars_n_alloc = 0
   ;  iface.pars_n_used = 0

;if(DEBUG3)fprintf(stderr, "1 + max c %lu\n", (ulong) (iface.map_c->max_seen+1))
                                 /* fixme: put the block below in a subroutine */
   ;  while (1)
      {  if (abc + one23 + longlist > TRUE)   /* OUCH */
         {  mcxErr(module, "multiple stream formats specified")
         ;  break
      ;  }
         if (!symmetric && streamer->tab_sym_in)
         {  mcxErr(module, "for now disallowed, single tab, different domains")
         ;  break
      ;  }
         if ((!one23 && !abc && !longlist))
         {  mcxErr(module, "not enough to get going")
         ;  break
      ;  }

                              /* These have maps associated with them.
                               * Note that bitsp may be changed (by filling in
                               * somewhat underspecified settings).
                               * todo hierverder: etc case supported below ?
                              */
         if (abc || etc)
         stream_state_set_map(symmetric, &iface, streamer, &bits)

      ;  if (xf->fp == NULL && (mcxIOopen(xf, ON_FAIL) != STATUS_OK))
         {  mcxErr(me, "cannot open stream <%s>", xf->fn->str)
         ;  break
      ;  }
         status = STATUS_OK
      ;  break
   ;  }

      iface.bits  =  bits

   ;  if (!status)
      while (1)
      {  unsigned long x = 876543210, y = 876543210
      ;  double value = 0
      ;  n_ite++

      ;  iface.x =  0
      ;  iface.y =  0

      ;  if (n_ite % 20000 == 0)
         fputc('.', stderr)               /* fixme conditional to sth */
      ;  if (n_ite % 1000000 == 0)
         fprintf(stderr, " %ldM\n", (long) (n_ite / 1000000))

                        /* 
                         * -  the read routines largely manage iface, including
                         * map_c->max_seen and map_r->max_seen.  It would be
                         * nice to encapsulate that management in a single
                         * place. Note the read_abc requirement that sometimes
                         * a label may need to be deleted from a hash. The fact
                         * that handle_label (called by read_etc and read_abc)
                         * also manages max_seen complicate encapsulation though.
                         *
                         * -  read_etc manages its line buffer.
                        */
      ;  status
         =     one23 ?     read_123(xf, linebuf, &iface, streamer, &value, bits)
            :  abc   ?     read_abc(xf, linebuf, &iface, &value)
            :  longlist ?  read_etc(xf, &iface, &etcstate, &value)
            :  STATUS_FAIL

      ;  x = iface.x
      ;  y = iface.y

                        /* considerme: etc status ignore could still expand column range.
                         * do we change the status and deal with not incorporating the row,
                         * or do we keep status, and change realloc/ignore logic below?
                        */
;if(0)fprintf(stderr, "#x now %lu status %s\n", (ulong) (iface.map_c->max_seen+1), MCXSTATUS(status))
                        /* etc/235 are special in that with NEW x and IGNORE y
                         * we respect x
                         * fixme: should not do that for auto-increment
                        */
      ;  if (status == STATUS_IGNORE)     /* maybe restrict mode */
         {  if
            (  longlist
            && iface.statusx == STATUS_NEW
            && iface.map_c->max_seen+1 > iface.pars_n_used    /* note mixed-sign comparison */
            )
            {  if ((status = pars_realloc(&iface, iface.map_c->max_seen+1)))
               break
         ;  }
            continue
      ;  }
         else if (status)                 /* FAIL or DONE */
         break

      ;  if
         (  iface.map_c->max_seen >= iface.pars_n_used     /* note mixed-sign comparison */
         && (status = pars_realloc(&iface, iface.map_c->max_seen+1))
         )
         break

      ;  status = STATUS_FAIL    /* fixme restructure logic, mid-re-initialization is ugly */

      ;  if
         ( bits & (MCLXIO_STREAM_LOGTRANSFORM | MCLXIO_STREAM_NEGLOGTRANSFORM) )
         {  if (bits & MCLXIO_STREAM_LOGTRANSFORM)
            value = value > 0 ? log(value) : -PVAL_MAX
         ;  else if (bits & MCLXIO_STREAM_NEGLOGTRANSFORM)
            value = value > 0 ? -log(value) : PVAL_MAX
         ;  if (bits & MCLXIO_STREAM_LOG10)
            value /= log(10)
      ;  }

         if (transform)
         {  mclp bufivp
         ;  bufivp.idx = 0
         ;  bufivp.val = value
         ;  value = mclpUnary(&bufivp, transform)
      ;  }

                                 /* fixme: below we have canonical dependence, index as offset */
         if (value)
         {  if(DEBUG3)fprintf(stderr, "attempt to extend %d\n", (int) x)
         ;  if (mclpARextend(iface.pars+x, y, value))
            {  mcxErr(me, "x-extend fails")
            ;  break
         ;  }
            if (mirror && mclpARextend(iface.pars+y, x, value))
            {  mcxErr(me, "y-extend fails")
            ;  break
         ;  }
         }
         status = STATUS_OK
   ;  }

      if (n_ite >= 1000000 && n_ite % 5000000)
      fputc('\n', stderr)

   ;  mcxTingFree(&(etcstate.etcbuf))

   ;  if (status == STATUS_FAIL || ferror(xf->fp))
      mcxErr(me, "error occurred (status %d lc %d)", (int) status, (int) xf->lc)
   ;  else
      {  mx = make_mx_from_pars(streamer, &iface, ivpmerge, bits)
      ;  status = mx ? STATUS_OK : STATUS_FAIL
   ;  }

      mcxTingFree(&linebuf)
   ;  free_pars(&iface)

   ;  if (status == STATUS_FAIL)
      {  if (ON_FAIL == EXIT_ON_FAIL)
         mcxDie(1, me, "fini")
   ;  }

               /* with 123, etcai there is simply no column tab
                * todo: perhaps create a dummy one (integers).
               */
      if
      (  !status
      && (abc || (bits & (MCLXIO_STREAM_ETC | MCLXIO_STREAM_ETC_AI | MCLXIO_STREAM_SIF)))
      )
      {  if (symmetric)
         streamer->tab_sym_out = make_tab(iface.map_c)
      ;  else
         {  if (!(bits & MCLXIO_STREAM_ETC_AI))
            streamer->tab_col_out = make_tab(iface.map_c)
;if(0)fprintf(stderr, "%p x %p\n", (void*) iface.map_c->map, (void*) iface.map_c->tab)
;if(0)mcxHashStats(stdout, iface.map_c->map)
         ;  streamer->tab_row_out = make_tab(iface.map_r)
      ;  }
      }

      mcxHashFree(&(iface.map_c->map), mcxTingRelease, NULL)
   ;  if (!symmetric)
      mcxHashFree(&(iface.map_r->map), mcxTingRelease, NULL)

   ;  return mx
;  }


/* mcxIOstreamIn */

