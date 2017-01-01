/*   (C) Copyright  2014  Stijn van Dongen
 *
 * This file is part of MCL.  You can redistribute and/or modify MCL under the
 * terms of the GNU General Public License; either version 3 of the License or
 * (at your option) any later version.  You should have received a copy of the
 * GPL along with MCL, in the file COPYING.
*/

/* TODO: funcify dump, load, read
   -start
   -end
   -t
   dimension check
      -nrow 1155515
      -ncol 2048

   sse3 instructions?

-  tanimoto working correctly on 2048 bit vectors?
-  distributed loading system working correctly?
-  high node degree (many bits?)
-  bit histogram
 */


#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "mcx.h"

#include "util/types.h"
#include "util/io.h"
#include "util/err.h"
#include "util/opt.h"
#include "util/compile.h"
#include "util/alloc.h"

#include "impala/io.h"
#include "impala/matrix.h"
#include "impala/tab.h"
#include "impala/stream.h"
#include "impala/app.h"

#include "gryphon/path.h"



   /* Big endian for convenient printing */
const char* bytes[256]
=  {  "00000000"
   ,  "10000000"
   ,  "01000000"
   ,  "11000000"
   ,  "00100000"
   ,  "10100000"
   ,  "01100000"
   ,  "11100000"
   ,  "00010000"
   ,  "10010000"
   ,  "01010000"
   ,  "11010000"
   ,  "00110000"
   ,  "10110000"
   ,  "01110000"
   ,  "11110000"
   ,  "00001000"
   ,  "10001000"
   ,  "01001000"
   ,  "11001000"
   ,  "00101000"
   ,  "10101000"
   ,  "01101000"
   ,  "11101000"
   ,  "00011000"
   ,  "10011000"
   ,  "01011000"
   ,  "11011000"
   ,  "00111000"
   ,  "10111000"
   ,  "01111000"
   ,  "11111000"
   ,  "00000100"
   ,  "10000100"
   ,  "01000100"
   ,  "11000100"
   ,  "00100100"
   ,  "10100100"
   ,  "01100100"
   ,  "11100100"
   ,  "00010100"
   ,  "10010100"
   ,  "01010100"
   ,  "11010100"
   ,  "00110100"
   ,  "10110100"
   ,  "01110100"
   ,  "11110100"
   ,  "00001100"
   ,  "10001100"
   ,  "01001100"
   ,  "11001100"
   ,  "00101100"
   ,  "10101100"
   ,  "01101100"
   ,  "11101100"
   ,  "00011100"
   ,  "10011100"
   ,  "01011100"
   ,  "11011100"
   ,  "00111100"
   ,  "10111100"
   ,  "01111100"
   ,  "11111100"
   ,  "00000010"
   ,  "10000010"
   ,  "01000010"
   ,  "11000010"
   ,  "00100010"
   ,  "10100010"
   ,  "01100010"
   ,  "11100010"
   ,  "00010010"
   ,  "10010010"
   ,  "01010010"
   ,  "11010010"
   ,  "00110010"
   ,  "10110010"
   ,  "01110010"
   ,  "11110010"
   ,  "00001010"
   ,  "10001010"
   ,  "01001010"
   ,  "11001010"
   ,  "00101010"
   ,  "10101010"
   ,  "01101010"
   ,  "11101010"
   ,  "00011010"
   ,  "10011010"
   ,  "01011010"
   ,  "11011010"
   ,  "00111010"
   ,  "10111010"
   ,  "01111010"
   ,  "11111010"
   ,  "00000110"
   ,  "10000110"
   ,  "01000110"
   ,  "11000110"
   ,  "00100110"
   ,  "10100110"
   ,  "01100110"
   ,  "11100110"
   ,  "00010110"
   ,  "10010110"
   ,  "01010110"
   ,  "11010110"
   ,  "00110110"
   ,  "10110110"
   ,  "01110110"
   ,  "11110110"
   ,  "00001110"
   ,  "10001110"
   ,  "01001110"
   ,  "11001110"
   ,  "00101110"
   ,  "10101110"
   ,  "01101110"
   ,  "11101110"
   ,  "00011110"
   ,  "10011110"
   ,  "01011110"
   ,  "11011110"
   ,  "00111110"
   ,  "10111110"
   ,  "01111110"
   ,  "11111110"
   ,  "00000001"
   ,  "10000001"
   ,  "01000001"
   ,  "11000001"
   ,  "00100001"
   ,  "10100001"
   ,  "01100001"
   ,  "11100001"
   ,  "00010001"
   ,  "10010001"
   ,  "01010001"
   ,  "11010001"
   ,  "00110001"
   ,  "10110001"
   ,  "01110001"
   ,  "11110001"
   ,  "00001001"
   ,  "10001001"
   ,  "01001001"
   ,  "11001001"
   ,  "00101001"
   ,  "10101001"
   ,  "01101001"
   ,  "11101001"
   ,  "00011001"
   ,  "10011001"
   ,  "01011001"
   ,  "11011001"
   ,  "00111001"
   ,  "10111001"
   ,  "01111001"
   ,  "11111001"
   ,  "00000101"
   ,  "10000101"
   ,  "01000101"
   ,  "11000101"
   ,  "00100101"
   ,  "10100101"
   ,  "01100101"
   ,  "11100101"
   ,  "00010101"
   ,  "10010101"
   ,  "01010101"
   ,  "11010101"
   ,  "00110101"
   ,  "10110101"
   ,  "01110101"
   ,  "11110101"
   ,  "00001101"
   ,  "10001101"
   ,  "01001101"
   ,  "11001101"
   ,  "00101101"
   ,  "10101101"
   ,  "01101101"
   ,  "11101101"
   ,  "00011101"
   ,  "10011101"
   ,  "01011101"
   ,  "11011101"
   ,  "00111101"
   ,  "10111101"
   ,  "01111101"
   ,  "11111101"
   ,  "00000011"
   ,  "10000011"
   ,  "01000011"
   ,  "11000011"
   ,  "00100011"
   ,  "10100011"
   ,  "01100011"
   ,  "11100011"
   ,  "00010011"
   ,  "10010011"
   ,  "01010011"
   ,  "11010011"
   ,  "00110011"
   ,  "10110011"
   ,  "01110011"
   ,  "11110011"
   ,  "00001011"
   ,  "10001011"
   ,  "01001011"
   ,  "11001011"
   ,  "00101011"
   ,  "10101011"
   ,  "01101011"
   ,  "11101011"
   ,  "00011011"
   ,  "10011011"
   ,  "01011011"
   ,  "11011011"
   ,  "00111011"
   ,  "10111011"
   ,  "01111011"
   ,  "11111011"
   ,  "00000111"
   ,  "10000111"
   ,  "01000111"
   ,  "11000111"
   ,  "00100111"
   ,  "10100111"
   ,  "01100111"
   ,  "11100111"
   ,  "00010111"
   ,  "10010111"
   ,  "01010111"
   ,  "11010111"
   ,  "00110111"
   ,  "10110111"
   ,  "01110111"
   ,  "11110111"
   ,  "00001111"
   ,  "10001111"
   ,  "01001111"
   ,  "11001111"
   ,  "00101111"
   ,  "10101111"
   ,  "01101111"
   ,  "11101111"
   ,  "00011111"
   ,  "10011111"
   ,  "01011111"
   ,  "11011111"
   ,  "00111111"
   ,  "10111111"
   ,  "01111111"
   ,  "11111111"
   }  ;


