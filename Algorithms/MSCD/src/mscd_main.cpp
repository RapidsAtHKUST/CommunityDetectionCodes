/************************************************************
 * Includes
*************************************************************/

#include "config.h"

#include <string>
#include <cstring>
#include <map>
#include <ctime>
#include <cstdio>
#include "graph.h"
#include "communities.h"
#include "graph_reader.h"
#include "community_writer.h"
#include "mscd_algorithm.h"
#include "tools.h"
#include "registry.h"

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
	
	fprintf(stdout, "Usage: %s -g graph -c cext -a alg params [-d] [-w] [-p expar] [-v level] [-h]\n", command);
	fprintf(stdout, "  -g: specify input graph file {%s}\n", gformats.c_str());
	fprintf(stdout, "  -c: community file format extension {%s}\n", cformats.c_str());
	fprintf(stdout, "  -a: community detection algorithms {%s} followed by scale parameter values (e.g.[1,2], [1:1:10])\n", algs.c_str());
	fprintf(stdout, "  -d: specifies that the graph is directed (default: undirected)\n");
	fprintf(stdout, "  -p: optional extra parameters for the given algorithm\n");
	fprintf(stdout, "  -w: write (dump) communities to disk as the algorithm runs\n");
	fprintf(stdout, "  -v: verbose level (default=0, max=2)\n");
	fprintf(stdout, "  -h: display this message\n");
}

/************************************************************
 * Entry point
 *************************************************************/

