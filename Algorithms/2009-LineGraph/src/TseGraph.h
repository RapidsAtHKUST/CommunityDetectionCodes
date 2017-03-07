#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <vector>

using namespace std;

/*
 * Definitions.
 *
 * Vertices have index v, 0<= v < (number vertices).
 *
 * Edges have index e, 0<=e<(number edges) 
 * Each edge has an outgoing stub, index (2e) and incoming stub index (2e+1). 
 * Thus total number of stubs is 2*(number edges).  Note that and exclusive or
 * (s ^1) whill get you from one stub to its partner.  As integer division 
 * drops the remainders e=s/2 is always the edge associated with stub s.
 * While the stub directions have no meaning for undirected graphs it is still 
 * convenient to maintain this convention.  It also means the code is easily 
 * adapted for directed graphs. If the edges are weighted then a vector of 
 * doubles stores the weights so edgeWeight[e]=w weight of edge s and so of 
 * stubs (2e) and (2e+1).
 *
 * The graph is stored using incident matrices.  First the vertex v at the end 
 * of each stub s is listed in the stubToVertex vector.  Thus stubToVertex[s]=v.
 * Secondly each vertex v has a list of incident stubs.  This means each
 * stub has a local index, n (for neighbour), indicating it is the n-th stub 
 * of vertex v.  Thus 0 <= n < k=(degree of vertex v).  This stub has a global 
 * index s and this is what the second incident matrix tells us.  So
 * vertexToStub[v][n]=s.  
 * 
 * Note that for directed graphs there will need to be
 * two vertexToStub lists, one for incoming stubs and one for outgoing stubs.  
 * Its recommended that vertexToStub is used for outgoing and a second one, 
 * vertexToIncomingStub list is used for incoming stubs.
*/
class TseGraph {
    
 /* Most of these variables should be private but need to create appropriate 
  *  methods to access them.
  */
 public:

/* Edges are Let edge index 0<=e<(number edges) outgoing stub 2e and incoming stub (2e+1)
 * stubToVertex[2e]=s, global index of source vertex for stub 2e
 * stubToVertex[2e+1]=t, global index of target vertex for stub 2e+1
 */
vector<int> stubToVertex;

/* vertexToStub[v] is a vector of neighbours of vertex v.
 * vertexToStub[v][n]=s tells us the n-th stub of vertex v (n for neighbour a
 * local stub index) is stub of global index s.
 */
vector<vector<int>  > vertexToStub;

// edgeWeight edgeWeight[e] is weight of edge e (stubs 2e and 2e+1)
vector<double> edgeWeight;

TseGraph();
TseGraph(int , int );

void setSize(int , int );

void read(char*, bool);
void write();
void write(char *);

bool isWeighted();
bool isDirected();
void setWeighted(bool);
void setDirected(bool);

int getNumberStubs();
int getNumberVertices();
int getVertexDegree(int);
double getVertexStrength(int);


void addVertex();

void addEdge(int , int);
void addEdge(int , int, double );
void addEdgeSlow(int , int, double );
void addEdgeUnweighted(int , int );
void addEdgeUnique(int , int );

int getStub(int , int );
int findStub(int , int );
double getStubWeight( int);
void increaseWeight(int, int, double);

bool check();

//TseGraph& makeLineGraph(TseGraph , int , bool );


private:

bool weightedGraph;
bool directedGraph;



};

