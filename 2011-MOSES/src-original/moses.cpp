#include <string.h>
#include <getopt.h>
#include <limits>

#include "graph_loading.hpp"
#include "graph_representation.hpp"
#include "overlapping.hpp"
#include "options.hpp"

long double option_p_in = 0.9;  // THESE ARE IRRELEVANT NOW
long double option_p_out = 1e-2;  // THESE ARE IRRELEVANT NOW
char option_overlapping[1000] = "";
char option_loadOverlapping[1000] = "";
int option_ignoreNlines = 0;
const char gitstatus[] = 
#include "comment.txt"
#include "gitstatus.txt"
;


int main(int argc, char **argv) {
	if(getenv("MOSES_DEBUG")) {
		PP(gitstatus);
		for (int i=0; i<argc; i++) {
			PP(argv[i]);
		}
	}

	{ int c, option_index; while (1)
		{
      static const struct option long_options[] = {
        {"maxDegree", required_argument,         0, 20},
        {"saveMOSESscores", required_argument,         0, 21},
        {"seed", required_argument,       0, 22},
        {0, 0, 0, 0}
      };
      /* getopt_long stores the option index here. */
      c = getopt_long (argc, argv, "", long_options, &option_index);
      if (c == -1) break; /* Detect the end of the options. */
     
      switch (c) {
        case '?': /* getopt_long already printed an error message. */ break;
        default: abort (); break;
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg) printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 20: // {"maxDegree", required_argument,         0, 20},
					option_maxDegree = atoi(optarg);
					break;
        case 21: // --saveMOSESscores= {"saveMOSESscores", required_argument,         0, 21},
					strcpy(option_saveMOSESscores, optarg);
					break;
        case 22: // --seed= {"seed", required_argument,         0, 22},
					option_seed = atol(optarg);
					break;

      }
    }
	}
     
	
		if (argc - optind != 2) {
			cout << "Usage: moses INPUT OUTPUT" << endl;
			cout << " INPUT: list of edges, one edge per line " << endl;
			cout << " OUTPUT: filename to write the grouping. Will contain one line per community. " << endl;
			cout << " [--seed 0]: seed for the random number generator. default 0. " << endl;
			cout << " [--saveMOSESscores FILENAME]: store the delta-objective for each community here (log-ratio of the MOSES posterior density). " << endl;
			cout << endl;
			cout << "MOSES v2011-01-26b: This software is made available under the Apache License 2.0. " << endl;
			cout << "The software is provided \"as is\" without express or implied warranty. If you use this software in your research, we encourage you to cite the associated paper: " << endl;
			cout << endl;
			cout << "- Aaron McDaid, Neil Hurley. Detecting highly overlapping communities with Model-based Overlapping Seed Expansion. ASONAM 2010. http://irserver.ucd.ie/dspace/handle/10197/2404 " << endl;
			cout << endl;
			cout << "       http://clique.ucd.ie/software and http://sites.google.com/site/aaronmcdaid/moses " << endl;
			cout << endl;
			cout << "This research was supported by Science Foundation Ireland (SFI) Grant No. 08/SRC/I1407 - Clique Research Cluster. " << endl;
			cout << endl;
			exit(1);
		}
		strcpy(option_overlapping, argv[optind+1]);
		setenv("Lookahead","2",1);
		SimpleStringGraph theGraph;
		graph_loading::loadSimpleStringGraphFromFile(theGraph, argv[optind]);
		{
			PP(theGraph.vcount());
			PP(theGraph.ecount());
			V maxdegree = 0;
			V mindegree = numeric_limits<V>::max();
			for(V v = 0; v<(V) theGraph.vcount(); v++) {
				maxdegree = max(maxdegree, (V) theGraph.degree(v));
				mindegree = min(mindegree, (V) theGraph.degree(v));
			}
			PP(mindegree);
			PP(maxdegree);
			double average_degree = 2.0 * theGraph.ecount() / theGraph.vcount();
			PP(average_degree);
			P("\n");
		}
		overlapping::flag_save_group_id_in_output = false;
		overlapping::overlapping(theGraph);
}
