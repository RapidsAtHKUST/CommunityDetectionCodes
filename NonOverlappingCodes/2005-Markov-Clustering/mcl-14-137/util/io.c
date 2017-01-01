/*   (C) Copyright 2000, 2001, 2002, 2003, 2004, 2005 Stijn van Dongen
 *   (C) Copyright 2006, 2007, 2008, 2009  Stijn van Dongen
 *
 * This file is part of tingea.  You can redistribute and/or modify tingea
 * under the terms of the GNU General Public License; either version 3 of the
 * License or (at your option) any later version.  You should have received a
 * copy of the GPL along with tingea, in the file COPYING.
*/


#define DEBUG 0

/* NOTE
 *    Very few routines should be allowed to touch buffer.
 *    create/free routines, and step and stepBack.
 *    currently many routines ignore the buffer (but warn
 *    when doing so).
 *    mcxIOtryCookie is currently only one filling buffer.
 *    Conceivably, mcxIOfind could be next.
 *
 * TODO
 *    buffering: document, who can initiate it?
 *    buffering: document, which routines are incompatible?
 *    general: remove dependency on ungetc.
 *    make mcxIOreadLine/ mcxIOstep zlib-aware.
 *    mcxIOfind can be made much faster.
 *    - inline fillpatbuf
 *    - get rid of modulus computations - subtract  patlen if necessary.
 *    - use buffered input.
*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>


#include "io.h"
#include "types.h"
#include "ting.h"
#include "ding.h"
#include "err.h"
#include "alloc.h"
#include "compile.h"
#include "getpagesize.h"


#define inbuffer(xf)  (xf->buffer_consumed < xf->buffer->len)

static void buffer_empty
(  mcxIO* xf
)
   {  mcxTingEmpty(xf->buffer, getpagesize())      /* sets xf->buffer->len to 0 */
   ;  xf->buffer_consumed = 0
;  }


static void buffer_spout
(  mcxIO* xf
,  const char* me
)
   {  mcxErr
      (  "mcxIO"
      ,  "warning: reader %s in file <%s> discards buffered input <%.*s>"
      ,  me
      ,  xf->fn->str
      ,  (int) (xf->buffer->len - xf->buffer_consumed)
      ,  xf->buffer->str+xf->buffer_consumed
      )
   ;  buffer_empty(xf)
;  }


int begets_stdio
(  const char* name
,  const char* mode
)
   {  if
      (  (  strchr(mode, 'r')
         && !strcmp(name, "-")
         )
      || (  (strchr(mode, 'w') || strchr(mode, 'a'))
         && (!strcmp(name, "-") || !strcmp(name, "stderr"))
         )
      )
      return 1
   ;  return 0
;  }


static int mcxIOwarnOpenfp
(  mcxIO*    xf
,  const char* who
)
   {  if (xf->fp && !xf->stdio)
      {  mcxIOerr(xf, who, "has open file pointer")
      ;  return 1
   ;  }
      return 0
;  }


mcxstatus mcxIOclose
(  mcxIO*    xf
)
   {  fflush(xf->fp)
;if (!strcmp(xf->fn->str, "-") && !strcmp(xf->mode, "w") && !xf->stdio)
 mcxDie(1, "tst", "should not happen")

   ;  if (xf->fp && !xf->stdio)
      {  fclose(xf->fp)
      ;  xf->fp = NULL
   ;  }
      else if (xf->fp && xf->stdio)
      {  int fe = ferror(xf->fp)    /* fixme why not in branch above? */
      ;  if (fe)
            mcxErr("mcxIOclose", "error [%d] for [%s] stdio", fe, xf->mode)
         ,  perror("mcxIOclose")
      ;  if (xf->ateof || feof(xf->fp))
         clearerr(xf->fp)
   ;  }
                                    /* fixme contract with usr_reset */
      return mcxIOreset(xf)
;  }


/* note: does not touch all members, notably
 *    usr
 *    fn
 *    mode
 *    fp
*/

