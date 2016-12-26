/*
 * Community_Finder.cpp
 *
 *  Created on: 04-Dec-2009
 *      Author: freid
 */

//Release canidate.
#include "Community_Finder.h"

//TIMEKEEPING
time_t t0 = clock();;

//REPORTING
#define REPORTING_OUTPUT_FREQUENCY 1000

//PARAMETER STORAGE
float Community_Finder::minimumOverlapToMerge;
float Community_Finder::numberOfTimesRequiredToBeSpokenFor;
float Community_Finder::spokenForThresholdOfUniqueness;
//SORT PREDICATES



//Note - flipped order in sort function - returns them sorted in reverse order - biggest first
bool sizeSortFunctionLargestFirst (Seed * two,  Seed * one)
{
	return one->getNumberOfNodes() < two->getNumberOfNodes();
}

bool sizeSortFunctionSmallestFirst (Seed * one,  Seed * two)
{
	return one->getNumberOfNodes() < two->getNumberOfNodes();
}

bool vectorSizeSortFunctionLargestFirst (vector <V> two,  vector <V> one)
{
        return one.size() < two.size();
}
bool vectorSizeSortFunctionSmallestFirst (vector <V> one,  vector <V> two)
{
        return one.size() < two.size();
}
//END SORT PREDICATES

//CONSTRUCTURORS

Community_Finder::Community_Finder(const char * filename, int minimumCliqueSize, float minimumOverlapToMerge, float alphaValueForFitness, float numberOfTimesRequiredToBeSpokenFor, float spokenForThresholdOfUniqueness) {
	Community_Finder::minimumOverlapToMerge = minimumOverlapToMerge;
	Seed::minimumOverlapToMerge = minimumOverlapToMerge;
	Seed::alphaValueForFitness = alphaValueForFitness;
	Community_Finder::numberOfTimesRequiredToBeSpokenFor = numberOfTimesRequiredToBeSpokenFor;
	Community_Finder::spokenForThresholdOfUniqueness = spokenForThresholdOfUniqueness;
	
	this->initialiseSeeds(filename, minimumCliqueSize);
}

Community_Finder::~Community_Finder() {
	for (vector< Seed * >::iterator seedItr = this->seeds.begin(); seedItr != this->seeds.end(); ++seedItr)
	{
		delete (*seedItr);
	}
}


//INTERFACE WITH CLIQUE FINDING CODE
int numberOfCliquesProcessed = 0;
void Community_Finder::operator () (const vector<V> & clique) { // V is simply typedef'd to an int. I suppose it might become a long if we deal with graphs with over 2 billion nodes/edges.
	// WARNING: Do NOT take a pointer to the clique. It will be invalidated by the clique finding code upon returning. You must copy the data into your own structures.
	vector <V > temp;

	for ( vector<V>::const_iterator cliqueVertexIterator = (clique).begin(); cliqueVertexIterator != (clique).end(); ++cliqueVertexIterator)
	{

		temp.push_back(*cliqueVertexIterator);		
	}

	this->cliques.push_back(temp);
	numberOfCliquesProcessed++;
	if (numberOfCliquesProcessed % REPORTING_OUTPUT_FREQUENCY == 0)
	{
		fprintf(stderr, "%.2fs: ",(double)(clock()-t0)/CLOCKS_PER_SEC);
		cerr << "Processed: " << numberOfCliquesProcessed << " cliques..." << endl;

	}
}

