/*
 * File:   TseGraph.cpp
 * Author: T.S.Evans, Physics Dept., Imperial College London
 * TseGraph is for weighted or unweighted, undirected  graphs.
 * Much of the code is OK for directed graphs but this is not fully implemented.
 * See TseGraph.h for more information.
 * Please acknowledge use of this code if you use this code.
 * Created on 14 August 2009, 15:25
 */

#include "TseGraph.h"

using namespace std;

TseGraph::TseGraph(){
weightedGraph=false;
directedGraph=false;
}

/**
 *Sets initial size and graph to be undirected and unweighted.
 */
TseGraph::TseGraph(int numberVertices, int numberStubs){
 setSize(numberVertices, numberStubs);
 weightedGraph=false;
 directedGraph=false;
 }

/**
 * Allocate initial memory but this can be exanded as we go.
 */
void
TseGraph::setSize(int numberVertices, int numberStubs){
// would like to allocate memory but not sure how.
// vertexToStub(numberVertices);
//  stubToVertex(numberStubs);
    if (vertexToStub.size()<numberVertices)
                        vertexToStub.resize(numberVertices);
 }


void
TseGraph::read(char *filename, bool weightsOn){

  cout << "Starting to read file " << filename << endl;
  ifstream finput;
  finput.open(filename,fstream::in);
  if (!finput) {
  cerr << "Can't open input file " << filename << endl;
  exit(1);
}


  weightedGraph = weightsOn;
  unsigned int lineNumber=0;

  int source , target, weight=-1;

  while (!finput.eof()) {
    lineNumber++;  
    source , target, weight=-1; // problem with last line of file
    if (weightedGraph)
      finput >> source >> target >> weight;
    else
      finput >> source >> target;

    if ((source<0) || (target<0)) cerr << "!!! Warning line number " << lineNumber << " ignored: source " << source <<", target " << target << endl;
    else if (finput) {// finput test deals with end of file issues
            //cout << source << " - " << target << "\n";
         addEdgeSlow(source, target, weight);}
    
    if (getNumberStubs()%1000000==0) {cerr << "."; fflush(stderr);}
    if (getNumberStubs()%10000000==0) {cerr << getNumberVertices() << "\n"; fflush(stderr);}

  }

  finput.close();
  cout << "\nFinished reading file " << filename << endl;


}

void
TseGraph::write() {
       int source,target=-1;
   for (int stub=0; stub<getNumberStubs(); stub++)
  {
      source=stubToVertex[stub++];
      target=stubToVertex[stub];
      cout <<source << "\t" << target;
      if (weightedGraph) cout << "\t" <<  edgeWeight[stub>>1];
      cout << endl;
    }
}


void
TseGraph::write(char *outFile) {
  ofstream fout(outFile);
    if (!fout) {
  cerr << "Can't open output file " << outFile << endl;
  exit(1);
}
int source,target=-1;
   for (int stub=0; stub<getNumberStubs(); stub++)
  {
      source=stubToVertex[stub++];
      target=stubToVertex[stub];
      fout << source << "\t" << target;
      if (weightedGraph) fout << "\t" <<  edgeWeight[stub>>1];
      fout << endl;
    }
  fout.close();
}


bool
TseGraph::isWeighted(){return weightedGraph;}

bool
TseGraph::isDirected(){return directedGraph;}

void
TseGraph::setWeighted(bool b){weightedGraph=b;}

void
TseGraph::setDirected(bool b){
if (b) {cerr <<"Currently only undirected graphs allowed for TseGraph.\n"; exit(0);}
directedGraph=b;}

bool 
TseGraph::check(){
 return  (edgeWeight.size()*2 == stubToVertex.size());
 } 
  

int
TseGraph::getNumberStubs(){return stubToVertex.size();}

int
TseGraph::getNumberVertices(){return vertexToStub.size();}

int
TseGraph::getVertexDegree(int v){return vertexToStub[v].size();}

/**
 * Finds the strength of a vertex.
 * Works for weighted and unweighted graphs.
 * If directed this is the out strength.
 */
double
TseGraph::getVertexStrength(int v){
    int k=getVertexDegree(v);
    if (!weightedGraph) return k;
    double s=0;
    for (int n=0; n<k; n++) s+=edgeWeight[(vertexToStub[v][n]>>1)];
    return s;
    }

int
TseGraph::getStub(int v, int n){
	return vertexToStub[v][n];
}


/**
 * Finds the strength of a stub.
 * Works for weighted and unweighted graphs.
 * Works for directed and undirected graphs.
 */
double
TseGraph::getStubWeight(int stub){
	return (weightedGraph?edgeWeight[stub>>1]:1);
}

/*
 * Adds a new vertex with no stubs.  No checks on size performed.
 */
void
TseGraph::addVertex(){vertexToStub.push_back(vector<int>());}



      


/**
* Adds a new edge from existing vertices s and t.
*If graph is weighted then weight of 1.0 is assumed.
* No checks on vertex indices performed.
*/
void 
TseGraph::addEdge(int s, int t){
  addEdge(s,t,1.0);
}

/**
 * Adds a new edge from existing vertices s and t.
 * If graph is weighted then makes new edge of weight.
 * Checks on vertex indices and increases size if needed.
*/
void
TseGraph::addEdgeSlow(int s, int t, double w){
  if (vertexToStub.size()<=max(s,t))
                        vertexToStub.resize(max(s,t)+1);
  addEdgeUnweighted(s,t);
  if (weightedGraph) edgeWeight.push_back(w);
}
      

/**
 * Adds a new edge from existing vertices s and t.
 * If graph is weighted then makes new edge of weight.
 * No checks on vertex indices performed.
*/
void
TseGraph::addEdge(int s, int t, double w){
  addEdgeUnweighted(s,t);
  if (weightedGraph) edgeWeight.push_back(w);
}

/**
 * Adds a new edge from existing vertices s and t.
 * Assumes graph is unweighted. 
* No checks on vertex indices performed.
*/
void
TseGraph::addEdgeUnweighted(int s, int t){
  vertexToStub[s].push_back(stubToVertex.size());
  stubToVertex.push_back(s);
  vertexToStub[t].push_back(stubToVertex.size());
  stubToVertex.push_back(t);
}


/**
  * Adds weight dw to edge from s to t.  Creates new edge if needed.
  * Assumes graph is unweighted.
  * No checks on vertex indices performed.
*/
void
TseGraph::addEdgeUnique(int s, int t){
	int stub=findStub(s,t);
	if (stub>=getNumberStubs() ) addEdgeUnweighted(s,t);
}
/**
  * Adds weight dw to edge from s to t.  Creates new edge if needed.
  * Assumes graph is unweighted.
  * No checks on vertex indices performed.
  */
void
TseGraph::increaseWeight(int s, int t, double dw){
	int stub=findStub(s,t);
	if (stub<getNumberStubs() ) edgeWeight[stub >>1] +=dw;
	else addEdge(s,t,dw);
}

/**
 * Finds if vertex s is connected to vertex t
 * returns stub number of s.  If this is equal to numberStubs
 * then no edge was found.
 * No checks on vertex indices performed.
  */
int
TseGraph::findStub(int s, int t){
        int k=vertexToStub[s].size();
	for (int n=0; n<k; n++){
		int stub = vertexToStub[s][n]  ;
		if (stubToVertex[(stub^1)]==t) return stub;
	}
	return getNumberStubs();
}