mcxstatus mcxIOreset
(  mcxIO*    xf
)
   {  xf->lc      =  0
   ;  xf->lo      =  0
   ;  xf->lo_     =  0
   ;  xf->bc      =  0
   ;  xf->ateof   =  0

                     /* regardless of read/write */
   ;  buffer_empty(xf)
                     /*  xf->fp not touched; promote user care */
   ;  if (xf->usr && xf->usr_reset)
      return xf->usr_reset(xf->usr)

   ;  return STATUS_OK
;  }


void mcxIOerr
(  mcxIO*   xf
,  const char     *complainer
,  const char     *complaint
)
   {  if (!xf)
      return

   ;  mcxErr
      (  complainer
      ,  "%s stream <%s> %s"
      ,  xf->mode
      ,  xf->fn->str
      ,  complaint
      )  
;  }


mcxIO* mcxIOnew
(  const char*       str
,  const char*       mode
)
   {  if (!str || !mode)
      {  mcxErr("mcxIOnew PBD", "void string or mode argument")
      ;  return NULL
   ;  }

      return mcxIOrenew(NULL, str, mode)
;  }


/* fixme: the code below is mildly muddy.
 * The ->stdio decision (for new streams) might best be made
 * at open time?
*/

mcxIO* mcxIOrenew
(  mcxIO*            xf
,  const char*       name
,  const char*       mode
)
   {  mcxbool twas_stdio = xf && xf->stdio      /* it was one of STD{IN,OUT,ERR} */
   ;  if
      (  mode
      && !strstr(mode, "w") && !strstr(mode, "r") && !strstr(mode, "a")
      )
      {  mcxErr ("mcxIOrenew PBD", "unsupported open mode <%s>", mode)
      ;  return NULL
   ;  }

      if
      (  getenv("TINGEA_PLUS_APPEND")
      && (  name && (uchar) name[0] == '+' )
      && (  mode && strchr(mode, 'w') )
      )
      {  name++               /* user can specify -o +foo to append to foo */
      ;  mode = "a"
   ;  }

      if (!xf)                /* case 1)   create a new one */
      {  if (!name || !mode)
         {  mcxErr("mcxIOrenew PBD", "too few arguments")
         ;  return NULL
      ;  }

         if (!(xf = (mcxIO*) mcxAlloc(sizeof(mcxIO), RETURN_ON_FAIL)))
         return NULL

      ;  if (!(xf->fn = mcxTingEmpty(NULL, 20)))
         return NULL

      ;  if (!(xf->buffer = mcxTingEmpty(NULL, getpagesize())))
         return NULL

      ;  xf->fp      =  NULL
      ;  xf->mode    =  NULL
      ;  xf->usr     =  NULL
      ;  xf->usr_reset =  NULL
      ;  xf->buffer_consumed = 0
   ;  }
      else if (xf->stdio)     /* case 2)   have one, don't close */
      NOTHING
   ;  else if (mcxIOwarnOpenfp(xf, "mcxIOrenew"))
      mcxIOclose(xf)          /* case 3)   have one, warn and close if open */

   ;  mcxIOreset(xf)

   ;  if (name && !mcxTingWrite(xf->fn, name))
      return NULL

   ;  if (mode)
      {  if (xf->mode)
         mcxFree(xf->mode)
      ;  xf->mode = mcxStrDup(mode)
   ;  }

      xf->stdio = begets_stdio(xf->fn->str, xf->mode)

                                       /* name changed, no longer stdio */
   ;  if (twas_stdio && !xf->stdio)
      xf->fp = NULL

   ;  if (xf->stdio && mode && strchr(mode, 'a'))     /* recently added */
      {  if (xf->mode)
         mcxFree(xf->mode)
      ;  xf->mode = mcxStrDup("w")
   ;  }

      return xf
;  }


