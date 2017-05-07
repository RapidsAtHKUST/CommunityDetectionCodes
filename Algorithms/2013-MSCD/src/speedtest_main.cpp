/************************************************************
 * Includes
 *************************************************************/

#include <string>
#include <map>
#include <ctime>
#include <cstdio>
#include "graph.h"
#include "communities.h"
#include "graph_reader.h"
#include "mscd_algorithm.h"
#include "community_writer.h"
#include "tools.h"
#include "registry.h"

/************************************************************
 * Dummy community writer
 *************************************************************/

class DummyCommunityListWriter: public mscd::io::out::CommunityWriter {
public:
    virtual std::string GetName() const { return ""; }
    virtual bool ToFile(const std::string &, const mscd::ds::Communities &) const { return true; }
};

/************************************************************
 * Functions
 *************************************************************/

void print_usage(const char * command) {
	std::string gformats = "";
	toolkit::Registry<mscd::io::in::GraphReader> & gr_registry = toolkit::Registry<mscd::io::in::GraphReader>::GetInstance();
	for (int i=0; i<gr_registry.Count(); ++i) 
		gformats += gr_registry.GetKey(i) + ((i==gr_registry.Count()-1)?"":", ");
    
	std::string cformats = "";
	toolkit::Registry<mscd::io::out::CommunityWriter> & cw_registry = toolkit::Registry<mscd::io::out::CommunityWriter>::GetInstance();
	for (int i=0; i<cw_registry.Count(); ++i)
		cformats += cw_registry.GetKey(i) + ((i==cw_registry.Count()-1)?"":", ");
	
	std::string algs = "";
	toolkit::Registry<mscd::alg::MSCDAlgorithm> & alg_registry = toolkit::Registry<mscd::alg::MSCDAlgorithm>::GetInstance();
	for (int i=0; i<alg_registry.Count(); ++i)
		algs += alg_registry.GetKey(i) + ((i==alg_registry.Count()-1)?"":", ");
	
	fprintf(stdout, "Usage: %s -n nbtests -g graph -a alg params [-d] [-p expar] [-h]\n", command);
    fprintf(stdout, "  -n: specify the number of tests to run\n");
	fprintf(stdout, "  -g: specify input graph file {%s}\n", gformats.c_str());
	fprintf(stdout, "  -a: community detection algorithms {%s} followed by scale parameter values (e.g.[1,2], [1:1:10])\n", algs.c_str());
	fprintf(stdout, "  -d: specifies that the graph is directed (default: undirected)\n");
	fprintf(stdout, "  -p: optional extra parameters for the given algorithm\n");
	fprintf(stdout, "  -h: display this message\n");
}

/************************************************************
 * Entry point
 *************************************************************/

