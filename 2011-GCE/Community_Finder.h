/*
 * Community_Finder.h
 *
 *  Created on: 04-Dec-2009
 *      Author: freid
 */

#ifndef COMMUNITY_FINDER_H_
#define COMMUNITY_FINDER_H_
#include <iostream>
#include <map>
#include <algorithm>
#include "graph_representation.hpp"
#include "graph_loading.hpp"
#include "cliques.hpp"
#include "Seed.h"
#include <time.h>

using namespace std;

//global graph
extern SimpleIntGraph theGlobalGraph;

extern vector< set<Seed*> > nodeToSeeds;

class Community_Finder {

private:
	void initialiseSeeds(const char * filename, int minimumCliqueSize);

public:
	static float minimumOverlapToMerge;
	static float numberOfTimesRequiredToBeSpokenFor;
	static float spokenForThresholdOfUniqueness;
	
	vector< vector<V> > cliques;

	int removeOverlappingFast();
	int removeOverlapping();
	vector< Seed* > seeds;
	void rawPrint();
	void printSeeds();
	void refreshAllSeedInternalCaches();
	void run();
	void operator () (const vector<V> & clique);
	Community_Finder(const char * filename, int minimumCliqueSize, float minimumOverlapToMerge, float alphaValueForFitness, float numberOfTimesRequiredToBeSpokenFor, float spokenForThresholdOfUniqueness);
	virtual ~Community_Finder();
	void sweepTheDead();
	void doSpokenForPruning();
	

};

#endif /* COMMUNITY_FINDER_H_ */