mcxstatus  mcxIOopen
(  mcxIO*   xf
,  mcxOnFail      ON_FAIL
)
   {  const char* fname    =  xf->fn->str
   ;  if (!xf)
      {  mcxErr("mcxIOnew PBD", "received void object")
      ;  if (ON_FAIL == RETURN_ON_FAIL)
         return STATUS_FAIL
      ;  exit(1)
   ;  }

      if (mcxIOwarnOpenfp(xf, "mcxIOopen PBD"))
      return STATUS_OK

   ;  if (!strcmp(fname, "-"))
      {  if (strchr(xf->mode, 'r'))
         xf->fp =  stdin
      ;  else if (strchr(xf->mode, 'w') || strchr(xf->mode, 'a'))
         xf->fp =  stdout
   ;  }

      else if
      (  !strcmp(fname, "stderr")
      && (strchr(xf->mode, 'w') || strchr(xf->mode, 'a'))
      )
      xf->fp =  stderr

   ;  else if ((xf->fp = fopen(fname, xf->mode)) == NULL)
      {  if (ON_FAIL == RETURN_ON_FAIL)
         return STATUS_FAIL
      ;  mcxIOerr(xf, "mcxIOopen", "cannae be opened")
      ;  mcxExit(1)
   ;  }

      return STATUS_OK
;  }


mcxstatus mcxIOtestOpen
(  mcxIO*         xf
,  mcxOnFail      ON_FAIL
)
   {  if (!xf->fp && mcxIOopen(xf, ON_FAIL) != STATUS_OK)
      {  mcxErr
         ("mcxIO", "cannot open file <%s> in mode %s", xf->fn->str, xf->mode)
      ;  return STATUS_FAIL
   ;  }
      return  STATUS_OK
;  }



void mcxIOrelease
(  mcxIO*  xf
)
   {  if (xf)
      {  mcxIOclose(xf)

      ;  if (xf->fn)
         mcxTingFree(&(xf->fn))
      ;  if (xf->mode)
         mcxFree(xf->mode)
   ;  }
   }


mcxstatus mcxIOappendName
(  mcxIO*         xf
,  const char*    suffix
)
   {  if (xf->fp && !xf->stdio)
      mcxErr
      (  "mcxIOappendName PBD"
      ,  "stream open while request for name change from <%s> to <%s>"
      ,  xf->fn->str
      ,  suffix
      )
   ;  else if (!mcxTingAppend(xf->fn, suffix))
      return STATUS_FAIL

   ;  xf->stdio = begets_stdio(xf->fn->str, "-")
   ;  return STATUS_OK
;  }


#if 0
mcxstatus mcxIOnewName
(  mcxIO*         xf
,  const char*    newname
)
   {  if (!mcxTingEmpty(xf->fn, 0))
      return STATUS_FAIL
   ;  return mcxIOappendName(xf, newname)
;  }
#else
mcxstatus mcxIOnewName
(  mcxIO*         xf
,  const char*    newname
)
   {  return mcxIOrenew(xf, newname, NULL) ? STATUS_OK : STATUS_FAIL
;  }
#endif


int mcxIOstepback
(  int c
,  mcxIO*    xf
)
   {  if (c == EOF)
      return EOF
   ;  else
      {  if (inbuffer(xf) && xf->buffer_consumed > 0)
         c = xf->buffer->str[--xf->buffer_consumed]
   /* insert a new branch for zlib aware reading here; splice into buffer */
      ;  else if (ungetc(c, xf->fp) == EOF)
         {  mcxErr
            (  "mcxIOstepback"
            ,  "failed to push back <%d> on stream <%s>\n"
            ,  c
            ,  xf->fn->str
            )
         ;  return EOF
      ;  }

         xf->bc--
      ;  if (c == '\n')
            xf->lc--
         ,  xf->lo = xf->lo_
         ,  xf->lo_ = 0
      ;  else
         xf->lo--
   ;  }
      return c
;  }


int mcxIOstep
(  mcxIO*    xf
)
   {  int c

#if 0
;if (xf->buffer)
fprintf(stderr, "buffer [%s]\n", xf->buffer->str)
;else
fprintf(stderr, "nobuffer\n")
#endif


   ;  if (xf->ateof)
      c = EOF
   ;  else if (inbuffer(xf))
      {  c = xf->buffer->str[xf->buffer_consumed++]
      ;  if (!inbuffer(xf))
         buffer_empty(xf)
   ;  }
      else
      c = fgetc(xf->fp)

   ;  switch(c)
      {
      case '\n'
      :  xf->lc++
      ;  xf->bc++
      ;  xf->lo_  =  xf->lo
      ;  xf->lo   =  0
      ;  break
      ;

      case EOF
      :  xf->ateof =  1
      ;  break
      ;

      default
      :  xf->bc++
      ;  xf->lo++
      ;  break
   ;  }
      return c
;  }