const int nbits[256] =
{  0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
   1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
   2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
   3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
   4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
}  ;

enum
{  MY_OPT_DATA = MCX_DISP_UNUSED
,  MY_OPT_CUTOFF
,  MY_OPT_OUTPUT
,  MY_OPT_DUMP
,  MY_OPT_LOAD
,  MY_OPT_WRITE_TAB
,  MY_OPT_WRITE_HIST
,  MY_OPT_COLUMN_START
,  MY_OPT_LABEL_COLUMN
,  MY_OPT_SKIP_ROW
,  MY_OPT_NJOBS
,  MY_OPT_LOWER
,  MY_OPT_JOBID
}  ;



mcxOptAnchor fpOptions[] =
{  {  "-data"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DATA
   ,  "<fname>"
   ,  "specify input fingerprint data"
   }
,  {  "-co"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_CUTOFF
   ,  "<fraction>"
   ,  "minimum tanimoto value for edge inclusion (default 0.5)"
   }
,  {  "-o"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_OUTPUT
   ,  "<fname>"
   ,  "output file name"
   }
,  {  "-J"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_NJOBS
   ,  "<int>"
   ,  "number of compute jobs overall"
   }
,  {  "--lower-diagonal"
   ,  MCX_OPT_DEFAULT
   ,  MY_OPT_LOWER
   ,  NULL
   ,  "only compute entries (i,j) where i < j"
   }
,  {  "-j"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_JOBID
   ,  "<int>"
   ,  "index of this compute job"
   }
,  {  "-write-tab"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_WRITE_TAB
   ,  "<fname>"
   ,  "write labels to file"
   }
,  {  "-write-hist"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_WRITE_HIST
   ,  "<fname>"
   ,  "write bit histogram to file"
   }
,  {  "-dump"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_DUMP
   ,  "<fname>"
   ,  "commit memory to file"
   }
,  {  "-startc"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_COLUMN_START
   ,  "<num>"
   ,  "bits start at column <num> (default 2)"
   }
,  {  "-l"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_LABEL_COLUMN
   ,  "<num>"
   ,  "labels are stored in column <num>"
   }
,  {  "-load"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_LOAD
   ,  "<fname>"
   ,  "load memory from file"
   }
,  {  "-skipr"
   ,  MCX_OPT_HASARG
   ,  MY_OPT_SKIP_ROW
   ,  "<int>"
   ,  "skip this many lines (rows)"
   }
,  {  NULL, 0, 0, NULL, NULL }
}  ;


