/*
 * Seed.h
 *
 *  Created on: 01-Dec-2009
 *      Author: freid
 */

#ifndef SEED_H_
#define SEED_H_
#include <set>
#include "graph_representation.hpp"
#include <map>
#include <math.h>
#include <algorithm>
//This class operates on a single representation of the graph, expected to be in global variable g
extern SimpleIntGraph theGlobalGraph;


class Seed {
public:

    	//responsible for updating the cached edges
	//adds a node, which is assumed not to be contained, and updates the cached internal edges
	void addNodeFromFrontier(V newNode);
	void addNode(V newNode);

	static float minimumOverlapToMerge;
	static float alphaValueForFitness;

	//need a calculate overlap method
	//Calculate the current fitness of the seed.
	float calculateFitness();
	pair <int,int> calculateEdgesIfInsertFromFrontier(V newNode);
	//float calculateFitnessIfInsertFromFrontier(V newNode);
	float calculateFitnessIfInsertFromFrontier(int, int);
	pair <int,int> calculateNumberOfInternalAndExternalEdgesForNodeFromScratch(V theNode);

	void updateFrontierFromScratch();
	void updateCachedEdgeValuesFromScratch();
	bool contains(V theNode);
	bool isEqualTo(Seed & other);
	bool frontierContains(V theNode);
	float addBestNodeFromFrontierToSeed();

	//accessor methods
	int getInternalEdges();
	int getExternalEdges();
	const set<V> & getNodes();

	Seed();
	virtual ~Seed();
	void rawPrint();
	void rawPrintInOrderOfAddition();
	void prettyPrint();
	void prettyPrintFrontier();

	float overlap(Seed & other);
	bool overlapsAlreadyAcceptedSeed();
	int getNumberOfNodes();
	bool dead;

	void clearCaches();
	//map<Seed*, int> overlapWithOtherSeedsInNodes;
	void addNodeNoCaching(V newNode);
	void putIntoNodeToSeedsCache();
	void removeFromNodeToSeedsList();
private:
	//This vector stores the nodes that are in the graph.
	//This vector will be sorted.
	set<V> nodes;

	vector <V> nodesInOrderOfAddition;
	int internalEdges ;
	int externalEdges ;

	bool cachesDirty;

	//Store the internal and external edges of each node that is in the frontier of the seed.
	map <V, pair<int,int> > frontierNodeToInternalAndExternalEdges;
	//Need a map of node to fitness for the frontier.

};

extern vector< set<Seed*> > nodeToSeeds;

#endif /* SEED_H_ */