/* fixme todo
 *
 * look at bc; substract it from sz (we might have read a part already).
 *
 * support growing files. and look at other items in the grep source code.
*/

mcxstatus  mcxIOreadFile
(  mcxIO    *xf
,  mcxTing   *filetxt
)
   {  struct stat mystat
   ;  size_t sz = 4096
   ;  ssize_t r
   ;  const char* me = "mcxIOreadFile"

   ;  mcxTingEmpty(filetxt, 0)

   ;  if (inbuffer(xf))
      buffer_spout(xf, me)

   ;  if (!xf->stdio)
      {  if (stat(xf->fn->str, &mystat))
         mcxIOerr(xf, me, "cannae stat file")
      ;  else
         sz = mystat.st_size
   ;  }

      if (!xf->fp && mcxIOopen(xf, RETURN_ON_FAIL))
      {  mcxIOerr(xf, me, "cannae open file")
      ;  return STATUS_FAIL
   ;  }

      if (xf->ateof)
      return STATUS_OK
                                          /* fixme; ting count overflow */
   ;  if (!(filetxt = mcxTingEmpty(filetxt, sz)))
      return STATUS_NOMEM

   ;  while ((r = mcxIOappendChunk(xf, filetxt, sz, 0)) > 0 && !xf->ateof)

   ;  if (r <0)
      return STATUS_FAIL                  /* fixme; look closer at error */

   ;  return STATUS_OK
;  }


static dim  mcxIO__rl_fillbuf__
(  mcxIO*   xf
,  char*    buf
,  dim      size
,  int*     last
)
   {  int   a  = 0
   ;  dim   ct = 0
   ;  while(ct<size && EOF != (a = mcxIOstep(xf)))
      {  if (a != '\0')
         buf[ct++] = a
      ;  if (a == '\n')
         break
   ;  }
      *last =  a
   ;  return ct      /* fixme: '\0' not accounted */
;  }


#define IO_MEM_ERROR (EOF-1)

static ofs mcxIO__rl_rl__
(  mcxIO    *xf
,  mcxTing  *lineTxt
)
#define MCX_IORL_BSZ 513
   {  char  cbuf[MCX_IORL_BSZ]
   ;  int   z

   ;  if (!mcxTingEmpty(lineTxt, 1))
      return IO_MEM_ERROR
     /* todo/fixme; need to set errno so that caller can know */

   ;  while (1)
      {  dim ct = mcxIO__rl_fillbuf__(xf, cbuf, MCX_IORL_BSZ, &z)
      ;  if (ct && !mcxTingNAppend(lineTxt, cbuf, ct))
         return IO_MEM_ERROR
      ;  if (z == '\n' || z == EOF)
         break
   ;  }
      if (z == EOF)
      xf->ateof = 1
   ;  return z
#undef MCX_IORL_BSZ
;  }


dim mcxIOdiscardLine
(  mcxIO       *xf
)
   {  int a
   ;  dim ct = 0

   ;  if (!xf->fp)
      {  mcxIOerr(xf, "mcxIOdiscardLine", "is not open")
      ;  return 0          /* fixme; set errno? */
   ;  }

      while(((a = mcxIOstep(xf)) != '\n') && a != EOF)
      ct++

   ;  if (inbuffer(xf))     /* fixme/design check buffer for line */
      buffer_spout(xf, "mcxIOdiscardLine")

   ;  return ct
;  }



