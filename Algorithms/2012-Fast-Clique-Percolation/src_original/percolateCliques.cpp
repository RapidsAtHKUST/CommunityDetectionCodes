using namespace std;
#include "graph/network.hpp"
#include "graph/loading.hpp"
#include <getopt.h>
#include <unistd.h>
#include <libgen.h>
#include <ctime>
#include <vector>
#include <set>
#include <map>
#include <iostream>
#include <fstream>
#include <tr1/unordered_set>
#include <tr1/unordered_map>

#include "pp.hpp"
#include "cliques.hpp"
#include "cmdline.h"

int option_minCliqueSize = 3;

void test_cliques_to_vector(const char * edgeListFileName, const int k);
void percolateCliques(const char * edgeListFileName, const int k, const char * outFileName);

int main(int argc, char **argv) {


    if (argc != 4)
    {
        cout << "Incorrect number of parameters! Usage: inputFileName, k, outputFileName" << endl;

        exit(1);

    }

	const char * edgeListFileName   = argv[1];
	const int k = atoi(argv[2]);

    const char * outFileName = argv[3];

	percolateCliques(edgeListFileName, k, outFileName);
    

}



void percolateCliques(const char * edgeListFileName, const int k, const char * outFileName) {

    //First find the cliques
	vector< vector<int64_t> > all_cliques_by_orig_name;

    std :: auto_ptr<graph :: NetworkInterfaceConvertedToString > network;
	network = graph :: loading :: make_Network_from_edge_list_int64(edgeListFileName, 0, 0);
	cliques :: cliquesToVector(all_cliques_by_orig_name, network.get(), k);
	PP(all_cliques_by_orig_name.size());



    //assumes contiguously numbered input.
    int64_t maxId = 0;
    
    vector < set < int> > cliques;

    for(vector< vector<int64_t> > :: const_iterator one_clique = all_cliques_by_orig_name.begin(); one_clique != all_cliques_by_orig_name.end(); ++one_clique) {
		for( vector<int64_t> :: const_iterator one_node = one_clique->begin(); one_node != one_clique->end(); ++one_node) {
            if (*one_node >= maxId)
            {
                maxId = *one_node;
            }        
		}

        //Make a copy of the cliques; wasteful, but not a bottleneck, currently.
        cliques.push_back( set<int> ( one_clique->begin(), one_clique->end()));
	}
    cout << "Max Id: " << maxId << endl;



    //This is effectively a map from nodes to cliques the node is in.
    vector < set < int> * > nodesToCliques;
    nodesToCliques.resize(maxId+1, NULL);

    for(int i = 0; i < cliques.size(); ++i) 
    {
		for( set<int> :: const_iterator current_node = cliques[i].begin(); current_node != cliques[i].end(); ++current_node) 
        {
            
            if (nodesToCliques[*current_node] == NULL)
            {
                //XXX this would leak memory, if the application didnt terminate after one run.
                nodesToCliques[*current_node] = new set<int>;
            }
            nodesToCliques[*current_node]->insert(i);
		}
	}
        

    cout << "Setup complete, about to percolate found cliques." << endl;
    map <int, int> cliquesToComponents;
    
    int currentComponent = 0;
    
    for (int i = 0; i < cliques.size(); ++i)
    {
        if (cliquesToComponents.find(i) == cliquesToComponents.end())
        {
            ++currentComponent;
            cliquesToComponents[i] = currentComponent;


            set <int> frontier;
            frontier.insert(i);
            
            while(! frontier.empty())
            {
                int currentClique = *(frontier.begin());
                frontier.erase(frontier.begin());
                
                //for each clique 'next to' the current clique...
                map <int, int> otherCliqueToOverlap;
                

                //for each node in the current clique
		        for( set<int> :: const_iterator current_node = cliques[currentClique].begin(); current_node != cliques[currentClique].end(); ++current_node) 
                {
                    //for this specific node, in the current clique, for each of the other cliques that the node is in.
                    for (set<int> :: const_iterator other_clique =  (nodesToCliques[*current_node])->begin(); other_clique != (nodesToCliques[*current_node])->end(); ++other_clique)
                    {
                        //increment the number of times the other clique overlaps the current clique
                        otherCliqueToOverlap[*other_clique] += 1;

                    }
                }

                //for each other clique that overlaps with the current clique
                for (map<int,int>::iterator mapItr = otherCliqueToOverlap.begin(); mapItr != otherCliqueToOverlap.end(); ++mapItr)
                {
                    //if the overlap size is greater than k
                    if ( (*mapItr).second >= k - 1)
                    {
                        //percolates
                        cliquesToComponents[(*mapItr).first] = currentComponent;
                        //add it to the frontier
                        frontier.insert((*mapItr).first);
                        
                        //remove from nodesToCliques map
                        for (set <int> :: const_iterator otherCliqueNodes = cliques[(*mapItr).first].begin(); otherCliqueNodes != cliques[(*mapItr).first].end(); ++otherCliqueNodes)
                        {
                            (nodesToCliques[*otherCliqueNodes])->erase((*mapItr).first);                
                        }
                    }
                }
            }

        }


    }
    


    cout << "Percolation complete" << endl;

    //Making the list of nodes in each component, from the directory of which cliques are in the same component.
    map <int, set < int > > componentsToNodes;
    for (map <int, int>::iterator cliquesToComponentsItr = cliquesToComponents.begin(); cliquesToComponentsItr != cliquesToComponents.end(); ++cliquesToComponentsItr)
    {
        int theClique = (*cliquesToComponentsItr).first;
        int theComponent = (*cliquesToComponentsItr).second;
        
        
        for ( set <int>::iterator cliqueItr = cliques[theClique].begin(); cliqueItr != cliques[theClique].end(); ++cliqueItr)
        {
            componentsToNodes[theComponent].insert( *cliqueItr);
        }
    }



    //Output
    ofstream myfile;
    myfile.open(outFileName);
    

    for (map <int , set <int> > ::iterator componentItr = componentsToNodes.begin(); componentItr != componentsToNodes.end(); ++componentItr)
    {
        for ( set <int>::iterator nodeItr = (*componentItr).second.begin(); nodeItr != (*componentItr).second.end(); ++nodeItr)
        {

            myfile << (*nodeItr) << " ";
        }
        myfile << endl;
    }

    myfile.close();
}

