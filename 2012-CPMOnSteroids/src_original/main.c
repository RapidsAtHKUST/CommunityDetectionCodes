#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "include/cliques.h"
#include "include/dsforest.h"
#include "include/cos.h"
#include "include/cospoc.h"
#include "include/debug.h"

cliques* all_cliques;
int k_max;
dsforest_t **global_dsf;
pthread_mutex_t *global_dsf_mutexes;
int NUM_THREADS = 8;			// Number of parallel threads
unsigned long int WINDOW_SIZE = 1ULL<<30; // sliding window size
int debug_level=0;

static void usage(const char *progname);

int main(int argc, char** argv) {
  int cos_flag = 1; // default method is cos
  //int cospoc_flag = 0;
  char *cliquelist_file = NULL;
  int c,ret=-1;
  while ((c = getopt (argc, argv, "pdW:P:h")) != -1){
    switch (c) {
    case 'p': // you want to use cospoc rather than cos ...
      //cospoc_flag = 1;
      cos_flag = 0;
      break;
    case 'd':
      // each time -d option is passed debug_level is increased by 1
#ifdef DEBUG
      debug_level++;
#else
      printf("Warning, debug not enabled\n");
#endif      
      break;
    case 'W':
      WINDOW_SIZE = 1ULL<<atoi(optarg);
      break;
    case 'P':
      NUM_THREADS = atoi(optarg);
      break;
    case 'h':
      usage(argv[0]);
      exit(EXIT_FAILURE);
    case '?':
      usage(argv[0]);
      exit(EXIT_FAILURE);
    default:
      abort();
    }
  }

  // non-option argument has not been given or more than one have been
  if(optind != argc-1) {
    usage(argv[0]);
    return(EXIT_FAILURE);
  }
  cliquelist_file = argv[argc-1];

  cliques_init(&all_cliques);
  cliques_load_unordered_maximal_cliques_list(all_cliques, cliquelist_file);
  k_max=all_cliques->k_max;



  if (cos_flag)
    ret = cos_();
  else //cospoc_flag
    ret = cospoc();
  if(ret)
    exit(EXIT_FAILURE);
  exit(EXIT_SUCCESS);
}

#ifdef DEBUG
void
debug__(int level, const char *fmt, ...)
{
	va_list	ap;
	int savederrno;
	
	/* only log if the level is high enough */
	if (debug_level < level)
		return;

	savederrno = errno;
	va_start(ap, fmt);

        vfprintf(stderr, fmt, ap);
        fputs("\n", stderr);

	va_end(ap);
	errno = savederrno;
}
#endif /* DEBUG */


static void usage(const char* progname){
  printf("Usage:\n");
#ifdef DEBUG
  printf("%s [-P <num_threads>] [-W <exp>] [-p] [-d] <mxcliques_file>\n", progname);
#else
  printf("%s [-P <num_threads>] [-W <exp>] [-p] <mxcliques_file>\n", progname);
#endif
  printf("%s -h\n\n", progname);

  printf("Where:\n");
  printf(" -P <num_threads>  Specifies the number of parallel threads to be executed.\n"
         "                   Default value is 8.\n");
  printf(" -W <exp>          Set the sliding window buffer size to 2**<exp> Bytes.\n"
         "                   Default value is 30 for a sliding window buffer size of 2**30 B = 1 GB.\n");
  printf(" -p                Specifies to use the proof-of-concept method COSpoc rather than COS.\n"
         "                   With this option, parameter -W is ignored.\n");
  printf(" -h                Prints this help and exits.\n");
#ifdef DEBUG
  printf(" -d                Set the debug level. This parameter can be passed from zero (no debug) to\n"
         "                   four (maximum verbosity level) times. Each time the parameter is passed,\n"
         "                   the verbosity level of the debug is increased by one.\n");
#endif
  printf(" <mxcliques_file>  A file containing a list of maximal cliques, one per line. Each maximal\n"
         "                   clique must be expressed as a whitespace-separated list of its nodes \n"
         "                   and must end with a virtual node -1. Nodes must be expressed as non-negative\n"
         "                   integer numbers.\n");

  printf("\n");
  printf("Output:\n");
  printf("For each k such that at least one k-clique community exists, a file named\n"
         "k_comunities.txt is output in the current working directory.\n"
         "Each line of the file has the format \"<community_id>:<maximal_clique>\", where:\n"
         " <community_id>    Is an integer number ranging from 0 to (Nk-1), where Nk is the number of k-clique\n"
         "                   communities discovered for a given k. This integer uniquely identifies the community.\n"
         " <maximal_clique>  Is a whistespace-separated list of nodes belonging to one input maximal clique,\n"
         "                   which is part of k-clique community identified by <community_id>\n\n");
  printf("In addition, a summary file containing the number of maximal cliques discovered for each k is output as well.\n"
         "Each line of this file, named k_num_communities.txt, has the format \"k  Nk\", where Nk is the number\n"
         "of k-clique communities discovered for a given k.\n");
  printf("\n");
  printf("Examples:\n");
  printf("%s <mxcliques_file>\n", progname);
  printf("      Run the program with 8 parallel threads and a 1 GB sliding window buffer.\n\n");
  printf("%s -P 80 <mxcliques_file>\n", progname);
  printf("      Run the program with 80 parallel threads.\n\n");
  printf("%s -P 80 -W 32 <mxcliques_file>\n", progname);
  printf("      Run the program with 80 parallel threads and a 2**32 B = 4 GB sliding window buffer.\n\n");
#ifdef DEBUG
  printf("%s -d <mxcliques_file>\n", progname);
  printf("      Run the program with minimum debug info.\n\n");
  printf("%s -d -d -d -d <mxcliques_file>\n", progname);
  printf("      Run the program with maximum debug info.\n\n");
#endif  
  printf("%s -p <mxcliques_file>\n", progname);
  printf("      Run the program with 8 parallel threads and a 1 GB sliding window buffer, using\n"
         "      the proof-of-concept method COSpoc.\n\n");

  return;
}