static dim progress_g  =  0;
static mcxIO* xfdata = NULL;
static const char* fndump = NULL;
static const char* fnload = NULL;
static mcxIO* xftab = NULL;
static mcxIO* xfhist = NULL;
static const char* out_g = "-";
static unsigned n_row_skip = -1;
static unsigned col_start = -1;
static unsigned label_column = -1;
static dim n_group_G = 1;
static dim i_group = 0;
static double cutoff_g = 1.0;
static mcxbool lowerdiagonal = -1;



static mcxstatus fpInit
(  void
)
   {  progress_g  =  0
   ;  xfdata      =  mcxIOnew("-", "r")
   ;  xftab       =  NULL
   ;  xfhist      =  NULL
   ;  n_row_skip  =  0
   ;  col_start   =  2
   ;  label_column=  1
   ;  cutoff_g    =  0.5
   ;  lowerdiagonal = FALSE
   ;  return STATUS_OK
;  }


static mcxstatus fpArgHandle
(  int optid
,  const char* val
)
   {  switch(optid)
      {  case MY_OPT_DATA
      :  mcxIOnewName(xfdata, val)
      ;  break
      ;

         case MY_OPT_OUTPUT
      :  out_g = val
      ;  break
      ;

         case MY_OPT_LOAD
      :  fnload = val
      ;  break
      ;

         case MY_OPT_LABEL_COLUMN
      :  label_column = atoi(val)
      ;  break
      ;

         case MY_OPT_COLUMN_START
      :  col_start = atoi(val)
      ;  break
      ;

         case MY_OPT_DUMP
      :  fndump = val
      ;  break
      ;

         case MY_OPT_SKIP_ROW
      :  n_row_skip = atoi(val)
      ;  break
      ;

         case MY_OPT_WRITE_TAB
      :  xftab = mcxIOnew(val, "w")
      ;  break
      ;

         case MY_OPT_WRITE_HIST
      :  xfhist = mcxIOnew(val, "w")
      ;  break
      ;

         case MY_OPT_CUTOFF
      :  cutoff_g = atof(val)
      ;  break
      ;

         case MY_OPT_LOWER
      :  lowerdiagonal = TRUE
      ;  break
      ;

         case MY_OPT_JOBID
      :  i_group =  atoi(val)
      ;  break
      ;

         case MY_OPT_NJOBS
      :  n_group_G =  atoi(val)
      ;  break
      ;

         default
      :  mcxExit(1) 
      ;
      }
   ;  return STATUS_OK
;  }



struct chem
{  uint64_t fp[32]
;
}  ;


void* chem_init
(  void* chemp
)
   {  struct chem* chem = chemp
   ;  int i
   ;  for (i=0;i<32;i++)
      chem->fp[i] = 0
   ;  return chemp
;  }


