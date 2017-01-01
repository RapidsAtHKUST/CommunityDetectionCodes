/*
scp, The sequential clique percolation algorithm.
Copyright (C) 2011  Aalto University

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

/*
A C++ implementation of the sequential clique percolation (SCP) algorithm. This version is seriously cripled and only unweighted clique percolation without 
thresholding is allowed. However, all the code to include further functionality is already included in the source code.

If you use this code to produce results for a publication, please cite:
A sequential algorithm for fast clique percolation, J.M. Kumpula, M. Kivelä, K. Kaski,and J. Saramäki, Phys. Rev. E 79, 026109 (2008)

The article contains detailed introduction to the SCP.

Authors: Eetu Latja, Mikko Kivelä and Jussi Kumpula.
*/


#include "k_clique.h"
#include "dendrogram.h"

#define HELPSTR "Usage: ./k_clique [inputfile] [options]\n\
The input file must be in the edge list format, where at each \
line there three columns: node1Index, node2Index and edge weight separated \
by a white space. Node indices must be integers from 0 to n-1, where n is the \
number of nodes in the network. Edges are undirected and there can only be \
at most one edge between two nodes in the edge file.\n\
Options:\n\
\t-o=[outputfile] : Write output to a specified file.\n\
\t-k=[clique size] : The size of the clique.\n\
\t-v : Verbose mode.\n "

//\t-w : Use weighted clique percolation.\n\
//\t-f=[weightfunction] : Specifies a weight function when using weighted clique percolation.\n "

struct Link{
  size_t source;
  size_t dest;
  float weight;
  Link(size_t source=0,size_t dest=0, float weight=0.0) : source(source), dest(dest), weight(weight) {}

};


// finds out the optimal size for the hash
size_t determineHashSize(const size_t numElements, const size_t k)
{
    size_t size_limit = 26;
    size_t t;
    if ( k < size_limit)
        t = k;
    else
        t = size_limit;
    while ( t < 10 ) t += k;
    while ( 1 << t < numElements && t <  size_limit )
    {
        t += k;
    }
    return t; // this many bits are needed
}

// determine the network size and number of links
// same link must not be twice in the network!
bool getNetSizeAndLinkNumbers(char * fileName, size_t & netSize, size_t & numLinks, std::list<Link> & linkList)
{

    std::ifstream myfile(fileName);
    std::string line;
    size_t source, dest;
    float weight;
    netSize=0;
    numLinks=0;

    if (myfile.is_open())
    {
        while (!myfile.eof())
        {
            getline(myfile, line);   // Read a line from input into a string.
            if (!line.empty())
            {
                std::istringstream is(line);  // Extract data using a stringstream.
		if ((is >> source) && (is >> dest) && (is >> weight));
		else {
		  std::cerr<<"Error reading line "<<numLinks << std::endl;
		  return false;}		

		linkList.push_back(Link(source,dest,weight));

                if (source > netSize)
                    netSize = source; //the size of the net is determined by the largest node index
                if (dest > netSize)
                    netSize = dest;
                ++numLinks;
            }
        }
        myfile.close();
    }
    ++netSize; // net size is one more than the largest index
    //std::cout << "ok\n";
    return true;
}

/*
  reads one line from the input file. The line contains the source and target nodes and weight.
*/
bool readLine(std::ifstream & myfile, size_t & source,  size_t & dest, float & weight)
{
    bool readlineok = false;
    if ( myfile.is_open())
    {

        std::string line;
        getline(myfile, line);   // Read a line from input into a string.
        if (!line.empty())
        {
            std::istringstream is(line);  // Extract data using a stringstream.
            is >> source;
            is >> dest;
            is >> weight;
            readlineok = true;
        }
    }
    else
    {
        std::cerr << "Error opening the network file!\n";
    }
    return readlineok;
}