int main (int argc, char * const argv[]) {

	// Get registered graph readers
	toolkit::Registry<mscd::io::in::GraphReader> & gr_registry = toolkit::Registry<mscd::io::in::GraphReader>::GetInstance();
	fprintf(stdout, "%d graph reader(s) registered: ", gr_registry.Count());
	for (int i=0; i<gr_registry.Count(); ++i)
		fprintf(stdout, "*.%s%s ", gr_registry.GetKey(i).c_str(), ((i==gr_registry.Count()-1)?"":","));
	fprintf(stdout, "\n");

	// Get registered community writers
	toolkit::Registry<mscd::io::out::CommunityWriter> & cw_registry = toolkit::Registry<mscd::io::out::CommunityWriter>::GetInstance();
	fprintf(stdout, "%d community writer(s) registered: ", cw_registry.Count());
	for (int i=0; i<cw_registry.Count(); ++i)
		fprintf(stdout, "*.%s%s ", cw_registry.GetKey(i).c_str(), ((i==cw_registry.Count()-1)?"":","));
	fprintf(stdout, "\n");

	// Get registered algorithms
	toolkit::Registry<mscd::alg::MSCDAlgorithm> & alg_registry = toolkit::Registry<mscd::alg::MSCDAlgorithm>::GetInstance();
	fprintf(stdout, "%d algorithm(s) registered: ", alg_registry.Count());
	for (int i=0; i<alg_registry.Count(); ++i)
		fprintf(stdout, "%s%s ", alg_registry.GetKey(i).c_str(), ((i==alg_registry.Count()-1)?"":","));
	fprintf(stdout, "\n");
	
	// Getting configuration
	bool directed = false, dump_communities = false;
	std::string algname = "";
	std::vector<double> ps, ex_par;
	std::string gname = "", cext = "coms";
	int verbose = 0;
	int i=1;
	while (i < argc) {
		// Help
		if (strcmp(argv[i],"-h") == 0) {
			print_usage(argv[0]);
			return EXIT_SUCCESS;
		}
		// Directed/Undirected
		else if (strcmp(argv[i],"-d") == 0) directed = true;
		// Dump communities
		else if (strcmp(argv[i],"-w") == 0) dump_communities = true;
		// Verbose on/off
		else if (strcmp(argv[i],"-v") == 0) {
			if (i+1 < argc) {
				verbose = atoi(argv[++i]);
				if ((verbose < 0) || (verbose > 2)) {
					fprintf(stderr, "Error: Verbose level must be in {0,1,2}\n");
					return EXIT_FAILURE;
				}
			}
			else {
				fprintf(stderr, "Error: Verbose level expected after -v\n");
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
		// Community name
		else if (strcmp(argv[i],"-c") == 0) {
			if (i+1 < argc)
				cext = std::string(argv[++i]);
			else {
				fprintf(stderr, "Error: Community file extension is expected after -c\n");
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
	
	// Look for appropriate community writer
	mscd::io::out::CommunityWriter * writer = cw_registry.Lookup(cext);
	if (!writer) {
		fprintf(stderr, "Error: Unknown community file extension: %s\n", cext.c_str());
		return EXIT_FAILURE;
	}
	
	// Look for appropriate algorithm
	mscd::alg::MSCDAlgorithm * algorithm = alg_registry.Lookup(algname);
	if (!algorithm) {
		fprintf(stderr, "Error: Unknown algorithm: %s\n", algname.c_str());
		return EXIT_FAILURE;
	}
	algorithm->SetVerbose(verbose>0);
	if (dump_communities) algorithm->SetCommunityWriter(writer);
	
	// Creating graph
	mscd::ds::Graph g(directed);
	fprintf(stdout, "Reading graph...\n");
	if (!reader->FromFile(gname, g)) {
		fprintf(stderr, "Error: Cannot read from file %s\n", gname.c_str());
		return EXIT_FAILURE;
	}
	fprintf(stdout, "done\n");
	
	// Output information
	fprintf(stdout, "%s Graph\n", (directed?"Directed":"Undirected"));
	fprintf(stdout, "%ld nodes\n", g.GetNbNodes());
	fprintf(stdout, "%ld edges\n", g.GetNbEdges());
	fprintf(stdout, "Algorithm: %s %s", algname.c_str(), toolkit::NumList2Str<double>(ps).c_str());
	if (!ex_par.empty()) {
		if (algorithm->SetExtraParams(ex_par))
			fprintf(stdout, "\ncalled with extra parameters %s\n", toolkit::NumList2Str<double>(ex_par).c_str());
		else fprintf(stdout, "\n(ignoring extra parameters)\n");
	}
	else fprintf(stdout, "\n");
	
	// Output graph
	if (verbose == 2) {
		for (long i=1, j; i<=g.GetNbNodes(); ++i) {
			if (g.GetNodeNbEdges(i) > 0) fprintf(stdout, "%ld: ", i);
			for (j=1; j<=g.GetNodeNbEdges(i); ++j)
				fprintf(stdout, "%ld ", g.GetTarget(i,j));
			if (g.GetNodeNbEdges(i) > 0) fprintf(stdout, "\n");
		}
	}
	
	// Initialise random number generator
	srand(time(0));
	
	// Run algorithm
	fprintf(stdout, "MSCD running...\n");
	std::vector<mscd::ds::Communities> coms_list;
	std::vector<double> Qs;
	clock_t tinit = time(0), tfinal, cinit = clock(), cfinal;
	if (!algorithm->Run(g, ps, coms_list, Qs)) {
		fprintf(stderr, "Error while running algorithm %s\n", algname.c_str());
		return EXIT_FAILURE;
	}
	cfinal = clock() - cinit;
	tfinal = time(0) - tinit;
	fprintf(stdout, "done in %d(s) / %f(proc s)\n", (int)tfinal, static_cast<double>(cfinal)/CLOCKS_PER_SEC);
	
	// Write/Output communities
	for (long p=0; p<coms_list.size(); ++p) {
		std::stringstream ss;
		ss << gname.substr(0,gname.find_last_of(".")) << "_" << ps[p] << "." << writer->GetName();
		if (!dump_communities) writer->ToFile(ss.str(), coms_list[p]);
		else {
			if (std::rename(coms_list[p].GetComFile().c_str(), ss.str().c_str()) != 0)
				fprintf(stderr, "Error: Could not rename community file %s to %s\n", coms_list[p].GetComFile().c_str(), ss.str().c_str());
			else coms_list[p].SetComFile(ss.str());
		}
		if (verbose == 2) {
			fprintf(stdout, "P=%f Q=%f\n", ps[p], Qs[p]);
			const std::vector<std::vector<long> > & coms = coms_list[p].GetCommunityList();
			if (!dump_communities) {
				for (long c=0, n; c<coms.size(); ++c) {
					fprintf(stdout, "C%ld: ", c+1);
					for (n=0; n<coms[c].size(); ++n) fprintf(stdout, "%ld ", coms[c][n]+1);
					fprintf(stdout, "\n");
				}
			}
			else fprintf(stdout, "Dumped to %s\n", coms_list[p].GetComFile().c_str());
		}
	}
	
	// Write Q value per community set
	std::stringstream ss;
	ss << gname.substr(0,gname.find_last_of(".")) << ".qs";
	FILE * f = fopen(ss.str().c_str(), "w");
	if (!f) return EXIT_FAILURE;
	for (int p=0; p<Qs.size(); ++p)
		fprintf(f, "%f\n", Qs[p]);
	fclose(f);

	// Return OK
    return EXIT_SUCCESS;
}