ofs mcxIOappendChunk
(  mcxIO      *xf
,  mcxTing     *dst
,  dim         sz
,  mcxbits     flags cpl__unused
)
   {  unsigned long psz =  getpagesize()
   ;  dim k          =  sz / psz       /* fixme: size checks? */
   ;  dim rem        =  sz % psz
   ;  ofs r          =  1              /* pretend in case k == 0 */
  /*  mcxbool  account = flags & MCX_CHUNK_ACCOUNT ? TRUE : FALSE */
   ;  dim offset     =  dst->len
   ;  char* p

   ;  if (!dst || !xf->fp || !mcxTingEnsure(dst, dst->len + sz))
      return -1
      /* fixme set some (new) errno */

   ;  if (k)
      while (k-- > 0 && (r = read(fileno(xf->fp),  dst->str+dst->len, psz)) > 0)
      dst->len += r     /* careful with unsignedness */

                        /* ^ Note: fileno is POSIX, not c99 */
   ;  if
      (  r > 0
      && rem > 0
      && (r = read(fileno(xf->fp),  dst->str+dst->len, rem)) > 0
      )
      dst->len += r

   ;  dst->str[dst->len] = '\0'
   ;  xf->bc += dst->len - offset

   ;  for (p = dst->str+offset; p<dst->str+dst->len; p++)
      {  if (*p == '\n')
         {  xf->lc++
         ;  xf->lo_ = xf->lo
         ;  xf->lo = 0
      ;  }
         else
         xf->lo++
   ;  }
                           /* fixme; what if k == 0, rem == 0 ? */
      if (!r)                    /* fixme; other possibilities? */
      xf->ateof = 1

   ;  return dst->len
;  }


mcxstatus  mcxIOreadLine
(  mcxIO    *xf
,  mcxTing  *dst
,  mcxbits  flags
)
   {  int      a
   ;  dim      ll
   ;  mcxbool  chomp    =  flags & MCX_READLINE_CHOMP       ? TRUE : FALSE
   ;  mcxbool  skip     =  flags & MCX_READLINE_SKIP_EMPTY  ? TRUE : FALSE
   ;  mcxbool  par      =  flags & MCX_READLINE_PAR         ? TRUE : FALSE
   ;  mcxbool  dot      =  flags & MCX_READLINE_DOT         ? TRUE : FALSE
   ;  mcxbool  bsc      =  flags & MCX_READLINE_BSC         ? TRUE : FALSE
   ;  mcxbool  repeat   =  dot || par || bsc                ? TRUE : FALSE
   ;  mcxbool  continuation   =  FALSE
   ;  mcxTing* line
   ;  mcxstatus stat    =  STATUS_OK

   ;  if (!xf->fp && mcxIOopen(xf, RETURN_ON_FAIL))
      {  mcxIOerr(xf, "mcxIOreadLine", "is not open")
      ;  return STATUS_FAIL
   ;  }

      if (xf->ateof)
      return STATUS_DONE

   ;  if (!dst || !mcxTingEmpty(dst, 1))
      return STATUS_NOMEM

   ;  if (skip || par)
      {  while((a = mcxIOstep(xf)) == '\n')
         NOTHING
      ;  if (xf->ateof)
         return STATUS_DONE
      ;  else
         mcxIOstepback(a, xf)
   ;  }

      if (!(line = repeat ? mcxTingEmpty(NULL, 1) : dst))
      return STATUS_NOMEM

   ;  while (1)
      {  ofs d = mcxIO__rl_rl__(xf, line)
      ;  if (IO_MEM_ERROR == d)
         {  stat = STATUS_NOMEM     /* fixme grainify error/status */
         ;  break
      ;  }

         ll = line->len

      ;  if (!repeat)
         break
      ;  else  /* must append line to dst */
         {  if
            (  dot
            && !continuation
            && line->str[0] == '.'
            && (  ll == 2
               || (ll == 3 && line->str[1] == '\r')
               )
               /* fixme still not fully covering */
            )
            break
                  /* do not attach the single-dot-line */

         ;  if (par && !continuation && ll == 1)
            break
                  /* do not attach the second newline */

         ;  if (!mcxTingNAppend(dst, line->str, line->len))
            {  stat = STATUS_NOMEM
            ;  break
         ;  }

            continuation = bsc && (ll > 1 && *(line->str+ll-2) == '\\')

         ;  if (continuation)
            mcxTingShrink(dst, -2)

         ;  if (!par && !dot && (bsc && !continuation))
            break

         ;  if (xf->ateof)
            break
      ;  }
      }

      if (repeat)
      mcxTingFree(&line)

   ;  if (stat)
      return stat    /* fixme; should we not check chomp first ? */

                     /* fixme _: \n\r ? */
   ;  if (chomp && dst->len && *(dst->str+dst->len-1) == '\n')
      mcxTingShrink(dst, -1)

   ;  if (xf->ateof && !dst->len)
      return STATUS_DONE

   ;  return STATUS_OK
;  }