void kCliquesFind(std::vector<weighedClique> & cliqueVector, NetType & net, size_t source, size_t dest, const float weight, const size_t k, const size_t weightFunction)
{

  // If k=2 cliques are links and the only new clique formed is the added link itself.
  if (k==2){
    std::vector<size_t> tempVector(2);
    tempVector[0] = source; tempVector[1] = dest;
    weighedClique tempClique(tempVector, net, weightFunction);
    cliqueVector.push_back(tempClique);
    net[source][dest] = weight; // add link
    return;
  }
  
  // find out degrees of source and dest
  size_t k_s, k_d;
  k_s = net(source).size(); 
  k_d = net(dest).size();
  
  // If degree of 'source' is larger than degree of 'dest', swap these. This makes it faster because we only iterate through nodes which are neighbors of 'source'.
  if (k_s > k_d)
    {
      size_t temp = source;
      source = dest;
      dest = temp;
    }
  

  if ( k_s  > k - 3 && k_d > k - 3)   // if this is not true, a new k-clique can not form
    {
      if (k == 3)
        {
	  std::vector<size_t> tempVector(3);
            tempVector[0] = source;
            tempVector[1] = dest;
            for ( NetType::const_edge_iterator i=net(source).begin(); !i.finished(); ++i)
	      {
                if ( net(*i)[dest] > 0 )
		  {
                    tempVector[2] = *i;
                    weighedClique tempClique(tempVector, net, weightFunction);
                    cliqueVector.push_back(tempClique);		    
		  }
	      }	    
        }
      else if (k == 4)
        {
	  std::vector<size_t> tempVector(4);
	  tempVector[0] = source;
	  tempVector[1] = dest;
	  for ( NetType::const_edge_iterator i=net(source).begin(); !i.finished(); ++i)
            {
	      NetType::const_edge_iterator j=i;
	      for ( ++j; !j.finished(); ++j) // makes j to point to the next iterable after i
                {
		  if ( net(*i)[dest] > 0 && net(*j)[dest] > 0 && net(*i)[*j] > 0)
                    {
		      tempVector[2] = *i;
		      tempVector[3] = *j;
		      weighedClique tempClique(tempVector, net, weightFunction);
		      cliqueVector.push_back(tempClique);
                    }
                }
            }	  
        }
      else // k>4
        {
	  nodeSet commonNeighborhood;
	  for ( NetType::const_edge_iterator i=net(source).begin(); !i.finished(); ++i)
            {
	      if ( net(*i)[dest] > 0 )
		commonNeighborhood.put(*i);
            }
	  
	  NetType subNet(commonNeighborhood.size());
	  
	  std::vector<size_t> sourceAndDest(2);
	  sourceAndDest[0] = source;
	  sourceAndDest[1] = dest;
	  
	  std::vector<weighedClique> k2cliqueVector;
	  for (nodeSet::iterator i = commonNeighborhood.begin(); !i.finished(); ++i)
            {
	      nodeSet::iterator j = i;
	      for ( ++j; !j.finished(); ++j) // makes j to point to the next iterable after i
                {
		  if (net(*i)[*j] > 0)
                    {
		      kCliquesFind(k2cliqueVector, subNet, *i, *j, net(*i)[*j], k - 2, 0);
		      for (std::vector<weighedClique>::iterator vectorIter = k2cliqueVector.begin(); vectorIter != k2cliqueVector.end(); vectorIter++)
                        {
			  weighedClique current = *vectorIter;
			  current.addNodes(sourceAndDest, net);
			  cliqueVector.push_back(current);
                        }
                    }
                }
            }	  
        }
    }

  //Finally add the new link to the network
  net[source][dest] = weight; 
}

void kCommunitiesFind(std::vector<weighedClique> & cliqueVector, KruskalTree & communities, cliqueHash & k1cliquesHash, NetType & net, const size_t k)
{
    std::vector<size_t> tempVector;
    weighedClique k1clique;
    for (std::vector<weighedClique>::iterator i = cliqueVector.begin(); i != cliqueVector.end(); ++i)
    {
        size_t rootIndex;
        for (size_t j = 0; j < k; j++)
        {
            tempVector.clear();
            for (size_t l = 0; l < k; l++)
                if (l != j)
                    tempVector.push_back(i->at(l));
            k1clique.replaceNodes(tempVector, net);

            // check if this (k-1)-clique is founded before
            int index = k1cliquesHash.getValue(k1clique);
            if (!j) // first (k-1)-clique from this k-clique
            {
                if (index >= 0) // found before
                {
                    rootIndex = index;
                }
                else
                {
                    rootIndex = communities.add();
                    k1cliquesHash.put(k1clique, rootIndex);
                }
            }
            else
            {
                if (index >= 0) // found before
                {
                    communities.connect(rootIndex, index);
                }
                else
                {
                    size_t newIndex = communities.add();
                    k1cliquesHash.put(k1clique, newIndex);
                    communities.connect(newIndex, rootIndex);
                }
            }
        }
    }
}

