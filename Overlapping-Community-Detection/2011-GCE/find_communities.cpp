#include <iostream>
#include <map>
#include <algorithm>
#include "graph_representation.hpp"
#include "graph_loading.hpp"
#include "cliques.hpp"
#include "Seed.h"
#include "Community_Finder.h"
#include <string.h>
using namespace std;



//global graph
SimpleIntGraph theGlobalGraph;
//This vector is a map of each node to the set of seeds that are in it.
vector< set<Seed*> > nodeToSeeds;

int main (int argc, char **argv) {
	cerr << "Greedy Clique Expansion Community Finder" << endl;
	if(argc!=6 && (argc!=2 || ((strcmp (argv[1],"--?")) == 0) || ((strcmp (argv[1],"--help")) == 0)|| ((strcmp (argv[1],"-h")) == 0)  )) {
		cout << "Community finder. Requires edge list of nodes. Processes graph in undirected, unweighted form. Edgelist must be two values separated with non digit character." << endl<<endl;
		cout << "Use with either full (if specify all 5) or default (specify just graph file) parameters:" << endl;
		cout << "Full parameters are:" << endl;
		cout << "The name of the file to load" << endl;
		cout << "The minimum size of cliques to use as seeds. Recommend 4 as default, unless particularly small communities are required (in which case use 3)." << endl;
		cout << "The minimum value for one seed to overlap with another seed before it is considered sufficiently overlapping to be discarded (eta). 1 is complete overlap. However smaller values may be used to prune seeds more aggressively. A value of 0.6 is recommended." << endl;
		cout << "The alpha value to use in the fitness function greedily expanding the seeds. 1.0 is recommended default. Values between .8 and 1.5 may be useful. As the density of edges increases, alpha may need to be increased to stop communities expanding to engulf the whole graph. If this occurs, a warning message advising that a higher value of alpha be used, will be printed." << endl;
		cout << "The proportion of nodes (phi) within a core clique that must have already been covered by other cliques, for the clique to be 'sufficiently covered' in the Clique Coveage Heuristic" << endl;
		cout << endl<< "Usage: " << argv[0] << " graphfilename minimumCliqueSizeK overlapToDiscardEta fitnessExponentAlpha CCHthresholdPhi" << endl;
		cout << endl<< "Usage (with defaults): " << argv[0] << " graphfilename" << endl;
		cout << "This will run with the default values of: minimumCliqueSizeK 4, overlapToDiscardEta 0.6, fitnessExponentAlpha 1.0, CCHthresholdPhi .75" << endl;
		cout << "Communities will be output, one community per line, with the same numbering as the original nodes were provided." << endl;
		exit(1);
	}

	const char * filename = argv[1];
	
	
	int minimumCliqueSize = 4;
	float minimumOverlapToMerge = 0.6;
	float alphaValueForFitness = 1.0;
	float numberOfTimesRequiredToBeSpokenFor = 1;
	float spokenForThresholdOfUniqueness = .75;
	if (argc==6)
	{
		minimumCliqueSize = atoi(argv[2]);
		minimumOverlapToMerge = atof(argv[3]);
		alphaValueForFitness = atof(argv[4]);
		spokenForThresholdOfUniqueness = atof(argv[5]);
	}

	cerr <<"Running with parameters: k: " << minimumCliqueSize << " eta: " << minimumOverlapToMerge << " alpha: " << alphaValueForFitness << " Phi: " << spokenForThresholdOfUniqueness << endl;


	cerr << "Loading file: " << filename << endl;
	Community_Finder communityFinder(filename, minimumCliqueSize, minimumOverlapToMerge, alphaValueForFitness, numberOfTimesRequiredToBeSpokenFor, spokenForThresholdOfUniqueness );

	cerr << "Edges in loaded graph:\t" << theGlobalGraph.ecount() << endl;
	cerr << "Nodes in loaded graph:\t" << theGlobalGraph.vcount() << endl;
	communityFinder.run();

	//communityFinder.printSeeds();

	cerr << "Finished\n";

	communityFinder.rawPrint();
/*
// print node->seed assignments
	int count = 0;
	for( vector< set<Seed*> >::iterator nodeToSeedsItr = nodeToSeeds.begin(); nodeToSeedsItr != nodeToSeeds.end(); ++nodeToSeedsItr)
	{
		cout << "Node " << count++ << " : ";
		for( set<Seed*> ::iterator innerItr = (*nodeToSeedsItr).begin(); innerItr != (*nodeToSeedsItr).end(); ++innerItr)
		{
			 (*innerItr)->prettyPrint();

		}
		cout << endl;
	}
*/
}