//Responsible for taking the initialised structures, and operating the algorithm on them.
void Community_Finder::run()
{

	// print graph
/*
	for (int i = 0; i < theGlobalGraph.vertex_count;i++)
	{
		pair <V*,V*> startAndEnd = theGlobalGraph.neighbours(i);
		cerr << "Neighbours of vertex " << i << " : ";
		for (V * otherVertex = startAndEnd.first; otherVertex != startAndEnd.second; otherVertex++)
		{
			cerr << " " << *otherVertex;
		}
		cerr << endl;

	}
*/
		fprintf(stderr, "%.2fs: ",(double)(clock()-t0)/CLOCKS_PER_SEC);

		fprintf(stderr, "%.2fs: ",(double)(clock()-t0)/CLOCKS_PER_SEC);
		cerr << "--------------------------------" << endl;

		cerr << "Number of seeds: " << this->seeds.size() << endl;
		
		sort(this->seeds.begin(),this->seeds.end(),sizeSortFunctionLargestFirst);
		
		vector< Seed* > resultsVec;
		int numberSeedsDiscardedBeforeExpansion = 0;
		int numberSeedsDiscardedAfterExpansion = 0;
		int numberSeedsKept = 0;
		int numberSeedsProcessed = 0;
		//for each seed

		bool issueAlphaWarning = false;
		
		for (vector< Seed * >::iterator seedItr = this->seeds.begin(); seedItr != this->seeds.end(); ++seedItr)
                {
			(*seedItr)->putIntoNodeToSeedsCache();
			
			if (numberSeedsProcessed % REPORTING_OUTPUT_FREQUENCY == 0)
			{
				fprintf(stderr, "%.2fs: ",(double)(clock()-t0)/CLOCKS_PER_SEC);
				cerr << "Processed: " << numberSeedsProcessed << " seeds\n";
		
				if(issueAlphaWarning)
				{
					cerr << "Warning: size of growing communities exceeds probable size: try increasing Alpha value." << endl; 
				}
				issueAlphaWarning = false;
			}
			numberSeedsProcessed++;
			
			bool alreadyCounted = false;
			alreadyCounted = (*seedItr)->overlapsAlreadyAcceptedSeed();
			
			if(!alreadyCounted)
			{

				(*seedItr)->updateCachedEdgeValuesFromScratch();
		                (*seedItr)->updateFrontierFromScratch();

				//expand to first peak fitness, within threshold
				while ( (*seedItr)->addBestNodeFromFrontierToSeed() > 0)
				{
				}

				if( (*seedItr)->getNumberOfNodes() > (theGlobalGraph.vertex_count / 4 ))
				{
					if(!issueAlphaWarning)
					{
						cerr << "Warning: size of growing communities exceeds probable size: try increasing Alpha value." << endl;
						issueAlphaWarning = true;
					}
				}

				
				alreadyCounted = (*seedItr)->overlapsAlreadyAcceptedSeed();
				if(!alreadyCounted)
				{
				//	cerr << "Was not a duplicate\n";
					//add it to results
					numberSeedsKept++;
					resultsVec.push_back((*seedItr));
				}
				else
				{
				//	cerr << "Was a duplicate\n";
					numberSeedsDiscardedAfterExpansion++;
				}

				(*seedItr)->clearCaches();				

			}
			else
			{
				numberSeedsDiscardedBeforeExpansion++;
			}			

			if(alreadyCounted)
			{
				(*seedItr)->dead = true;
				(*seedItr)->removeFromNodeToSeedsList();
			}
			
		}
		
		fprintf(stderr, "%.2fs: ",(double)(clock()-t0)/CLOCKS_PER_SEC);
		cerr << "Number seeds discarded before expansion: " << numberSeedsDiscardedBeforeExpansion << " seeds\n";
		cerr << "Number seeds discarded after expansion: " << numberSeedsDiscardedAfterExpansion << " seeds\n";
		cerr << "Number seeds kept: " << numberSeedsKept << " seeds\n";

		this->sweepTheDead();
		//TODO make this more efficient in future.
		this->seeds.assign(resultsVec.begin(), resultsVec.end());

}

