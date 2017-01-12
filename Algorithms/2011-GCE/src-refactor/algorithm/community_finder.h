/*
 * Community_Finder.h
 *
 *  Created on: 04-Dec-2009
 *      Author: freid
 */

#ifndef COMMUNITY_FINDER_H_
#define COMMUNITY_FINDER_H_

#include <time.h>

#include <iostream>
#include <map>
#include <algorithm>

#include "../util/graph/graph_representation.hpp"
#include "../util/graph/graph_loading.hpp"
#include "cliques.hpp"
#include "seed.h"

using namespace std;

//global graph
extern SimpleIntGraph theGlobalGraph;

extern vector<set<Seed *> > nodeToSeeds;

class community_finder {

private:
    void initialiseSeeds(const char *filename, int minimumCliqueSize);

public:
    static float minimumOverlapToMerge;
    static float numberOfTimesRequiredToBeSpokenFor;
    static float spokenForThresholdOfUniqueness;

    vector<vector<V> > cliques;

    int removeOverlappingFast();

    int removeOverlapping();

    vector<Seed *> seeds;

    void rawPrint();

    void printSeeds();

    void refreshAllSeedInternalCaches();

    void run();

    void operator()(const vector<V> &clique);

    community_finder(const char *filename, int minimumCliqueSize, float minimumOverlapToMerge,
                     float alphaValueForFitness, float numberOfTimesRequiredToBeSpokenFor,
                     float spokenForThresholdOfUniqueness);

    virtual ~community_finder();

    void sweepTheDead();

    void doSpokenForPruning();


};

#endif /* COMMUNITY_FINDER_H_ */