int main (int argc, char * const argv[]) {
	
	// Get registered graph readers
	toolkit::Registry<mscd::io::in::GraphReader> & gr_registry = toolkit::Registry<mscd::io::in::GraphReader>::GetInstance();

	// Get registered algorithms
	toolkit::Registry<mscd::alg::MSCDAlgorithm> & alg_registry = toolkit::Registry<mscd::alg::MSCDAlgorithm>::GetInstance();
	
	// Getting configuration
	bool directed = false;
	std::string algname = "";
	std::vector<double> ps, ex_par;
	std::string gname = "";
	int i=1, nb_tests = 0;
	while (i < argc) {
		// Help
		if (strcmp(argv[i],"-h") == 0) {
			print_usage(argv[0]);
			return EXIT_SUCCESS;
		}
		// Directed/Undirected
		else if (strcmp(argv[i],"-d") == 0) directed = true;
        // Number of tests
		else if (strcmp(argv[i],"-n") == 0) {
			if (i+1 < argc) nb_tests = atoi(argv[++i]);
			else {
				fprintf(stderr, "Error: Number of tests expected after -n\n");
				return EXIT_FAILURE;
			}
		}
		// Graph name
		else if (strcmp(argv[i],"-g") == 0) {
			if (i+1 < argc)
				gname = std::string(argv[++i]);
			else {
				fprintf(stderr, "Error: Graph file is expected after -g\n");
				return EXIT_FAILURE;
			}
		}
		// Algorithm and parameters
		else if (strcmp(argv[i],"-a") == 0) {
			if (i+2 < argc) {
				algname = std::string(argv[++i]);
				if (!alg_registry.Contain(algname)) {
					fprintf(stderr, "Error: Unknown algorithm: %s\n", algname.c_str());
					return EXIT_FAILURE;
				}
				if (!toolkit::ParseRange(std::string(argv[++i]), ps)) {
					fprintf(stderr, "Error: Wrong parameters format\n");
					return EXIT_FAILURE;
				}
			}
			else {
				fprintf(stderr, "Error: Algorithm name and parameters are expected after -a\n");
				return EXIT_FAILURE;
			}
		}
		// Extra parameters
		else if (strcmp(argv[i],"-p") == 0) {
			if (i+1 < argc) {
				if (!toolkit::ParseRange(std::string(argv[++i]), ex_par)) {
					fprintf(stderr, "Error: Wrong extra parameters format\n");
					return EXIT_FAILURE;
				}
			}
			else {
				fprintf(stderr, "Error: Extra parameters are expected after -p\n");
				return EXIT_FAILURE;
			}
		}
		// Check wrong option
		else if (argv[i][0] == '-') {
			fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
			return EXIT_FAILURE;
		}
		// Next command line argument
		++i;
	}
	
	// Check that parameters have been given
	if (ps.empty()) {
		fprintf(stderr, "Error: At least one scale parameter is required\n");
		return EXIT_FAILURE;
	}	
	
	// Look for appropriate file reader
	std::string ext = gname.substr(gname.find_last_of(".")+1);
	mscd::io::in::GraphReader * reader = gr_registry.Lookup(ext);
	if (!reader) {
		fprintf(stderr, "Error: Unknown graph file extension: %s\n", ext.c_str());
		return EXIT_FAILURE;
	}
    else reader->SetVerbose(false);
	
	// Look for appropriate algorithm
	mscd::alg::MSCDAlgorithm * algorithm = alg_registry.Lookup(algname);
	if (!algorithm) {
		fprintf(stderr, "Error: Unknown algorithm: %s\n", algname.c_str());
		return EXIT_FAILURE;
	}
    // Communities are not written to disk nor kept in memory
	algorithm->SetCommunityWriter(new DummyCommunityListWriter());
	
	// Creating graph
	mscd::ds::Graph g(directed);
	fprintf(stdout, "Reading graph...");
	if (!reader->FromFile(gname, g)) {
		fprintf(stderr, "Error: Cannot read from file %s\n", gname.c_str());
		return EXIT_FAILURE;
	}
	fprintf(stdout, " done\n");
	
	// Output information
    fprintf(stdout, "%d tests / %s Graph / %ld nodes / %ld edges\n", nb_tests, (directed?"Directed":"Undirected"), g.GetNbNodes(), g.GetNbEdges());
	fprintf(stdout, "Algorithm: %s %s", algname.c_str(), toolkit::NumList2Str<double>(ps).c_str());
	if (!ex_par.empty()) {
		if (algorithm->SetExtraParams(ex_par))
			fprintf(stdout, " called with extra parameters %s\n", toolkit::NumList2Str<double>(ex_par).c_str());
		else fprintf(stdout, " (ignoring extra parameters)\n");
	}
	else fprintf(stdout, "\n");
	
	// Initialise random number generator
	srand(time(0));
	
	// Run algorithm
	fprintf(stdout, "Tests running"); fflush(stdout);
	std::vector<mscd::ds::Communities> coms_list;
	std::vector<double> Qs;
	clock_t tinit = time(0), tfinal, cinit = clock(), cfinal;
    for (int t=0; t<nb_tests; ++t) {
        coms_list.clear();
        Qs.clear();
        if (!algorithm->Run(g, ps, coms_list, Qs)) {
            fprintf(stderr, "Error while running algorithm %s\n", algname.c_str());
            return EXIT_FAILURE;
        }
        fprintf(stdout, "."); fflush(stdout);
    }
	cfinal = clock() - cinit;
	tfinal = time(0) - tinit;
	fprintf(stdout, " done in %d(s) / %f(proc s)\n", (int)tfinal, static_cast<double>(cfinal)/CLOCKS_PER_SEC);
    fprintf(stdout, "Average=%f(s)\n", static_cast<double>(tfinal)/nb_tests);
    
	// Return OK
    return EXIT_SUCCESS;
}