void mcxIOlistParmodes
(  void
)
   {  fprintf
      (  stdout
      ,  "%5d  vanilla mode, fetch next line\n"
      ,  MCX_READLINE_DEFAULT
      )
   ;  fprintf(stdout, "%5d  chomp trailing newline\n", MCX_READLINE_CHOMP)
   ;  fprintf(stdout, "%5d  skip empty lines\n", MCX_READLINE_SKIP_EMPTY)
   ;  fprintf(stdout, "%5d  paragraph mode\n", MCX_READLINE_PAR)
   ;  fprintf(stdout, "%5d  backslash escapes newline\n", MCX_READLINE_BSC)
   ;  fprintf
      (  stdout
      ,  "%5d  section mode, ended by singly dot on a single line\n"
      ,  MCX_READLINE_DOT
      )
;  }


void mcxIOpos
(  mcxIO*    xf
,  FILE*          channel
)
   {  const char* ateof =  xf->ateof ? "at EOF in " : ""
   ;  fprintf
      (  channel
      ,  "[mcxIO] %sstream <%s>, line <%ld>, character <%ld>\n"
      ,  ateof
      ,  xf->fn->str
      ,  (long) xf->lc
      ,  (long) xf->lo
      )
;  }


void mcxIOfree_v
(  void*  xfpp
)  
   {  mcxIOfree((mcxIO**) xfpp)
;  }


void mcxIOfree
(  mcxIO**  xfpp
)  
   {  if (*xfpp)
      {  mcxIO* xf = *xfpp
      ;  mcxIOrelease(xf)
      ;  mcxTingFree(&(xf->buffer))
      ;  if (xf->usr && xf->usr_free)
         xf->usr_free(xf->usr)
      ;  mcxFree(xf)
      ;  *xfpp =  NULL
   ;  }
   }



mcxstatus mcxIOexpectReal
(  mcxIO*        xf
,  double*       dblp
,  mcxOnFail     ON_FAIL
)
   {  int   n_read   =  0
   ;  int   n_conv   =  0

   ;  if (inbuffer(xf))
      buffer_spout(xf, "mcxIOexpectReal")

   ;  mcxIOskipSpace(xf)      /* keeps accounting correct */

   ;  n_conv   =  fscanf(xf->fp, " %lf%n", dblp, &n_read)

   ;  xf->bc += n_read  /* fixme do fscanf error handling */
   ;  xf->lo += n_read

   ;  if (1 != n_conv)
      {  if (ON_FAIL == EXIT_ON_FAIL)
         {  mcxIOpos(xf, stderr)
         ;  mcxErr("parseReal", "parse error: expected to find real")
         ;  mcxExit(1)
      ;  }
         return STATUS_FAIL
   ;  }
      return STATUS_OK
;  }


mcxstatus mcxIOexpectNum
(  mcxIO*        xf
,  long*         lngp
,  mcxOnFail     ON_FAIL
)
   {  int   n_read   =  0
   ;  int   n_conv   =  0
   ;  mcxstatus status = STATUS_OK

   ;  if (inbuffer(xf))
      buffer_spout(xf, "mcxIOexpectNum")

   ;  mcxIOskipSpace(xf)      /* keeps accounting correct */

   ;  errno = 0
   ;  n_conv   =  fscanf(xf->fp, "%ld%n", lngp, &n_read)

   ;  xf->bc += n_read  /* fixme do fscanf error handling */
   ;  xf->lo += n_read

   ;  if (1 != n_conv)
         mcxErr("mcxIOexpectNum", "parse error: expected to find integer")
      ,  status = STATUS_FAIL
   ;  else if (errno == ERANGE)
         mcxErr("mcxIOexpectNum", "range error: not in allowable range")
      ,  status = STATUS_FAIL

   ;  if (status)
      {  mcxIOpos(xf, stderr)
      ;  if (ON_FAIL == EXIT_ON_FAIL)
         mcxExit(1)
   ;  }

      return status
;  }



