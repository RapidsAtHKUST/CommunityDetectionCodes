/*
 * Seed.cpp
 *
 *  Created on: 01-Dec-2009
 *      Author: freid
 */

#include "Seed.h"

//////////////////////////////
//CONSTRUCTION AND DESTRUCTION
//////////////////////////////
Seed::Seed()
{
	this->internalEdges = 0;
	this->dead = false;
	this->externalEdges = 0;
	//this->cachesDirty = false;
}

Seed::~Seed() {
}



void Seed::removeFromNodeToSeedsList()
{
        for(set<V>::iterator nodeItr= this->nodes.begin(); nodeItr != this->nodes.end(); ++nodeItr)
        {
                //for each of the nodes
                ( nodeToSeeds[ (*nodeItr) ] ).erase(this);
        }
}

float Seed::minimumOverlapToMerge;
float Seed::alphaValueForFitness;

/////////
//TEST FOR OVERLAP
/////////

bool Seed::isEqualTo(Seed & other)
{
	set<V> result;
	set_difference( (this)->nodes.begin(), (this)->nodes.end(), (other).nodes.begin(), (other).nodes.end(), inserter(result,result.begin() ));
        return (result.size() == 0);
}


//returns true if this seed overlaps ANY seed that has already been accepted
//makes use of the node->seeds map, for speed.
bool Seed::overlapsAlreadyAcceptedSeed()
{
	//get the union of the set of cliques that each node is in


	set <Seed*> result;		
	set <Seed*> oldResult;
	//for each of the nodes
	
	for(set<V>::iterator nodeItr= this->nodes.begin(); nodeItr != this->nodes.end(); ++nodeItr)
        {
		
		result.clear();
		//this could be faster if it returned a pointer to a set of seed*, rather than a set of seed*
		set_union( nodeToSeeds[ (*nodeItr) ].begin(),nodeToSeeds[(*nodeItr)].end(),oldResult.begin(), oldResult.end(), inserter(result,result.begin()) );
		oldResult = result;
	}
	
	
	//result now contains all the seeds this seed can overlap with.

	//cerr << "Size of the set of seeds this seed may overlap with: " << result.size() << endl;
	for (set<Seed*>::iterator seedItr = result.begin(); seedItr != result.end(); ++seedItr)
	{
		if( ((*seedItr) != this) && !(*seedItr)->dead)
		{
			if(this->overlap(**seedItr) >= Seed::minimumOverlapToMerge)
			{
				//cerr << "Found an overlap between: " <<endl;
				//this->prettyPrint();
				//cerr << " and other: "<<endl;
				//(**seedItr).prettyPrint();
				return true;
			}
		}
	}	
	return false;
}


//long numberOfOverlapChecks = 0;

//Return a parameter for how much the two seeds overlap.
float Seed::overlap(Seed & other)
{
	//TEST CODE TO INVESITGATE THE REDUCTION IN COMPARISIONS WHEN USING THE NODE->SEEDS OVERLAP:
	//numberOfOverlapChecks++;
	//if (numberOfOverlapChecks % 10000 == 0)
	//{
	//	cerr << "Number of overlap checks done so far: " << numberOfOverlapChecks << endl;
	//}
	set<V> result;
	set_intersection(this->nodes.begin(), this->nodes.end(), other.nodes.begin(),other.nodes.end(), inserter(result,result.begin()));
	return float(result.size()) / min( other.getNodes().size(),this->getNodes().size()  );
}


///////////////////
//COMMUNITY FINDING
///////////////////

//Need a function that adds the best node from the frontier to the seed