//This is the CCH implementation, which gets the number of times each node is 'spoken for' by another clique.
//In our release, we always use '1' as the value for this number.
//It also checks what percentage of each candidate clique has then being 'spoken for' or 'covered'. If this percentage
//is above a threshold, then the clique is rejected as being insufficiently distinct.
//This function uses the following static parameters: Community_Finder::numberOfTimesRequiredToBeSpokenFor (normally set to 1)
// and Community_Finder::spokenForThresholdOfUniqueness, normally set to around .75
void Community_Finder::doSpokenForPruning()
{
	sort(this->seeds.begin(),this->seeds.end(),sizeSortFunctionLargestFirst);
	cerr << "Cliques sorted. About to run spokenFor pruning..." << "number of seeds before: " << this->seeds.size()<< endl;
	
	vector <int> spokenForTimes( theGlobalGraph.vertex_count );
	int numberOfCliquesProcessed = 0;
	
	for (vector< Seed * >::iterator seedItr = this->seeds.begin(); seedItr != this->seeds.end(); ++seedItr)
	{

		numberOfCliquesProcessed++;
		if (numberOfCliquesProcessed % REPORTING_OUTPUT_FREQUENCY == 0)
		{
			fprintf(stderr, "%.2fs: ",(double)(clock()-t0)/CLOCKS_PER_SEC);
			cerr << "Processed: " << numberOfCliquesProcessed << " cliques..." << endl;
		}


		int numberOfNodesAlreadySpokenFor = 0;
		for (set<V>::iterator nodeItr = (*seedItr)->getNodes().begin(); nodeItr != (*seedItr)->getNodes().end(); ++nodeItr)
		{
			int numberOfTimesRequiredToBeSpokenFor = Community_Finder::numberOfTimesRequiredToBeSpokenFor;
                        if (spokenForTimes[(*nodeItr)] > numberOfTimesRequiredToBeSpokenFor)
			{	
				numberOfNodesAlreadySpokenFor++;
			}
		}		
		
		//As the amount of nodes spoken for gets large, this value increases towards 1
		float proportionOfNodesAlreadySpokenFor = (float)numberOfNodesAlreadySpokenFor / (float)(*seedItr)->getNodes().size();	
	
		if (proportionOfNodesAlreadySpokenFor >= Community_Finder::spokenForThresholdOfUniqueness)
		{
			//if the proportion of nodes that have been taken is greater than the threshold, then discard this clique.
		
			//	cerr << "pruning, number of nodes spoken for: " << (float)numberOfNodesAlreadySpokenFor << " size of seed: " << (float)(*seedItr)->getNodes().size() <<  "value: " << (float)numberOfNodesAlreadySpokenFor / (float)(*seedItr)->getNodes().size()   << endl;		
			(*seedItr)->dead = true;
		}
		else
		{
			//this clique is good
			//so tag its nodes as taken.
			for (set<V>::iterator nodeItr = (*seedItr)->getNodes().begin(); nodeItr != (*seedItr)->getNodes().end(); ++nodeItr)
	                {
				spokenForTimes[(*nodeItr)]++;
			}
		}
	}
	
	this->sweepTheDead();
	cerr << "SpokenFor complete. Number of seeds after: "<< this->seeds.size() << endl;
}



