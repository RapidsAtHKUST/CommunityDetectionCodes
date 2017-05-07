/************************************************************
 * Includes
 *************************************************************/

#include <string>
#include <cstdio>
#include "graph.h"
#include "graph_reader.h"
#include "graph_writer.h"
#include "communities.h"
#include "community_reader.h"
#include "community_writer.h"
#include "registry.h"

/************************************************************
 * Functions
 *************************************************************/

void print_usage(const char * command) {
	std::string ginformats = "";
	toolkit::Registry<mscd::io::in::GraphReader> & gin_registry = toolkit::Registry<mscd::io::in::GraphReader>::GetInstance();
	for (int i=0; i<gin_registry.Count(); ++i) 
		ginformats += gin_registry.GetKey(i) + ((i==gin_registry.Count()-1)?"":", ");

	std::string goutformats = "";
	toolkit::Registry<mscd::io::out::GraphWriter> & gout_registry = toolkit::Registry<mscd::io::out::GraphWriter>::GetInstance();
	for (int i=0; i<gout_registry.Count(); ++i) 
		goutformats += gout_registry.GetKey(i) + ((i==gout_registry.Count()-1)?"":", ");

	std::string cinformats = "";
	toolkit::Registry<mscd::io::in::CommunityReader> & cin_registry = toolkit::Registry<mscd::io::in::CommunityReader>::GetInstance();
	for (int i=0; i<cin_registry.Count(); ++i) 
		cinformats += cin_registry.GetKey(i) + ((i==cin_registry.Count()-1)?"":", ");
	
	std::string coutformats = "";
	toolkit::Registry<mscd::io::out::CommunityWriter> & cout_registry = toolkit::Registry<mscd::io::out::CommunityWriter>::GetInstance();
	for (int i=0; i<cout_registry.Count(); ++i) 
		coutformats += cout_registry.GetKey(i) + ((i==cout_registry.Count()-1)?"":", ");

	fprintf(stdout, "Usage: %s {-g|-c} -i in -o out\n", command);
	fprintf(stdout, "  -g: convert a graph file\n");
	fprintf(stdout, "      input formats : {%s}\n", ginformats.c_str());
	fprintf(stdout, "      output formats: {%s}\n", goutformats.c_str());
	fprintf(stdout, "  -c: convert a community file\n");
	fprintf(stdout, "      input formats : {%s}\n", cinformats.c_str());
	fprintf(stdout, "      output formats: {%s}\n", coutformats.c_str());
	fprintf(stdout, "  -i: input file\n");
	fprintf(stdout, "  -o: output file\n");
	fprintf(stdout, "  -d: specifies that graph is directed when converting a graph (default: undirected)\n");
	fprintf(stdout, "  -h: display this message\n");
}

/************************************************************
 * Entry point
 *************************************************************/

int main (int argc, char * const argv[]) {
	
	// Getting configuration
	bool convert_graph, directed = false;
	std::string fin = "", fout = "";
	int i=1;
	while (i < argc) {
		// Help
		if (strcmp(argv[i],"-h") == 0) {
			print_usage(argv[0]);
			return EXIT_SUCCESS;
		}
		// Graph
		else if (strcmp(argv[i],"-g") == 0) convert_graph = true;
		// Community
		else if (strcmp(argv[i],"-c") == 0) convert_graph = false;
		// Directed/Undirected
		else if (strcmp(argv[i],"-d") == 0) directed = true;
		// Input name
		else if (strcmp(argv[i],"-i") == 0) {
			if (i+1 < argc)
				fin = std::string(argv[++i]);
			else {
				fprintf(stderr, "Error: Input file is expected after -i\n");
				return EXIT_FAILURE;
			}
		}
		// Output name
		else if (strcmp(argv[i],"-o") == 0) {
			if (i+1 < argc)
				fout = std::string(argv[++i]);
			else {
				fprintf(stderr, "Error: Output file is expected after -o\n");
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
	
	// Check command
	if ((fin=="") || (fout=="")) {
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	
	// Get extensions
	std::string extin = fin.substr(fin.find_last_of(".")+1),
				extout = fout.substr(fout.find_last_of(".")+1);
	
	// If converting a graph
	if (convert_graph) {
		// Get registered readers and writers
		toolkit::Registry<mscd::io::in::GraphReader> & in_registry = toolkit::Registry<mscd::io::in::GraphReader>::GetInstance();
		toolkit::Registry<mscd::io::out::GraphWriter> & out_registry = toolkit::Registry<mscd::io::out::GraphWriter>::GetInstance();
		
		// Look for appropriate file reader
		mscd::io::in::GraphReader * reader = in_registry.Lookup(extin);
		if (!reader) {
			fprintf(stderr, "Error: Unknown graph file extension: %s\n", extin.c_str());
			return EXIT_FAILURE;
		}
		
		// Look for appropriate file writer
		mscd::io::out::GraphWriter * writer = out_registry.Lookup(extout);
		if (!writer) {
			fprintf(stderr, "Error: Unknown graph file extension: %s\n", extout.c_str());
			return EXIT_FAILURE;
		}
		
		// Reading graph
		mscd::ds::Graph g(directed);
		fprintf(stdout, "Reading graph from %s\n", fin.c_str());
		if (!reader->FromFile(fin, g)) {
			fprintf(stderr, "Error: Cannot read from file %s\n", fin.c_str());
			return EXIT_FAILURE;
		}
		
		// Writing output
		fprintf(stdout, "Writing graph to %s\n", fout.c_str());
		if (!writer->ToFile(fout, g)) {
			fprintf(stderr, "Error: Cannot write to file %s\n", fout.c_str());
			return EXIT_FAILURE;
		}
	}
	// Otherwise converting communities
	else {
		// Get registered readers and writers
		toolkit::Registry<mscd::io::in::CommunityReader> & in_registry = toolkit::Registry<mscd::io::in::CommunityReader>::GetInstance();
		toolkit::Registry<mscd::io::out::CommunityWriter> & out_registry = toolkit::Registry<mscd::io::out::CommunityWriter>::GetInstance();
		
		// Look for appropriate file reader
		mscd::io::in::CommunityReader * reader = in_registry.Lookup(extin);
		if (!reader) {
			fprintf(stderr, "Error: Unknown community file extension: %s\n", extin.c_str());
			return EXIT_FAILURE;
		}
		
		// Look for appropriate file writer
		mscd::io::out::CommunityWriter * writer = out_registry.Lookup(extout);
		if (!writer) {
			fprintf(stderr, "Error: Unknown community file extension: %s\n", extout.c_str());
			return EXIT_FAILURE;
		}
		
		// Reading communities
		mscd::ds::Communities c;
		fprintf(stdout, "Reading communities from %s\n", fin.c_str());
		if (!reader->FromFile(fin, c)) {
			fprintf(stderr, "Error: Cannot read from file %s\n", fin.c_str());
			return EXIT_FAILURE;
		}
		
		// Writing output
		fprintf(stdout, "Writing communities to %s\n", fout.c_str());
		if (!writer->ToFile(fout, c)) {
			fprintf(stderr, "Error: Cannot write to file %s\n", fout.c_str());
			return EXIT_FAILURE;
		}
	}
	
    // Return OK
	return EXIT_SUCCESS;
}