int mcxIOskipSpace
(  mcxIO*               xf
)
   {  int c
   ;  while ((c = mcxIOstep(xf)) != EOF && isspace(c))
      ;

      return mcxIOstepback(c, xf)
;  }


mcxbool mcxIOtryCookie
(  mcxIO* xf
,  const uchar abcd[4]
)
   {  uchar efgh[5]
   ;  int n_read = fread(efgh, sizeof efgh[0], 4, xf->fp)
   ;  int error  = ferror(xf->fp)
   ;  dim i = 0

   ;  if (n_read == 4)
      for (i=0; i<4 && abcd[i] == efgh[i]; i++)
      NOTHING

#if 0
;fprintf(stderr, "IN %d %d %d %d %d %d\n", i, n_read, (int) efgh[0], (int) efgh[1], (int) efgh[2], (int) efgh[3])
;fprintf(stderr, "IN %d %d %d %d %d %d\n", i, n_read, (int) abcd[0], (int) abcd[1], (int) abcd[2], (int) abcd[3])
#endif
   ;  if (i == 4)
      {  xf->bc += 4
      ;  return TRUE
   ;  }

      if (!fseek(xf->fp, -n_read, SEEK_CUR))
      xf->bc += (4-n_read)
   ;  else
      {  mcxTingNAppend(xf->buffer, (char*) efgh, n_read)
      ;  /* xf->bc += n_read  mcxIOstep does subsequent accounting */
      ;  if (!error)
         clearerr(xf->fp)
   ;  }

      return FALSE
;  }


mcxbool mcxIOwriteCookie
(  mcxIO* xf
,  const uchar abcd[4]
)
   {  dim n_written = fwrite(abcd, sizeof abcd[0], 4, xf->fp)
   ;  if (n_written != 4)
      {  mcxErr("mcxIOwriteCookie", "failed to write <%.4s>", abcd)
      ;  return FALSE
   ;  }

      return TRUE
;  }



/* fixme: newlines in str thrash correct lc/lo counting */

int mcxIOexpect
(  mcxIO          *xf
,  const char     *str
,  mcxOnFail      ON_FAIL
)
   {  const char* s  =  str
   ;  int         c  =  0
   ;  int         d  =  0
   ;  int         n_trailing

  /*  
   *  no functional behaviour yet attached to this state change
   *  fixme: semantics for STDIN agree between stdlib and us?
  */

   ;  while
      (  c = (uchar) s[0]
      ,  
         (  c
         && (  d = mcxIOstep(xf)
            ,  c == d
            )
         )
      )
      s++

   ;  n_trailing = strlen(s)     /* truncintok */

   ;  if (c && ON_FAIL == EXIT_ON_FAIL)
      {  mcxErr("mcxIOexpect", "parse error: expected to see <%s>", str)
      ;  mcxIOpos(xf, stderr)
      ;  mcxExit(1)
   ;  }
      return n_trailing
;  }



typedef struct
{  int            tbl[256]
;  int*           circle         /* circular buffer */
;  int            circle_last    /* circle bumper */
;  const char*    pat
;  int            patlen
;
}  mcxIOpat       ;