//This function adds the best node from the frontier to the seed, as long as the resulting fitness is above a certain threshold.
//It returns the resulting fitness of the seed, or -1 if no node could be added.
//Requires the new node to increase the fitness.
float Seed::addBestNodeFromFrontierToSeed()
{
	//search across the possible fitnesses of the frontier.
	//cerr << "In addBestNodeFromFrontierToSeed for seed: ";
	//this->prettyPrint();
	float highestFitness = this->calculateFitness();
	//float highestFitness = -1; 
	V bestNode = -1;

	for (map <V,pair<int,int> >::iterator frontierNodeToInternalAndExternalEdgesItr = this->frontierNodeToInternalAndExternalEdges.begin(); frontierNodeToInternalAndExternalEdgesItr != this->frontierNodeToInternalAndExternalEdges.end(); ++frontierNodeToInternalAndExternalEdgesItr)
	//for( set < V >::iterator frontierItr= this->frontier.begin(); frontierItr != this->frontier.end(); ++frontierItr)
	{
	//	cerr << "Considering next frontier node...\n";
	//	this->prettyPrintFrontier();
	//	cerr << (*frontierItr);
		//if ((*frontierItr).second > highestFitness)
		
		
		//This is the main optimisation, right here, where, instead of iterating across (the current Seed Union theNewNode) and calculating each nodes internal and external degree by traversing the nodes edges, and seeing if they are inside or outside the seed - instead we use the cached values of the number of edges that are inside and outside the seed, for each node on the frontier.  This cache can later by updated, as it only changes for those frontier nodes which are connected to the node added to the seed.
		float tempFitness =  this->calculateFitnessIfInsertFromFrontier((*frontierNodeToInternalAndExternalEdgesItr).second.first,(*frontierNodeToInternalAndExternalEdgesItr).second.second );
	//	cerr << " with cached fitness: " << tempFitness;
		if(tempFitness > highestFitness)
		{
	//		cerr << "...higher than existing highest fitness" << highestFitness << " \n";
			bestNode = ((*frontierNodeToInternalAndExternalEdgesItr).first);
			highestFitness = tempFitness;
		}
		else
		{
	//		cerr << "...NOT higher than existing highest fitness" << highestFitness << " \n";

		}
		
	}
	if (bestNode > -1)
	{
	//	cerr << "So, adding node: " << bestNode << " to seed\n";
		//Found a node with a better fitness than the threshold.
		this->addNodeFromFrontier(bestNode);
	
	//DEBUG
	//	this->rawPrint();
	//	this->prettyPrintFrontier();
	//	cerr << "Fitness now, recaclulated: " << this->calculateFitness();
	
		return highestFitness;
	}
	else
	{
	//	cerr << "No best node found\n";
		return -1;
	}
}


//Can add a node from anywhere.
//This dirties the caches.
void Seed::addNode(V newNode)
{
	this->nodesInOrderOfAddition.push_back(newNode);
	this->nodes.insert(newNode);
	(nodeToSeeds[newNode]).insert(this);
}



void Seed::addNodeNoCaching(V newNode)
{
	this->nodesInOrderOfAddition.push_back(newNode);
	this->nodes.insert(newNode);
}

//put all of this seeds nodes into the NodeToSeeds Cache
void Seed::putIntoNodeToSeedsCache()
{
	for(set<V>::iterator innerSeedItr= this->nodes.begin(); innerSeedItr != this->nodes.end(); ++innerSeedItr)
        {
		(nodeToSeeds[(*innerSeedItr)]).insert(this);
	}
}



//Calculate the number of edges that go into the seed and go out of the seed that this node has
pair <int,int> Seed::calculateNumberOfInternalAndExternalEdgesForNodeFromScratch(V theNode)
{
	pair <V*,V*> otherStartAndEnd = theGlobalGraph.neighbours(theNode);

	int thatNodesInternalNodes = 0;
	int thatNodesExternalNodes = 0;

	for (V * theOtherVertex = otherStartAndEnd.first; theOtherVertex != otherStartAndEnd.second; theOtherVertex++)
	{

		V theOtherNode = *theOtherVertex;
		if ( this->contains(theOtherNode))
		{
			thatNodesInternalNodes++;
		}
		else
		{
			//the other node isn't in the seed, so is now in frontier
			thatNodesExternalNodes++;
		}
	}
	pair <int, int> otherTempPair;
	otherTempPair.first = thatNodesInternalNodes;
	otherTempPair.second = thatNodesExternalNodes;
	return otherTempPair;
}