void outputCommunityStructure(KruskalTree & communities, cliqueHash & k1cliquesHash, std::ofstream & file)
{
    // Go through each community and put nodes from each community into a nodeSet
    std::map<size_t, nodeSet> realCommunities;
    for (std::pair<clique,size_t> currentPair = k1cliquesHash.begin(); !k1cliquesHash.finished(); currentPair = k1cliquesHash.next())
    {
        size_t community = communities.findRoot(currentPair.second);
        for (size_t i = 0; i < currentPair.first.size(); i++)
        {
            realCommunities[community].put(currentPair.first.at(i));
        }
    }

    if (file.is_open())
    {
        int communityIndex = 1;
        for (std::map<size_t, nodeSet>::iterator i = realCommunities.begin(); i != realCommunities.end(); i++)
        {
	  //file << communityIndex << ": "; // Uncomment if you want to enumerate the communities
            for (nodeSet::iterator j = i->second.begin(); !j.finished(); ++j)
                file << *j << " ";
            file << "\n";
            communityIndex++;
        }
    }
    file << "\n";
}


void unweightedSCP(NetType & net, std::list<Link> & linkList, const size_t numberOfLinks, const size_t k, std::string outputFile,bool verbose)
{
    size_t source, dest;
    float weight;
    std::vector<weighedClique> cliqueVector;
    
    //Determine the number of k-1 cliques
    size_t numberOfSmallCliques=0;
    NetType *tempNetPointer= new NetType(net.size());
    NetType &tempNet=*tempNetPointer;
    for (std::list<Link>::iterator linkIterator=linkList.begin() ; linkIterator != linkList.end(); linkIterator++ )
    {
      source=(*linkIterator).source; dest=(*linkIterator).dest;weight=(*linkIterator).weight;
      cliqueVector.clear();
      kCliquesFind(cliqueVector, tempNet, source, dest, weight, k-1, 0);
      numberOfSmallCliques+=cliqueVector.size();
    }
    delete tempNetPointer;

    //Use the number of k-1 cliques to determine the hash size
    size_t hash_bits = determineHashSize(numberOfSmallCliques, k - 1);
    size_t numSlots=1; numSlots = numSlots << hash_bits; // Number of slots is 2**hash_bits
    if (verbose){
      std::cout<< "Number of "<< k-1 <<"-cliques: " <<numberOfSmallCliques<<std::endl;
      std::cout<<"Number of bits in the hash table: "<<hash_bits<<std::endl;
      std::cout<<"Number of slots in the hash table: "<<numSlots<<std::endl;
    }
    cliqueHash k1cliquesHash(numSlots, hash_bits, k - 1);        

    KruskalTree communities;
    size_t nLinksLeft=linkList.size();
    while (nLinksLeft>0)
    {
      nLinksLeft--;
      Link link=linkList.front();linkList.pop_front();
      source=link.source;dest=link.dest;weight=link.weight;
    
      // phase I
      cliqueVector.clear();
      kCliquesFind(cliqueVector, net, source, dest, weight, k, 0);
      
      // phase II
      kCommunitiesFind(cliqueVector, communities, k1cliquesHash, net, k);
    }
    
    //communities are now detected, next we output the structure
    std::ofstream ofile(outputFile.c_str());
    outputCommunityStructure(communities, k1cliquesHash, ofile);
   
}

bool weighedCliqueCmp(const weighedClique lhs, const weighedClique rhs)
{
    return lhs.getWeight() > rhs.getWeight();
}

void kCommunitiesFindWeighted(std::vector<weighedClique> & cliqueVector, KruskalTree & communities, cliqueHash & k1cliquesHash, NetType & net, const size_t k, const float threshold)
{

    std::vector<size_t> tempVector;
    weighedClique k1clique;
    nodeCommunities nodeComs;
    Dendrogram dendrogram;
    size_t currentCliqueNumber = 0;
    CommunityTracker comTracker(k1cliquesHash, communities, nodeComs, dendrogram, net);
    for (std::vector<weighedClique>::iterator i = cliqueVector.begin(); i != cliqueVector.end(); ++i)
    {
        if (i->getWeight() < threshold)
            break;

        comTracker.addClique(*i);

        currentCliqueNumber++;

        //nodeComs.printCommunities(nodeCommunitOutputFile);
    }

    //dendrogram.printTree(dendrogramOutputFile);
}

void weightedSCP(NetType & net, std::ifstream & file, const size_t numberOfLinks, const size_t k, const float threshold, const size_t weightFunction, std::string outputFile)
{
    size_t source, dest;
    float weight;

    // store all k-cliques here and sort after that
    std::vector<weighedClique> cliqueVector;

    // phase I
    while (readLine(file, source, dest, weight))
        kCliquesFind(cliqueVector, net, source, dest, weight, k, weightFunction);

    std::sort(cliqueVector.begin(), cliqueVector.end(), weighedCliqueCmp);

    //std::cout << cliqueVector.size() << " k-cliques" << std::endl;

    // Use an upper bound for the number of k-1 cliques as hash size. Each k-clique has k k-1 cliques.
    size_t hash_bits = determineHashSize(cliqueVector.size() * k, k - 1);

    cliqueHash k1cliquesHash(1 << hash_bits, hash_bits, k - 1);
    KruskalTree communities;

    // phase II
    kCommunitiesFindWeighted(cliqueVector, communities, k1cliquesHash, net, k, threshold);

    //communities are now detected, next we output the structure
    std::ofstream ofile(outputFile.c_str());
    outputCommunityStructure(communities, k1cliquesHash, ofile);
    //std::cout << "largest: " << communities.getLargestComponentSize() << std::endl;
}