void chem_set_bit
(  struct chem* chem
,  unsigned pos            /* e.g. range 0 to 2047 */
)
   {  int offset = pos / 64
   ;  uint64_t bit    = 1ULL << (pos & 63)
   ;  if (offset >= 32)
      mcxDie(1, "mcx fp", "position %d too large\n", (int) offset)
   ;  chem->fp[offset] |= bit
;if(0)fprintf(stderr, "offset %d bit %llu (bitpos %lu, check %d)\n", (int) offset, (unsigned long long) bit, (unsigned long) pos, (int) (pos & 63))
;  }


void bucket_print
(  uint64_t b
)
   {  int i
   ;  for (i=0;i<8;i++)
      {  unsigned byte = 255 & (b >> (8*i))
      ;  fprintf(stdout, "%s", bytes[byte])
   ;  }
   }


unsigned chem_count_bits
(  struct chem* c
,  int n_buckets
)
   {  int n_bits = 0, i, j
   ;  for (i=0;i<n_buckets;i++)
      {  for (j=0;j<8;j++)
         {  const unsigned int b = 255 & (c->fp[i] >> (j*8))
         ;  n_bits += nbits[b]
;if(0)fprintf(stderr, "%d\n", nbits[b])
      ;  }
   ;  }
      return n_bits
;  }


float chem_tanimoto
(  struct chem* c1
,  struct chem* c2
,  int n_buckets
)
   {  int n_shared = 0
   ;  int n_total  = 0
   ;  int i, j
   ;  for (i=0;i<n_buckets;i++)
      {  for (j=0;j<8;j++)
         {  const unsigned int b1 = 255 & (c1->fp[i] >> (j*8))
         ;  const unsigned int b2 = 255 & (c2->fp[i] >> (j*8))
;if(0)fprintf(stderr, "%d %d shared %d join %d\n", (int) nbits[b1], (int) nbits[b2], (int) nbits[b1 & b2], nbits[b1 | b2])
         ;  n_shared += nbits[b1 & b2]
         ;  n_total  += nbits[b1 | b2]
      ;  }
   ;  }
;if(0)fprintf(stderr, "%d %d\n", (int) n_shared, (int) n_total);
      return n_total ? n_shared * 1.0 / n_total : 0.0
;  }


dim chem_n_buckets
(  dim n_bits
)
   {  dim n_buckets = 0
   ;  if (!n_bits)
      n_bits = 2048

   ;  n_buckets = n_bits / 64

   ;  if (n_bits % 64)
      n_buckets++

   ;  if (n_buckets > 32)
         mcxErr("mcx fp", "limit error for %d (buckets set to 32)", (int) n_bits)
      ,  n_buckets = 32

   ;  return n_buckets
;  }


void chem_print
(  struct chem* chem
,  int n_bits
)
   {  int i, j
   ;  int n_buckets = chem_n_buckets(n_bits)

   ;  for (i=0;i<n_buckets;i++)
      bucket_print(chem->fp[i])

   ;  fputc('\n', stdout)
;  }


void chem_sim
(  struct chem* chems
,  int n_chem
,  dim start
,  dim end
,  int n_bits
,  FILE* fp
)
   {  int i, j, n_shared, n_total
   ;  dim n_buckets = chem_n_buckets(n_bits)
   ;  uint64_t n_pairs = 0, n_batch_pairs = 0, n_batch_hits = 0, n_progress = 1, n_hits = 0
   ;  for (i=start; i<end; i++)
      {  int jstart = lowerdiagonal ? i+1 : 0
      ;  for (j=jstart; j<n_chem; j++)
         {  double tmoto = chem_tanimoto(chems+i, chems+j, n_buckets)
         ;  n_pairs++, n_batch_pairs++
         ;  if (tmoto >= cutoff_g)
               fprintf(fp, "%d\t%d\t%.4f\n", i, j, tmoto)
            ,  n_batch_hits++
      ;  }
#if 0
         if (n_pairs > n_progress * 1000000)
         {  double frac = n_batch_hits * 1.0 / n_batch_pairs
         ;  fprintf(stderr, "%.4f\n", frac)
         ;  n_hits += n_batch_hits
         ;  n_batch_hits = 0
         ;  n_batch_pairs = 0
         ;  n_progress++
      ;  }
#endif
      }
   }