static void mcxio_newpat
(  mcxIOpat* md
,  const char* pattern
)
   {  int i
   ;  int *tbl = md->tbl
   ;  const char* pat
   ;  int patlen = strlen(pattern)     /* truncintok */

   ;  md->circle = mcxAlloc(patlen * sizeof(int), EXIT_ON_FAIL)
   ;  md->pat = pattern
   ;  md->patlen = patlen

   ;  pat = md->pat
                                 /* initialize */
   ;  for (i = 0; i < 256; i ++)
      tbl[i] = patlen

   ;  for (i = 0; i < patlen-1; i++)
      tbl[(uchar) pat[i]] = patlen -i -1

#if DEBUG
   ;  for (i=0; i<patlen; i++)
      fprintf
      (  stderr
      ,  "shift value for %c is %d\n", pat[i], tbl[(uchar) pat[i]]
      )
#endif

   ;  md->circle_last = patlen -1
;  }


static void mcxIOcleanpat
(  mcxIOpat* md
)
   {  mcxFree(md->circle)
;  }



static int fillpatbuf
(  mcxIO* xfin
,  int shift
,  mcxIOpat* md
)
   {  int c = 0
   ;  int z = 0
   ;  int patlen = md->patlen

   ;  while (z < shift && (c = mcxIOstep(xfin)) != EOF)
      {  int q = (md->circle_last+z+1) % patlen
      ;  md->circle[q] = c
      ;  z++
   ;  }

      md->circle_last = (md->circle_last+shift) % patlen
   ;  return c
;  }


mcxstatus mcxIOfind
(  mcxIO*      xfin
,  const char* pat
,  mcxOnFail   ON_FAIL  
)
   {  int j, k
   ;  int shift, patlen
   ;  int* tbl
   ;  int* circle
   ;  mcxIOpat md
   ;  int found = 0

   ;  mcxio_newpat(&md, pat)

   ;  patlen   =  md.patlen
   ;  tbl      =  md.tbl
   ;  circle   =  md.circle
   ;  shift    =  patlen

     /*
      * hum. This means that empty pattern matches on empty string ..
      * Need to fix fillpatbuf if this should be reversed.
     */
   ;  if (!patlen)
      found = 1
   ;  else
      do
      {  if (EOF == fillpatbuf(xfin, shift, &md))
         break
      ;  for
         (  j=md.circle_last+patlen, k=patlen-1
         ;  j>md.circle_last && circle[j%patlen] == (uchar) pat[k]
         ;  j--, k--
         )
         NOTHING
#if DEBUG
fprintf
(stderr
,"comparing circlebuf pos %d char [%c] with pattern pos %d char [%c]\n"
,(int) (j%patlen)
,(int) ((uchar) circle[j%patlen])
,(int) k
,(int) pat[k] 
)
#endif

      ;  if (j == md.circle_last)
         {  found++
         ;  break
      ;  }

         shift = tbl[circle[md.circle_last % patlen]]

#if DEBUG
;  fprintf
   (  stderr
   ,  "___ last[%d] index[%d] pivot[%c] shift[%d]\n"
   ,  (int) md.circle_last
   ,  (int) md.circle_last % patlen
   ,  (int) circle[md.circle_last % patlen]
   ,  (int) shift
   )
#endif
   ;  }
      while (1)

   ;  mcxIOcleanpat(&md)

   ;  if (!found && ON_FAIL == RETURN_ON_FAIL)
      return STATUS_FAIL
   ;  else if (!found)
      exit(EXIT_FAILURE)

   ;  return STATUS_OK
;  }


dim mcxIOdiscard
(  mcxIO*      xf
,  dim         amount
)
   {  dim bsz        =  xf->buffer->mxl
   ;  char* buf      =  xf->buffer->str
   ;  dim  n_read    =  0
   ;  dim  n_chunk   =  amount / bsz
   ;  dim  rem       =  amount - bsz * n_chunk
   ;  dim  i, n

   ;  if (inbuffer(xf))
      buffer_spout(xf, "mcxIOdiscard")

   ;  for (i=0;i<n_chunk;i++)
      {  n = fread(buf, 1, bsz, xf->fp)
      ;  n_read += n
      ;  xf->bc += n  
      ;  if (n != bsz)
         break
   ;  }

      if (i < n_chunk)
      return n_read

   ;  if (rem)
         n = fread(buf, 1, rem, xf->fp)
      ,  n_read += n
      ,  xf->bc += n  

   ;  return n_read
;  }