bool validateLinkList(std::list<Link> &linkList,size_t netSize,bool verbose){
  if (verbose) std::cout << "Checking that the node labels are not sparce and there are no multiedges... ";
  NetType tempNet;
  for (std::list<Link>::iterator linkIterator=linkList.begin() ; linkIterator != linkList.end(); linkIterator++ ){
    if (tempNet[linkIterator->source][linkIterator->dest]!=0){
      std::cerr <<"Error: The input file contains multi-edges."<<std::endl;
      return false;
    }
    tempNet[linkIterator->source][linkIterator->dest]=linkIterator->weight;
  }
  if (tempNet.size()!=netSize){
    std::cerr <<"Error: Node labels are sparse. Please name nodes from 0 to n-1."<<std::endl;
    return false;
  }
  if (verbose) std::cout << "Ok."<< std::endl;
  return true;
}


int percolation(char * fileName, const size_t k, const size_t weighted, const float threshold, const size_t weightFunction, std::string outputFile,bool verbose,bool sanityCheck)
{
    size_t numberOfLinks;
    size_t netSize;
    std::list<Link> linkList;
    
    // First read in the network from the file
    if (verbose) std::cout << "Reading in the network...\n";
    if (!getNetSizeAndLinkNumbers(fileName, netSize, numberOfLinks,linkList)) return EXIT_FAILURE;    
    if (verbose) std::cout<< "Number of nodes: " << netSize << "\nNumber of links: " <<numberOfLinks << "\n";

    //Check that the edge list is valid, this will waste some time
    if (sanityCheck) if (!validateLinkList(linkList,netSize,verbose)) return EXIT_FAILURE;

    // Finally, proceed with the clique percolation
    std::ifstream file(fileName);
    if (file.is_open())
    {
        NetType net(netSize);
        if (!weighted)
	  unweightedSCP(net, linkList, numberOfLinks, k, outputFile,verbose);
        if (weighted)
            weightedSCP(net, file, numberOfLinks, k, threshold, weightFunction, outputFile);
    }
    file.close();
    return EXIT_SUCCESS;
}

int main(int argc, char* argv[])
{
    size_t netSize, numberOfLinks;
    size_t k = 3;
    size_t weighted = 0;
    float threshold = 0;
    size_t weightFunction = 0;
    bool verbose=false;
    std::string outputFile;

    //-- Parse arguments
    for (size_t i = 2; i < argc; i++)
    {
      if (!strncmp(argv[i], "-k=", 3))
	k = atoi(argv[i] + 3);
      else if (!strncmp(argv[i], "-o=", 3))
	outputFile = argv[i] + 3;
      else if (!strcmp(argv[i], "-v"))
	verbose=true;
/*
      else if (!strcmp(argv[i], "-w"))
	weighted = 1;
      else if (!strncmp(argv[i], "-t=", 3))
	threshold = atof(argv[i] + 3);
      else if (!strcmp(argv[i], "-f"))
	weightFunction = atof(argv[++i]);
*/      else{
	std::cerr << "Invalid argument: "<<argv[i] <<std::endl;
	std::cerr << HELPSTR<<std::endl;
	return EXIT_FAILURE;	  
      }	        
    }

    //--- Sanity checks for the input arguments

    //Check that the input file was given
    if (argc==1){
      std::cerr << "Invalid number of arguments."<<std::endl;
      std::cerr << HELPSTR<<std::endl;
      return EXIT_FAILURE;
    }

    //Check that the clique size is valid
    if (k < 3)
    {
        std::cerr << "Invalid value of clique size k: " << k << std::endl << "The value of k must be 3 or larger." << std::endl;
        return EXIT_FAILURE;
    }
    //Check the the output file is given. If not, use "[inputfile]_output"
    if (outputFile.empty())
    {
        outputFile = argv[1];
        outputFile.append("_output");
    }


    //--- Run clique percolation    
    int exitCode=percolation(argv[1], k, weighted, threshold, weightFunction, outputFile,verbose,true);


    // calculate timings
    if (verbose)
      std::cout << "Time used: " << (double)clock() / (double)CLOCKS_PER_SEC << "s" << std::endl;

    return exitCode;
}