//THE NODE IN QUESTION MUST BE ON THE FRONTIER
//This adds a node, without invalidating the caches.
void Seed::addNodeFromFrontier(V newNode)
{
	//This if statement existed as an assertion to catch coding errors; disable for performance now that the implementation is stable.
	//if (this->frontierContains(newNode))
	//{
		pair <int, int> newValuesIfInsert = this->frontierNodeToInternalAndExternalEdges[newNode];
		this->frontierNodeToInternalAndExternalEdges.erase(newNode);
		//this->frontier.erase(newNode);	
		
	//	cerr << "\n Cached node internal edges: " << newValuesIfInsert.first;
	//	cerr << " cached node external edges " << newValuesIfInsert.second << "  ";
	
 		//update the seed caches of internal and external edges
		int totalInternalEdges = (newValuesIfInsert.first * 2) + this->getInternalEdges();
       	 	int totalExternalEdges = this->getExternalEdges() + newValuesIfInsert.second - (newValuesIfInsert.first);
		
		this->internalEdges = totalInternalEdges;
		this->externalEdges = totalExternalEdges;
	//	cerr << " -- new seed internal edges: " << this->internalEdges << " new seed external edges: " << this->externalEdges << "\n";		


		//bool temp = this->cachesDirty;
		this->addNode(newNode);
		//this->cachesDirty = temp;

		//Now just need to update the frontier.
		//For each of the new seedNodes neighbours, if the neighbour is in the seed, it doesn't need to be changed.
		//but if it isn't in the seed, and its in the frontier, it needs its fitness updated.
		//also, if it isn't in the seed or the frontier, it must be added to the frontier, and have its fitness calculated.

		//SURGICALLY UPDATE THE FRONTIER
		//weve decided to add newNode. The nodes that were connected to newNode need their values changed.
		pair <V*,V*> startAndEnd = theGlobalGraph.neighbours(newNode);
	//	cerr << "Neighbours of new node : ";
		for (V * otherVertex = startAndEnd.first; otherVertex != startAndEnd.second; otherVertex++)
		{
	//		cerr << " " << *otherVertex;
			V otherNode = *otherVertex;
			if(this->contains(otherNode))
			{
	//			cerr << " was already in seed. ";
								

				//float theFitness = this->calculateFitnessIfInsertFromFrontier(otherNode);
				//cerr << " so new fitness, on frontier, for " << otherNode << " would be: " << theFitness;
				//this->frontierNodeToFitness[otherNode] = theFitness;
			}
			else
			{
				//This node must now be in the frontier
				//recalculate the internal and external edges for that node
				//if its currently in the frontier, take its cached values, and simply subtract one external edge and add one internal
				//if its not in the frontier, then all its values must be calculated.
	//			cerr << " was NOT in seed. ";
				if(this->frontierContains(otherNode))
				{
	//				cerr << " but was in frontier ";
					pair <int, int> oldValuesCached = this->frontierNodeToInternalAndExternalEdges[otherNode];
	//				cerr << " and had internal: " << oldValuesCached.first << " and external: " << oldValuesCached.second << " "; 
					oldValuesCached.first = oldValuesCached.first +1;
					oldValuesCached.second = oldValuesCached.second -1;
			
					//this->frontier.insert(otherNode);
					this->frontierNodeToInternalAndExternalEdges[otherNode] = oldValuesCached;
	//				cerr << " updated from caches with internal: " << oldValuesCached.first << " and external: " << oldValuesCached.second << " "; 
					
				}
				else
				{
	//				cerr << " and was NOT in frontier";
					//If the node that we are looking at is not in the frontier, then we must add it to the frontier.
					//But we must also add its current internal and external degree to the frontierNode->internalAndExternalEdges cache.
					//We do that here; its not a cheap operation but it is necessary even in the non-caching solution, and its much cheaper overall than not using a cache.
					pair <int,int> otherTempPair = this->calculateNumberOfInternalAndExternalEdgesForNodeFromScratch(otherNode);
					//this->frontier.insert(otherNode);
					this->frontierNodeToInternalAndExternalEdges[otherNode] = otherTempPair;
	//				cerr << " updated from scratch with internal: " << otherTempPair.first << " and external: " << otherTempPair.second << " "; 
				}
				

			}
		}
//	}
//	else
//	{
		//error
	//	cerr << "error, node not in frontier" << endl;
//		exit(1);
//	}

}


