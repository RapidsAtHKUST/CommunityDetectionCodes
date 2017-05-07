/************************************************************
 * Includes
 *************************************************************/

#include <string>
#include <cstdio>
#include <deque>
#include <cmath>
#include "communities.h"
#include "community_reader.h"
#include "registry.h"
#include "tools.h"
#include "nmi.h"

/************************************************************
 * Functions
 *************************************************************/

void print_usage(const char * command) {
	std::string cinformats = "";
	toolkit::Registry<mscd::io::in::CommunityReader> & cin_registry = toolkit::Registry<mscd::io::in::CommunityReader>::GetInstance();
	for (int i=0; i<cin_registry.Count(); ++i) 
		cinformats += cin_registry.GetKey(i) + ((i==cin_registry.Count()-1)?"":", ");
	
	fprintf(stdout, "Usage: %s -i pfxname ext -p params -n nb_nodes -s steps -c crefs [-g]\n", command);
	fprintf(stdout, "  -i: input files prefix\n");
	fprintf(stdout, "  -p: scale parameter list\n");
	fprintf(stdout, "  -n: number of nodes in the graph\n");
	fprintf(stdout, "  -s: number of successive communities to consider for NMI analysis (list of values)\n");
	fprintf(stdout, "  -c: specifies reference communities to compare NMI with (list of file names)\n");
	fprintf(stdout, "  -g: uses the generalised NMI instead of NMI (required for overlapping communities)\n");
	fprintf(stdout, "  -h: display this message\n");
	fprintf(stdout, "  available community formats: {%s}\n", cinformats.c_str());
}

/************************************************************
 * Entry point
 *************************************************************/

