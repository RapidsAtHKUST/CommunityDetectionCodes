
#include "utilityGraphPartitioner.h"
//#define PRINT_DEBUG_INFO_
using namespace std;

void computeEdgeCut( MilanGraphCompressedTwice &inputGraph, 
		     vector<MilanLongInt>::iterator VertexPartitioning, MilanLongInt P ) 
{
  MilanLongInt NVer=0, NEdge=0;
  MilanBool isC=false;
  
  //Get the iterators for the graph:
  vector<MilanLongInt>::iterator verPtr  = inputGraph.getVerPtr_b();
  vector<MilanLongInt>::iterator verInd  = inputGraph.getVerInd_b();
  vector<MilanReal>::iterator edgeWeight = inputGraph.getEdgeWt_b();
  inputGraph.getGraphParamters(NVer, NEdge, isC);
  cout<<"|V|= "<<NVer<<" |E|= "<<NEdge<<endl;

  MilanLongInt myProcId=0, myEdgeCut=0, totalEdgeCut = 0, minEdgeCut = NEdge+1, maxEdgeCut = 0;
  MilanLongInt i=0, k=0, adj1=0, adj2=0;
  
  vector<MilanLongInt> edgesPerPart, edgeCutPerPart; //Vertices per processor
  //Reserve the Memory for Graph Data Structure:
  try
    {
      edgesPerPart.reserve(P);
      edgeCutPerPart.reserve(P);
    }
  catch ( length_error )
    {
      cerr<<"Within Function: computeEdgeCut() \n";
      cout<<"Error: Not enough memory to allocate \n";
      exit(1);
    }
  edgesPerPart.resize(P, 0);
  edgeCutPerPart.resize(P, 0);

  for( i = 0; i<NVer; i++ )   //O(|E|)
    {     
      adj1 = verPtr[i]; 
      adj2 = verPtr[i+1];
      myProcId = VertexPartitioning[i]; //What is my cluster/Processor
      myEdgeCut = 0;
      for( k = adj1; k < adj2; k++ ) //Add edges adj(vi)
	if ( myProcId != VertexPartitioning[verInd[k]] ) //Look at the other end
	  myEdgeCut++;
      
      totalEdgeCut = totalEdgeCut +  myEdgeCut; //Total Edgecut      
      if ( myEdgeCut > maxEdgeCut )
	maxEdgeCut = myEdgeCut;
      if ( myEdgeCut < minEdgeCut )
	minEdgeCut = myEdgeCut;

      edgesPerPart[myProcId]   += (adj2-adj1);
      edgeCutPerPart[myProcId] += myEdgeCut;      
    } //End of outer for loop
  cout<<"----------------------------------------------------------\n";
  cout<<"Data partitioning statistics:\n";
  cout<<"----------------------------------------------------------\n";
  cout<<"Vertex-level statistics:"<<endl;
  cout<<"Total edgecut       : "<<totalEdgeCut/2<<endl;
  cout<<"Average edgecut/Vtx : "<<totalEdgeCut/NVer<<endl;
  cout<<"Min edgecut/Vtx     : "<<minEdgeCut<<endl;
  cout<<"Max edgecut/Vtx     : "<<maxEdgeCut<<endl;  
  cout<<"----------------------------------------------------------\n";  
  
  minEdgeCut = NEdge+1; maxEdgeCut=0; totalEdgeCut=0;
  MilanLongInt minEdges=NEdge+1, maxEdges=0, totalEdges=0, myEdges=0;
  for( i = 0; i<P; i++ )   //O(|P|)
    { 
      myEdges = edgesPerPart[i];
      myEdgeCut = edgeCutPerPart[i];
      totalEdgeCut = totalEdgeCut +  myEdgeCut; //Total Edgecut
      totalEdges = totalEdges +  myEdges; //Total Edges

      if ( myEdgeCut > maxEdgeCut )
	maxEdgeCut = myEdgeCut;
      if ( myEdgeCut < minEdgeCut )
	minEdgeCut = myEdgeCut;

      if ( myEdges > maxEdges )
	maxEdges = myEdges;
      if ( myEdges < minEdges )
	minEdges = myEdges;
      //cout<<"Partition["<<i+1<<"] = |Ep|= "<<myEdges<<"\t |Ecp|= "<<myEdgeCut<<endl;
    }
  //cout<<"----------------------------------------------------------\n";  
  cout<<"Partition-level statistics:"<<endl;
  cout<<"Total edgecut        : "<<totalEdgeCut/2<<endl;
  cout<<"Average edgecut/Part : "<<totalEdgeCut/P<<endl;
  cout<<"Min edgecut/Part     : "<<minEdgeCut<<endl;
  cout<<"Max edgecut/Part     : "<<maxEdgeCut<<endl;
  cout<<"Max edges/Part       : "<<maxEdges <<endl;
  cout<<"Min edges/Part       : "<<minEdges <<endl;
  cout<<"----------------------------------------------------------\n";  
  cout.precision(2);
  cout.setf(ios::showpoint);
  cout<<"Ratio of edges cut   : "<<((double)(totalEdgeCut/2) / (double)NEdge) <<endl;
  if (minEdges > 0)
    cout<<"Load imbalance       : "<<(double)maxEdges/(double)minEdges<<endl;
  else
    cout<<"Load imbalance       : "<<maxEdges<<endl;  
  cout<<"----------------------------------------------------------\n";
  if ( (totalEdges/2) != NEdge )
    cout<<"WARNING!! totalEdge/2= "<<(totalEdges/2)<<" NEdges= "<<NEdge<<endl;
}