//This function updates the frontier, requiring no other information.
//It updates the frontierNodeToInternalAndExternalEdges array
//Takes the current seed, and completely updates its frontier.
void Seed::updateFrontierFromScratch()
{
	//cerr << "Updating frontier from scratch for seed: " << endl;
	//this->prettyPrint();

	this->frontierNodeToInternalAndExternalEdges.clear();
	//this->frontier.clear();

	for(set<V>::iterator innerSeedItr= this->nodes.begin(); innerSeedItr != this->nodes.end(); ++innerSeedItr)
	{
		V currentNode = (*innerSeedItr);
		pair <V*,V*> startAndEnd = theGlobalGraph.neighbours(currentNode);

		int thisNodesInternalNodes = 0;
		int thisNodesExternalNodes = 0;
		

		for (V * otherVertex = startAndEnd.first; otherVertex != startAndEnd.second; otherVertex++)
		{
			
			V otherNode = *otherVertex;
			if ( this->contains(otherNode))
			{
				thisNodesInternalNodes++;
			}
			else
			{
				pair <int,int> otherTempPair = this->calculateNumberOfInternalAndExternalEdgesForNodeFromScratch(otherNode);
				//this->frontier.insert(otherNode);
				this->frontierNodeToInternalAndExternalEdges[otherNode] = otherTempPair;
	//			cerr << "Got internal edges: " << otherTempPair.first;
	//			cerr << " and external edges: " << otherTempPair.second;
	//			cerr << " for frontier node: " << otherNode << endl;

				//the other node isn't in the seed, so is now in frontier
				thisNodesExternalNodes++;
			}
		}
		
		
	}
	//cerr << "Frontier now is: " << endl;
	//this->prettyPrintFrontier();
}

//Update the number of internal and external edges currently in the seed
void Seed::updateCachedEdgeValuesFromScratch()
{
	int countInternals = 0;
	int countExternals = 0;

	for(set<V>::iterator innerSeedItr= this->nodes.begin(); innerSeedItr != this->nodes.end(); ++innerSeedItr)
	{
		V currentNode = (*innerSeedItr);
		//For each of the edge points of current node, is it inside or outside the seed?

		//ASSUMES NO SELF LOOPS
		pair <V*,V*> startAndEnd = theGlobalGraph.neighbours(currentNode);
		for (V * otherVertex = startAndEnd.first; otherVertex != startAndEnd.second; otherVertex++)
		{
			//iterate across the neighbours
			V otherNode = *otherVertex;
			if (this->contains(otherNode))
			{
				countInternals++;
			}
			else
			{
				countExternals++;
			}
		}
	}

	//Note, each of the internal edges have been counted twice, whereas the external edges have only been counted once.
	//We consider graphs as undirected, and consider two seperate edges as a single bidirectional one.
	countInternals = countInternals;

	this->externalEdges = countExternals;
	this->internalEdges = countInternals;
	//this->cachesDirty = false;
}

void Seed::clearCaches()
{
	//this->frontier.clear();
	this->frontierNodeToInternalAndExternalEdges.clear();

}

//////////////////
//Fitness function
//////////////////

//THE CACHES MUST BE UP TO DATE
inline float Seed::calculateFitness()
{
	//Lancichinetti et al
	return (this->getInternalEdges()) / pow( ((this->getInternalEdges())+this->getExternalEdges()), Seed::alphaValueForFitness);
}

//float Seed::calculateFitnessIfInsertFromFrontier(V newNode)
//This function is a fast way of calculating what the fitness would be if a node with the given properties was to be inserted into the seed.
//This makes use of the cached values of the internal and external edges of the existing seed, as well as the cached values of the internal and external degree of the node in question (which are retrived from the map before this function is called) to calcuate the resulting fitness, if this node would be inserted. This is much faster than trying to recalculate the fitness from scratch for the new Seed, by iterating across all the edges of all the nodes in the seed.
inline float Seed::calculateFitnessIfInsertFromFrontier(int internalEdgesOfNode, int externalEdgesOfNode)
{
	//assuming that newNode is on the frontier, and not in the seed, calculate how its fitness would change
	//pair <int,int> internalAndExternal = this->calculateEdgesIfInsertFromFrontier(newNode);
	
//	pair <int, int> internalAndExternal = this->frontierNodeToInternalAndExternalEdges[newNode];

//	int internalEdgesOfNode = internalAndExternal.first;
//	int externalEdgesOfNode = internalAndExternal.second;

	int totalInternalEdges = (internalEdgesOfNode * 2) + this->getInternalEdges();
	int totalExternalEdges = this->getExternalEdges() + externalEdgesOfNode - (internalEdgesOfNode);

	return (totalInternalEdges) / pow( (totalInternalEdges + totalExternalEdges),Seed::alphaValueForFitness);
	//return (internalEdges) / pow( ( (internalEdges) + externalEdges ), Seed::alphaValueForFitness);
}