int main (int argc, char * const argv[]) {
	
	// Getting configuration
	std::string pfxname = "", ext="";
	std::vector<std::string> crefs;
	std::vector<int> sucs;
	std::vector<double> ps;
	long nb_nodes = 0;
	bool gen = false;
	int i=1;
	while (i < argc) {
		// Help
		if (strcmp(argv[i],"-h") == 0) {
			print_usage(argv[0]);
			return EXIT_SUCCESS;
		}
		// Input name prefix
		else if (strcmp(argv[i],"-i") == 0) {
			if (i+1 < argc)
				pfxname = std::string(argv[++i]);
			else {
				fprintf(stderr, "Error: Name prefix and extension are expected after -i\n");
				return EXIT_FAILURE;
			}
			if (i+1 < argc)
				ext = std::string(argv[++i]);
			else {
				fprintf(stderr, "Error: Name prefix and extension are expected after -i\n");
				return EXIT_FAILURE;
			}
		}
		// Input name prefix
		else if (strcmp(argv[i],"-p") == 0) {
			if (i+1 < argc) {
				if (!toolkit::ParseRange(std::string(argv[++i]), ps)) {
					fprintf(stderr, "Error: Wrong parameters format\n");
					return EXIT_FAILURE;
				}
			}
			else {
				fprintf(stderr, "Error: Scale parameter list is expected after -p\n");
				return EXIT_FAILURE;
			}
		}
		// Number of nodes
		else if (strcmp(argv[i],"-n") == 0) {
			if (i+1 < argc) nb_nodes = std::atoi(argv[++i]);
			else {
				fprintf(stderr, "Error: Number of nodes is expected after -n\n");
				return EXIT_FAILURE;
			}
		}
		// Reference communities
		else if (strcmp(argv[i],"-c") == 0) {
			while ((i+1 < argc) && (argv[i+1][0] != '-'))
				crefs.push_back(std::string(argv[++i]));
			if (crefs.empty()) {
				fprintf(stderr, "Error: Reference communities are expected after -c\n");
				return EXIT_FAILURE;
			}
		}
		// NMI analysis steps
		else if (strcmp(argv[i],"-s") == 0) {
			while ((i+1 < argc) && (argv[i+1][0] != '-'))
				sucs.push_back(std::atoi(argv[++i]));
			if (!sucs.empty()) std::sort(sucs.begin(), sucs.end());
		}
		// Generalised NMI
		else if (strcmp(argv[i],"-g") == 0) gen = true;
		// Check wrong option
		else if (argv[i][0] == '-') {
			fprintf(stderr, "Error: Unknown option %s\n", argv[i]);
			return EXIT_FAILURE;
		}
		// Next command line argument
		++i;
	}
	
	// Check command
	if ((pfxname=="") || ps.empty()) {
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	if ((!sucs.empty() || !crefs.empty()) && (nb_nodes == 0)) {
		fprintf(stderr, "Number of nodes is required for NMI computation\n");
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}
	
	// Print request
	fprintf(stdout, "Analysing results of '%s' with extension '%s'\n", pfxname.c_str(), ext.c_str());
	fprintf(stdout, "#scale parameters %s\n",  toolkit::NumList2Str<double>(ps).c_str());
	fprintf(stdout, "#number of nodes %ld\n",  nb_nodes);
	if (!sucs.empty())
		fprintf(stdout, "#successive steps %s\n",  toolkit::NumList2Str<int>(sucs).c_str());
	if (!crefs.empty())
		fprintf(stdout, "#reference communities %s\n", toolkit::NumList2Str<std::string>(crefs).c_str());
	
	// Look for appropriate community reader
	toolkit::Registry<mscd::io::in::CommunityReader> & in_registry = toolkit::Registry<mscd::io::in::CommunityReader>::GetInstance();
	mscd::io::in::CommunityReader * reader = in_registry.Lookup(ext);
	if (!reader) {
		fprintf(stderr, "Error: Unknown community file extension: %s\n", ext.c_str());
		return EXIT_FAILURE;
	}
	
	// Read reference communities
	std::vector<mscd::ds::Communities> comsref(crefs.size());
	for (int i=0; i<crefs.size(); ++i) {
		if (!reader->FromFile(crefs[i], comsref[i])) {
			fprintf(stderr, "Error: Could not read community file %s\n", crefs[i].c_str());
			return EXIT_FAILURE;
		}
	}
	
	// Process communities
	mscd::ds::Communities coms;
	std::vector<long> nb_coms(ps.size());
	std::deque<mscd::ds::Communities> qcoms;
	std::vector< std::vector<double> > nmi_dps(sucs.size());
	for (int i=0; i<sucs.size(); ++i) nmi_dps[i].resize(ps.size());
	std::vector< std::vector<double> > nmi_ref(crefs.size());
	for (int i=0; i<crefs.size(); ++i) nmi_ref[i].resize(ps.size());
	for (int p=0; p<ps.size(); ++p) {
		//printf("P=%f\n",ps[p]);
		// Get current community
		if (!qcoms.empty()) {
			coms.Swap(qcoms.front());
			qcoms.pop_front();
		}
		else {
			std::stringstream ss; ss << pfxname << "_" << ps[p] << "." << ext;
			if (!reader->FromFile(ss.str(), coms)) {
				fprintf(stderr, "Error: Could not read community file %s\n", ss.str().c_str());
				return EXIT_FAILURE;
			}
		}
		//printf("done\n");
		
		// Store current number of communities
		nb_coms[p] = coms.GetNbCommunities();
		
		// If NMI with successive partitions is required
		if (!sucs.empty()) {
			// Keep/load sequence of communities loaded
			while ((qcoms.size() < sucs.back()) && (p+qcoms.size()+1 < ps.size())) {
				std::stringstream ss; ss << pfxname << "_" << ps[p+qcoms.size()+1] << "." << ext;
				qcoms.push_back(mscd::ds::Communities());
				if (!reader->FromFile(ss.str(), qcoms.back())) {
					fprintf(stderr, "Error: Could not read community file %s\n", ss.str().c_str());
					return EXIT_FAILURE;
				}
			}
			// Compute NMI
			double sum_nmi_dp = 0.;
			for (int i=0, k=0; i<sucs.size(); ++i) {
				//printf("i=%d k=%d\n", i,k);
				//if (k<qcoms.size())
				//	printf("NMI c1=%ld c2=%ld\n", coms.GetNbCommunities(), qcoms[k].GetNbCommunities());
				//else
				//	printf("NMI c1=%ld c2=NOOO\n", coms.GetNbCommunities());
				while ((k<sucs[i]) && (k<qcoms.size()))
					sum_nmi_dp += toolkit::NMI(coms, qcoms[k++], nb_nodes, gen);
				if (!qcoms.empty())
					nmi_dps[i][p] = sum_nmi_dp/std::min(sucs[i],static_cast<int>(qcoms.size()));
				else nmi_dps[i][p] = 0.;
			}
		}
		
		//printf("CREF\n");
		// If NMI with reference partitions is required
		if (!crefs.empty())
			for (int i=0; i<crefs.size(); ++i)
				nmi_ref[i][p] = toolkit::NMI(coms, comsref[i], nb_nodes, gen);
		//printf("done\n");
	}
	
	// Write the number of communities file
	fprintf(stdout, "Writing number of communities file...\n");
	{
		std::stringstream ss;
		ss << pfxname << ".nc";
		FILE * f = fopen(ss.str().c_str(), "w");
		if (!f) return false;
		for (long p=0; p<nb_coms.size(); ++p)
			fprintf(f, "%ld\n", nb_coms[p]);
		fclose(f);
	}
	
	// Write the NMI with successive sets files
	if (!nmi_dps.empty())
		fprintf(stdout, "Writing NMI with successive communities files...\n");
	for (int i=0; i<nmi_dps.size(); ++i) {
		std::stringstream ss;
		ss << pfxname << "_nmi_dp_" << sucs[i] << ".nmi";
		FILE * f = fopen(ss.str().c_str(), "w");
		if (!f) return false;
		for (int p=0; p<nmi_dps[i].size(); ++p)
			fprintf(f, "%f\n", nmi_dps[i][p]);
		fclose(f);
	}
	
	// Write the NMI with reference partitions files
	if (!nmi_ref.empty())
		fprintf(stdout, "Writing NMI with reference communities files...\n");
	for (int i=0; i<nmi_ref.size(); ++i) {
		std::stringstream ss;
		std::string crpfx = crefs[i].substr(0,crefs[i].find_last_of("."));
		ss << pfxname << "_nmi_cr_" << crpfx << ".nmi";
		FILE * f = fopen(ss.str().c_str(), "w");
		if (!f) return false;
		for (int p=0; p<nmi_ref[i].size(); ++p)
			fprintf(f, "%f\n", nmi_ref[i][p]);
		fclose(f);
	}
	
    // Return OK
	return EXIT_SUCCESS;
}