/*---------------------------------------------------------------------------*/

///// METIS as a Graph Partitioner ///////
void MetisGraphPartitioner( MilanGraphCompressedTwice &inputGraph, 
			    vector<MilanLongInt>::iterator VertexPartitioning, 
			    MilanLongInt K_way, MilanLongInt PartAlgo )
{
#ifdef PRINT_DEBUG_INFO_
  cout<<" Within MetisGraphPartitioner(): \n" ;
  cout<<" Number of partitions requested: "<<K_way<<endl;fflush(stdout);
#endif
  MilanLongInt NVer=0, NEdge=0;
  MilanBool isC=false;
  
  //Get the iterators for the graph:
  vector<MilanLongInt>::iterator verPtr  = inputGraph.getVerPtr_b();
  vector<MilanLongInt>::iterator verInd  = inputGraph.getVerInd_b();
  vector<MilanReal>::iterator edgeWeight = inputGraph.getEdgeWt_b();
  inputGraph.getGraphParamters(NVer, NEdge, isC);

#ifdef PRINT_DEBUG_INFO_
  cout<<" Number of vertices: "<<NVer<<"\n" ;fflush(stdout);
  cout<<" Number of edges   : "<<NEdge<<"\n" ;fflush(stdout);
#endif
    
  //Variables for calling Metis graph partitioner:
  int wgtflag = 0, numflag=0; 
  vector<int> options;
  options.resize(5,0);
  options[0]= 0; //zero means default values
  options[1]= 3; 
  options[2]= 1;
  options[3]= 3;
  options[4]= 0;
  int edgecut=0;
  
  //vector<int> part;
  //part.reserve(NVer);
  //part.resize(NVer,0);
  //Call the Metis Function:
  switch ( PartAlgo )
    {
    case 0:  METIS_PartGraphKway( &NVer, &verPtr[0], &verInd[0], NULL, NULL, 
				  &wgtflag, &numflag, &K_way, &options[0], 
				  &edgecut, &VertexPartitioning[0]); break;

    case 1:  METIS_PartGraphRecursive( &NVer, &verPtr[0], &verInd[0], NULL, NULL, 
				  &wgtflag, &numflag, &K_way, &options[0], 
				  &edgecut, &VertexPartitioning[0]); break;

    case 2:  METIS_PartGraphVKway( &NVer, &verPtr[0], &verInd[0], NULL, NULL, 
				  &wgtflag, &numflag, &K_way, &options[0], 
				  &edgecut, &VertexPartitioning[0]); break;
    
    default: METIS_PartGraphKway( &NVer, &verPtr[0], &verInd[0], NULL, NULL, 
				  &wgtflag, &numflag, &K_way, &options[0], 
				  &edgecut, &VertexPartitioning[0]);
  
    }
  //cout<<"Graph Partitioning by Metis, Edge cut is:"<<edgecut<<"\n" ;fflush(stdout);
  cout<<"Edge cut: "<<edgecut<<"\t" ;fflush(stdout);
  
  //for (int i=0; i<NVer; i++)
  //  VertexPartitioning[i]=part[i];
#ifdef PRINT_DEBUG_INFO_
  cout<<" Returned back from Metis. The edge cut is:"<<edgecut<<"\n" ;fflush(stdout);
  cout<<" End of function MetisGraphPartitioner() "<<endl;fflush(stdout);
#endif
} //End of MetisGraphPartitioner()