///////////////////
//Utility functions
///////////////////



//Not thread safe - must be executed as a block.
inline int Seed::getInternalEdges()
{
	//if (!this->cachesDirty)
	//{
		return internalEdges;
	//}
	//else
	//{//TODO eventually this will be an error condition, for speed.
	//	cerr << "ERROR: CACHES DIRTY!";
	//	exit(1);
	//	this->updateCachedEdgeValuesFromScratch();
	//	return internalEdges;	
	//}
}
//Not thread safe - must be executed as a block.
inline int Seed::getExternalEdges()
{
	//if (!this->cachesDirty)
	//{
		return externalEdges;
	//}
	//else
	//{//TODO eventually this will be an error condition, for speed.
	//	cerr << "ERROR: CACHES DIRTY!";
	//	exit(1);
	//	this->updateCachedEdgeValuesFromScratch();
	//	return externalEdges;
	//}
}


const set<V> & Seed::getNodes()
{
	return nodes;
}

int Seed::getNumberOfNodes()
{
	return this->nodes.size();

}

inline bool Seed::contains(V theNode)
{
	//return true if the node is contained in this seed
	if (this->nodes.find(theNode) != this->nodes.end())
	{
		return true;
	}
	else
	{
		return false;
	}

}

inline bool Seed::frontierContains(V theNode)
{
	//return true if the node is contained in the frontier
	if (this->frontierNodeToInternalAndExternalEdges.find(theNode) != this->frontierNodeToInternalAndExternalEdges.end())
	{
		//its on the frontier
		return true;
	}
	else
	{
		return false;
	}
}


void Seed::prettyPrintFrontier()
{
//	cerr << "\tFrontier:";
//	cerr << " current fitness: " << this->calculateFitness() << "\n";
//	for( set < V >::iterator innerSeedItr= this->frontierNodeToInternalAndExternalEdges.begin(); innerSeedItr != this->frontier.end(); ++innerSeedItr)
//	{
//		cerr << "\tFrontierNode: " << (*innerSeedItr) << " frontierNodeInternalDegree: " << this->frontierNodeToInternalAndExternalEdges[(*innerSeedItr)].first  << " externalDegree: " << this->frontierNodeToInternalAndExternalEdges[(*innerSeedItr)].second << " fitness if add: " << this->calculateFitnessIfInsertFromFrontier((*innerSeedItr))  ;
//	cerr << endl;
//	}
}

void Seed::rawPrintInOrderOfAddition()
{
	for(vector<V>::iterator innerSeedItr= this->nodesInOrderOfAddition.begin(); innerSeedItr != this->nodesInOrderOfAddition.end(); ++innerSeedItr)
	{
		cout << theGlobalGraph.name_of_one_node_asString((*innerSeedItr)) << " ";
	}
	cout << endl;


}

void Seed::rawPrint()
{
	for(set<V>::iterator innerSeedItr= this->nodes.begin(); innerSeedItr != this->nodes.end(); ++innerSeedItr)
	{
	 	cout << theGlobalGraph.name_of_one_node_asString((*innerSeedItr)) << " ";
	}
	cout << endl;
}

void Seed::prettyPrint()
{

	cerr << "Seed: Members:";

	for(set<V>::iterator innerSeedItr= this->nodes.begin(); innerSeedItr != this->nodes.end(); ++innerSeedItr)
	{
		cerr << " " << (*innerSeedItr);
	}
	cerr << " Internal Edges: " << this->getInternalEdges() << " External Edges:" << this->getExternalEdges() << " Fitness: " << this->calculateFitness();
	cerr << endl;
}