void chem_printall
(  struct chem* chems
,  int n_chem
,  int n_bits
)
   {  int i
   ;  for (i=0;i<n_chem;i++)
      chem_print(chems+i, n_bits)
;  }


void write_hist
(  struct chem* chems
,  int n_chem
,  mcxIO* xf
)
   {  unsigned hist[2049] = { 0 }
   ;  int i = 0
   ;  dim n_buckets = chem_n_buckets(0)

   ;  for (i=0; i< n_chem; i++)
      {  unsigned n_bits = chem_count_bits(chems+i, n_buckets)
      ;  if (n_bits > 2048)
         mcxDie(1, "mcx fp", "too many bits!")
      ;  hist[n_bits]++
   ;  }
      for (i=0; i< 2049; i++)
      fprintf(xf->fp, "%d\t%u\n", (int) i, (unsigned) hist[i])
;  }


static mcxstatus fpMain
(  int          argc_unused      cpl__unused
,  const char*  argv_unused[]    cpl__unused
)
   {  mclx* mx
   ;  mclv* res = NULL
   ;  mcxTing* line = mcxTingEmpty(NULL, 5000)
   ;  dim n_chems_alloc = 10000, i_chem = 0, n_line = 0, i
   ;  struct chem* chems = mcxNAlloc(n_chems_alloc, sizeof chems[0], chem_init, EXIT_ON_FAIL)
   ;  int n_columns = 0, n_rows = 0
   ;  struct chem* map = NULL
   ;  int map_filesize = 0, map_fd = 0

   ;  dim start = 0, end = 0

   ;  double fp    = 0.0, ccmax = 0.0
   ;  mcxIO* xfout =  mcxIOnew(out_g, "w")

   ;  if (n_group_G && i_group >= n_group_G)
      mcxDie(1, "mcx fp", "task error")

   ;  mcxIOopen(xfout, EXIT_ON_FAIL)
   ;  if (xftab)
      mcxIOopen(xftab, EXIT_ON_FAIL)
   ;  if (xfhist)
      mcxIOopen(xfhist, EXIT_ON_FAIL)

#if 0
   ;  progress_g  =  mcx_progress_g
#endif

   ;  if (fnload)
      {  int i
      ;  map_fd = open(fnload, O_RDONLY)
      ;  struct stat mystat
      ;  if (map_fd == -1)
         {  perror("no load file descriptor")
         ;  exit(1)
      ;  }
         if (fstat(map_fd, &mystat))
         {  perror("cannot stat file")
         ;  exit(1)
      ;  }
         map_filesize = mystat.st_size
      ;  map = mmap(0, map_filesize, PROT_READ, MAP_SHARED, map_fd, 0)

      ;  if (map == MAP_FAILED)
         {  close(map_fd)
         ;  perror("cannot read mapped file")
         ;  exit(1);
      ;  }

         chems = map
      ;  i_chem = map_filesize / sizeof chems[0]
      ;  mcxTell("mcx fp", "loading fingerprints for %d compounds", (int) i_chem)
   ;  }
      else           /* default is STDIN */
      while (STATUS_OK == mcxIOreadLine(xfdata, line, MCX_READLINE_CHOMP))
      {  const char* a = line->str
      ;  const char* z = a + line->len
      ;  const char* p = a, *q = NULL
      ;  int n_bit = 0
      ;  int n_tab_seen = 0
      ;  int read_bit = 1

      ;  if (n_row_skip && xfdata->lc <= n_row_skip)
         continue

      ;  while (col_start > 1 + n_tab_seen)
         {  if (!(q = strchr(p, '\t')))
            {  mcxErr("mcx fp", "parse error (skip) at line %d", (int) mcxIOlc(xfdata))
            ;  break
         ;  }
            n_tab_seen++
         ;  if (xftab && label_column == n_tab_seen)
            fprintf(xftab->fp, "%d\t%.*s\n", (int) n_rows++, (int) (q-p), p)
         ;  p = q+1
      ;  }
         if (!q)
         break

;if(0)fprintf(stderr, "(start %d) rest [%.40s]\n", (int) col_start, p)
      ;  while (p < z)
         {  if (read_bit)
            {  read_bit = 0
            ;  if (p[0] != '1' && p[0] != '0')
               {  mcxErr("mcx fp", "parse error (bit) at line %d", (int) mcxIOlc(xfdata))
               ;  break
            ;  }
               if (p[0] == '1')
               chem_set_bit(chems+i_chem, n_bit)
            ;  n_bit++
         ;  } else
            {  if (p[0] != '\t')
               {  mcxErr("mcx fp", "parse error (tab) at line %d", (int) mcxIOlc(xfdata))
               ;  break
            ;  }
               read_bit = 1
         ;  }
            p++
      ;  }
         i_chem++

      ;  if (p != z)
         {  mcxErr("mcx fp", "skipping line %d", (int) mcxIOlc(xfdata))
         ;  chem_init(chems+i_chem-1)
         ;  continue
      ;  }

         if (!n_columns)
         n_columns = n_bit
      ;  else if (n_columns != n_bit)
         mcxDie(1, "mcx fp", "line %d columnd count discrepancy %d %d", (int) xfdata->lc, (int) n_columns, (int) n_bit)

      ;  if (i_chem >= n_chems_alloc)
         {  dim n_alloc_new = n_chems_alloc * 1.41
         ;  chems = mcxNRealloc(chems, n_alloc_new, n_chems_alloc, sizeof chems[0], chem_init, EXIT_ON_FAIL)
         ;  n_chems_alloc = n_alloc_new
      ;  }
      }

      mcxIOclose(xfdata)

   ;  if (xftab)
      mcxIOclose(xftab)

   ;  if (xfhist)
         write_hist(chems, i_chem, xfhist)
      ,  mcxIOclose(xfhist)

   ;  if (fndump)
      {  int fd = open(fndump, O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600)
      ;  uint64_t filesize = i_chem * sizeof chems[0]
      ;  int result, i
      ;  if (fd == -1)
         {  perror("no dump file descriptor")
         ;  exit(1)
      ;  }
         result = lseek(fd, filesize -1, SEEK_SET)
      ;  if (result == -1)
         {  close(fd)
         ;  perror("lseek failed")
         ;  exit(1)
      ;  }
         result = write(fd, "", 1)
      ;  if (result == -1)
         {  close(fd)
         ;  perror("failed to write at end")
         ;  exit(1)
      ;  }
         map = mmap(0, filesize, PROT_WRITE, MAP_SHARED, fd, 0)
      ;  if (result == -1)
         {  close(fd)
         ;  perror("no map")
         ;  exit(1)
      ;  }
         for (i=0; i<i_chem; i++)
         map[i] = chems[i]
      ;  if (munmap(map, filesize) == -1)
         perror("error unmapping")
      ;  close(fd)
      ;  return 0
   ;  }

      if (xftab || fndump)
      return 0

   ;  if (n_group_G)
      {  dim task_size
      ;  if (n_group_G > i_chem)
         n_group_G = i_chem / 2
      ;  task_size = (i_chem + 2 * n_group_G) / (2 * n_group_G)
      ;  if (i_chem % (2 * n_group_G) == 0)
         task_size--
      ;  start = task_size * i_group
      ;  end   = start + task_size
   ;  }
      else
      {  start = 0
      ;  end = i_chem
   ;  }

      if (end > (i_chem+1) / 2)
      end = (i_chem+1) / 2

   ;  if (start < end)
      {  dim start2 = i_chem - end
      ;  dim end2 = i_chem - start
;fprintf(stderr, "I  start end %d %d\n", (int) start, (int) end)
      ;  chem_sim(chems, i_chem, start, end, 0, xfout->fp)
      ;  if (start2 < end)
         start2 = end
      ;  if (start2 < end2)
         chem_sim(chems, i_chem, start2, end2, 0, xfout->fp)
;fprintf(stderr, "II start end %d %d\n", (int) start2, (int) end2)
   ;  }


   ;  if (map && munmap(map, map_filesize) == -1)
      perror("error un-mmapping the file")
   ;  if (map_fd)
      close(map_fd)
   ;  return 0
;  }



mcxDispHook* mcxDispHookFp
(  void
)
   {  static mcxDispHook fpEntry
   =  {  "fp"
      ,  "fp [options]"
      ,  fpOptions
      ,  sizeof(fpOptions)/sizeof(mcxOptAnchor) - 1

      ,  fpArgHandle
      ,  fpInit
      ,  fpMain

      ,  0
      ,  0
      ,  MCX_DISP_MANUAL
      }
   ;  return &fpEntry
;  }