//Responsible for using the clique finding code (which is an implementation of bron-kerbosch) to
//generate the maximal cliques in the graph.
//These cliques are then stored (in the callback) into the 'cliques' vector.
//This 'cliques' vector is then loaded into our 'Seed' objects, which are again stored in a vector.
//The Seed objects at this point are really just responsible for storing the initial, core, cliques.
//We then do the 'spoken for pruning' - or the CCH as it is also called, to prune out some of the highly
//degenerate cliques.
//After this, we consider the set of seeds to be initialised, and this function returns.
//Next, the 'run' function will be called to
//grow them into the candidate communities.
//TODO this implementation stores _every_ clique into memory before any are pruned using the CCH.
//this should later be optimised to run a clique at a time, only storing those not found to be discarded by the CCH, to save memory.
void Community_Finder::initialiseSeeds(const char * filename, int minimumCliqueSize)
{



	graph_loading::loadSimpleIntGraphFromFile(theGlobalGraph, filename);

	cliques::findCliquesJustIDs(theGlobalGraph, *this, minimumCliqueSize);
	
	cerr << "Loaded : " << this->cliques.size() << " cliques" << endl;
	//cliques now has all the cliques.  sort it by size.
        nodeToSeeds.clear();
        nodeToSeeds.resize(theGlobalGraph.vertex_count);


	numberOfCliquesProcessed = 0;
	for ( vector < vector <V> >::iterator cliqueItr = this->cliques.begin(); cliqueItr != this->cliques.end(); ++cliqueItr)
	{
		Seed * mySeed = new Seed();
		
		for (vector<V>::iterator innerCliqueItr = (*cliqueItr).begin(); innerCliqueItr != (*cliqueItr).end(); ++innerCliqueItr)
		{
			//mySeed->addNode((*innerCliqueItr));
			mySeed->addNodeNoCaching((*innerCliqueItr));
		}
		
		this->seeds.push_back(mySeed);

		numberOfCliquesProcessed++;
		if (numberOfCliquesProcessed % REPORTING_OUTPUT_FREQUENCY == 0)
		{
			fprintf(stderr, "%.2fs: ",(double)(clock()-t0)/CLOCKS_PER_SEC);
			cerr << "Processed: " << numberOfCliquesProcessed << " cliques..." << endl;
			cerr << "Current size of clique being processed: " << (*cliqueItr).size() << endl;
		}
	}

	//remove initial clique storage
	this->cliques.clear();
	
	//Run spoken for optimisation.
	this->doSpokenForPruning();

	fprintf(stderr, "%.2fs: ",(double)(clock()-t0)/CLOCKS_PER_SEC);
	cerr << "Total number of seeds remaining: " << this->seeds.size() << endl;
}


//Utility function which discards seeds that have been marked as dead.
//Remove all the seeds that have previously been marked as 'dead' from the vector storing the seeds
//This is necessary because we store the seeds in a vector.
void Community_Finder::sweepTheDead()
{
	//makes use of temporary vector
	//This temporary vector means we can write all the seeds to the new vector without having to delete and reallocate as we go.
	vector <Seed*> newVector;

	cerr << "Sweeping the seeds marked for deletion\n";
        for (vector< Seed * >::iterator seedItr = this->seeds.begin(); seedItr != this->seeds.end();++seedItr)
        {
                if ((*seedItr)->dead)
                {
                        delete *seedItr;
                }
                else
                {
			newVector.push_back(*seedItr);
                }
        }

	this->seeds.assign(newVector.begin(), newVector.end());

	cerr << "Done\n";
	
}


//Seeds have some cached values.
//They cache a map of nodes that make up their frontier.
//The values in this map are the number of internal and external edges that each of these nodes has - ie, the internal and external degree of the frontier node.
//We also cache the total internal and external degree of all the nodes inside the seed not on the frontier.
void Community_Finder::refreshAllSeedInternalCaches()
{
	for (vector< Seed * >::iterator seedItr = this->seeds.begin(); seedItr != this->seeds.end(); ++seedItr)
	{
		(*seedItr)->updateCachedEdgeValuesFromScratch();
		(*seedItr)->updateFrontierFromScratch();
	}
}

//TODO look up node names in map, here
void Community_Finder::rawPrint()
{
	for (vector< Seed * >::iterator seedItr = this->seeds.begin(); seedItr != this->seeds.end(); ++seedItr)
	{
		//(*seedItr)->rawPrint();
		(*seedItr)->rawPrintInOrderOfAddition();
		//(*seedItr)->prettyPrintFrontier();
		//(*seedItr)->prettyPrintFrontier();
	}


}

void Community_Finder::printSeeds()
{
	for (vector< Seed * >::iterator seedItr = this->seeds.begin(); seedItr != this->seeds.end(); ++seedItr)
	{
		(*seedItr)->prettyPrint();
	}
}
