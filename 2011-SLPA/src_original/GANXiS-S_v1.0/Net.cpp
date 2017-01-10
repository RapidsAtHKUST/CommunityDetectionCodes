//============================================================================
// Name        : Net.cpp
// Author      : Jierui Xie (jierui.xie@gmail.com)
// Date        : Oct. 2011
// Version     :
// Copyright   : All rights reserved.
// Description : SLPA algorithm for community detection.
// Web Site    : https://sites.google.com/site/communitydetectionslpa/
// Publication:
//             J. Xie, B. K. Szymanski and X. Liu, "SLPA: Uncovering Overlapping Communities in Social Networks via A Speaker-listener Interaction Dynamic Process", IEEE ICDM workshop on DMCCI 2011, Vancouver, CA.
//============================================================================

#include "Net.h"
#include "fileOpts.h"
#include "CommonFuns.h"
#include "CommonFuns_TMP.h"
#include <cstdlib>
#include <exception>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

#include "LabelRank.h"
//#include <tr1/unordered_set>

typedef std::tr1::unordered_set<int> UOrderedSet_INT;
typedef std::tr1::unordered_set<NODE *> UOrderedSet_NODEptr;


struct sortIDs {
	bool operator() (int i,int j) { return (i<j);} //increasing
} sortIDs_inc;


Net::Net(string path,string name,string fname){
	networkPath=path;
	netName=name;

	fileName=fname;

	isDebug=true;
}


Net::~Net() {
	while (!NODES.empty()) {
		delete NODES.back();
		NODES.pop_back();
	}
}

int Net::getNumberofEdges(){
	int m=0;
	for(int i=0;i<NODES.size();i++)
		m+=NODES[i]->numNbs;

	return m;
}

void Net::readNet(string fileName,bool isSymmetrize){
	//read the pairs file: 3 or 2 col, the w is IGNORED
	// Lines not starting with a number are ignored
	// *1.remove self loop
	// *2.auto symmetrize (no matter if it is already is)

	//changedNODESLIST: a set of nodes being updated(changed)
	//                  due to the change on edges
	changedNODESLIST.clear();


	string oneLine, whiteSpace=" \t\n";
	fstream fp;
	fp.open(fileName.c_str(),fstream::in); //|fstream::out|fstream::app

	if(fp.is_open()){//if it does not exist, then failed

		// repeat until all lines is read
		while ( fp.good()){
			getline (fp,oneLine);

			//--------------------------
			//skip empty line
			if(oneLine.find_first_not_of(whiteSpace)==string::npos) continue;

			//skip any line NOT starting with digit number
			if(!isdigit(oneLine.at(oneLine.find_first_not_of(whiteSpace)))) continue;

			//cout<<"Line:"<<oneLine<<endl;
			//====================================
			//	process one line(edge)
			//====================================
			int fromID,toID;
			double w=1.0;    //default value

			stringstream linestream(oneLine);
			if((linestream>>fromID) && (linestream>>toID)){
				//if(linestream>>w)
				//	cout<<"w="<<w<<endl;

				if(fromID==toID) continue; 				//**remove selfloop

				pre_ReadInOneEdge(fromID,toID);
				if(isSymmetrize)
					pre_ReadInOneEdge(toID,fromID);     //**symmetrize


   			    //------------------------------------
				//     tracking the node
				//------------------------------------
				//***what if toID(v) does not exist??
				if(NODESTABLE.count(fromID)>0)
					changedNODESLIST.insert(NODESTABLE[fromID]);
				else
					cout<<"WARNING::fromID="<<fromID<<" is not added as a change node!!"<<endl;

				if(NODESTABLE.count(toID)>0)
					changedNODESLIST.insert(NODESTABLE[toID]);
				else
					cout<<"WARNING::toID="<<toID<<" is not added as a change node!!"<<endl;

			}
		} //while

		fp.close();
	}
	else{
		cout<<"ERROR: open failed"<<endl;
		exit(1);
	}
}

void Net::readNetwork_EdgesList(string fileName, bool isUseLargestComp, bool isSymmetrize){
	//string(isFileExist(fileName.c_str()))

	//---------------------------------------
	if(!isFileExist(fileName.c_str())){
		cout<< fileName << " not found!" <<endl;
		exit(1);
	}

	NODES.clear();
	NODESTABLE.clear();

	//---------------------------------------
	// a. read in 2 or 3 col network file
	//    initialize NODES,NODESTABLE,v->nbSet
	//---------------------------------------
	time_t st=time(NULL);

	readNet(fileName,isSymmetrize);

	//------------------------------------
	//  b. initialize nbList using nbset (remove duplicated nbs)
	//------------------------------------
	pre_convert_nbSet2_nbList();
	N=NODES.size();
	M=getNumberofEdges();

	//cout<<"Reading in the network takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;
	//------------------------------------
	//   b. Using only the largest connected component
	//      do not use this for very large network
	//------------------------------------
	if(isUseLargestComp){
		cout<<"INFO: Using largest connected component only.......\n";
		//post_UseLargestComponent_UnorderSet_cpmpointer();
		pre_findGiantComponents();
	}



	//------------------------------------
	//------------------------------------
	if(isDebug){
		if(isSymmetrize)
			cout<<"INFO: N="<<N<< " M="<<M<<"(symmetric)"<<endl;
		else
			cout<<"INFO: N="<<N<< " M="<<M<<endl;

		//cout<<"load "<<fileName<< " done.."<<endl;
	}

	//net.showVertices();net->showVertices_Table();
}



void Net::pre_ReadInOneEdge(int fromID,int toID){
	//***WHY add only fromID?? (march 07,2012)
	//***What about toID does not have outgoing edges??
	map<int,NODE *>::iterator mit;
	NODE *vp;

	if (NODESTABLE.count(fromID)>0){//add nb
		//mit=NODESTABLE.find(fromID);
		//vp=mit->second;
		vp=NODESTABLE[fromID];

		vp->nbSet.insert(toID);
	}
	else{//add new node
		vp=new NODE(fromID);  //create
		vp->nbSet.insert(toID);

		NODES.push_back(vp);
		NODESTABLE.insert(pair<int,NODE *>(fromID,vp));
	}
}

void Net::showVertices(){
	cout<<"-----------------------"<<endl;
	cout<<"N="<<NODES.size()<<endl;
	for(int i=0;i<NODES.size();i++){
		NODE * vp=NODES[i];
		cout<< vp->ID<<endl;
	}
}

void Net::showVertices_Table(){
	cout<<"-----------------------"<<endl;
	//cout<<"N="<<NODESTABLE.size()<<endl;
	cout<<"N="<<N<<"("<<NODESTABLE.size()<<")"<<" M="<<M<<endl;

	map<int,NODE *>::iterator it;
	for(it=NODESTABLE.begin();it!=NODESTABLE.end();it++){
		NODE * v=it->second;
		cout<< v->ID<<endl;

		for(int j=0;j<v->numNbs;j++)
			cout<< "->" << v->nbList_P[j]->ID<<endl;
	}
}

void Net::pre_convert_nbSet2_nbList(){
	//now, we can use both nbList_P and nbSet
	//*use nbset rather that nbList_P is after symmetrized (not check),remove duplicated nb
	//  nbList is NOT available
	//
	UOrderedSet_INT::iterator sit;
	NODE * v;
	NODE * nbv;

	//create nbList-pointer version: ASSUMING the TABLES is ready
	for(int i=0;i<NODES.size();i++){
		v=NODES[i];

		v->nbList_P.clear();
		for(sit=v->nbSet.begin();sit!=v->nbSet.end();sit++){
			nbv=NODESTABLE.find(*sit)->second;
			v->nbList_P.push_back(nbv);  //pointer
		}
		v->numNbs=v->nbList_P.size();
	}
}




/*
void Net::pre_convert_nbSet2_nbList_org(){
	//now, we can use both nbList_P and nbSet
	//*use nbset rather that nbList_P is after symmetrized (not check),remove duplicated nb
	//  nbList is NOT available
	//
	set<int>::iterator sit;
	NODE * v;
	NODE * nbv;

	//create nbList-pointer version: ASSUMING the TABLES is ready
	for(int i=0;i<NODES.size();i++){
		v=NODES[i];

		v->nbList_P.clear();
		for(sit=v->nbSet.begin();sit!=v->nbSet.end();sit++){
			nbv=NODESTABLE.find(*sit)->second;
			v->nbList_P.push_back(nbv);  //pointer
		}
		v->numNbs=v->nbList_P.size();
	}
}*/

vector<vector<int>* > Net::post_removeSubset_UorderedHashTable_cpmpointer(vector<vector<int>* >& cpm){
	//no need to be in the net. it does not use structure
	//???????????????????????????????????
	//*****(after have newcpm) at the end,NEED to delete the old cpm pointers*****

	//cout<<"isDebug="<<isDebug<<endl;
	//if(isDebug)
	//    cout<<"start post_removeSubset_UorderedHashTable_cpmpointer()...."<<endl;

	time_t st;
	//bool isDEBUG=true;
	//if(isDEBUG) cout<<"removeSubset (Unordered HASH)............."<<endl;

	vector<vector<int>* > newcpm;


	//1. ***sort cpm by the community size(***decreasing***)
	st=time(NULL);
	sort_cpm_pointer(cpm);  //***CMPP

	//cout<<"sort_cpm takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;

	//2.check the subset relationship
	//cout<<"***before cpm.sie="<<cpm.size()<<endl;
	st=time(NULL);

	//2.1 vector of map corresponding to the sorted cpm(decreasing)
	vector<UOrderedH_INT_INT* > vectHTable;

	for(int i=0;i<cpm.size();i++){
		UOrderedH_INT_INT* H=new UOrderedH_INT_INT;  //**
		for(int j=0;j<cpm[i]->size();j++)            //***CMPP
			//H->insert(pair<int,int>(cpm[i][j],cpm[i][j])); //id as both key and value
			H->insert(pair<int,int>((*cpm[i])[j],(*cpm[i])[j])); //id as both key and value

		vectHTable.push_back(H);
	}

	//0.the indicator
	//vector<int> indicators;
	//for(int i=0;i<cpm.size();i++)
	//	indicators.push_back(1);  //1-default ok
	int indicators[cpm.size()];
	for(int i=0;i<cpm.size();i++)
		indicators[i]=1;  		  //1-default ok

	//2.2 find the subset (compare smaller cpmi to largest H(j) first)
	bool issubset;
	for(int i=cpm.size()-1;i>0;i--){
		for(int j=0;j<i;j++){
			//visit all coms(in HASH) that are LARGER than it
			//check if cpm(i) is a subset of H(j)
			issubset=true;
			for(int k=0;k<cpm[i]->size();k++)
				if(vectHTable[j]->count((*cpm[i])[k])==0){//not found
					issubset=false;
					break;
				}

			//issubset=issubset_cpm_hash(cpm[i],vectHTable[j]);

			if(issubset){  //remove this cpm
				indicators[i]=0;   //**change i**
				break;
			}
		}
	}

	//3.newcpm
	for(int i=0;i<cpm.size();i++){
		if(indicators[i]==0) continue;

		newcpm.push_back(cpm[i]);
	}

	//release memory
	for(int i=0;i<vectHTable.size();i++)
		delete vectHTable[i];

	return newcpm;
}

vector<vector<int>* >  Net::pre_findAllConnectedComponents_InOneCluster_CPM_cpmpointer(vector<vector<int>* >& cpm){
	//*****(after have newcpm) at the end,NEED to delete the old cpm pointers*****

	//***The cluster could be a sub cluster or the whole network***
	//INPUT: CPM(line contains one cluster with node ids). if a com consists of separate components, they become
	//       separate comms.
	// Trick: using set instead of list

	//OUTPUT: newCPM sorted in **decreasing** cluster size order


	map<int,NODE *>::iterator mit;
	NODE *v;

	//*****(use pointer)
	//SLPA::sort_cpm(cpm);
	//**sort BOTH inside(node id) and among comms(comm size)!**
	sort_cpm_pointer(cpm);

	//*****
	vector<vector<int>* > newcpm;

	for(int i=0;i<cpm.size();i++){
		//for each community
		//*****(use pointer)
		set<int> Com(cpm[i]->begin(),cpm[i]->end());  //copy one com

		while(Com.size()>0){
			//for each component
			set<int>  exploredSet;
			set<int>  unexploredSet;

			//first node
			int vid=getFirstElemnet_Set_PRIMITIVE<int>(Com);

			//set<int> nbSet=NODESTABLE.get(vid).nbSet;
			mit=NODESTABLE.find(vid);
			v=mit->second;

			//*********
			//set<int> nbSet=v->nbSet;
			set<int> nbSet(v->nbSet.begin(),v->nbSet.end());


			//Key**: confined to one cluster
			//set<int> newnbSet=mySet_Intersect_PRIMITIVE<int>(nbSet,Com);  //CollectionFuns.interSet(nbSet,Com);
			set<int> newnbSet=mySet_Intersect_PRIMITIVE<int>(nbSet,Com);  //CollectionFuns.interSet(nbSet,Com);

			unexploredSet.insert(newnbSet.begin(),newnbSet.end());        //unexploredSet.addAll(newnbSet);
			Com=mySet_Diff_PRIMITIVE<int>(Com,newnbSet);                  //Com.removeAll(newnbSet);

			exploredSet.insert(vid);   //exploredSet.add(vid);
			Com.erase(vid);            //Com.remove(vid);


			while(unexploredSet.size()>0){
				//first node
				vid=getFirstElemnet_Set_PRIMITIVE<int>(unexploredSet);  //vid=getFirstElemnetInSet(unexploredSet);
				mit=NODESTABLE.find(vid);					 //nbSet=NODESTABLE.get(vid).nbSet;
				v=mit->second;
				nbSet.clear();

				//**************
				//nbSet=v->nbSet;
				set<int> xx(v->nbSet.begin(),v->nbSet.end());
				nbSet=xx;

				//***Key: confined to one cluster
				newnbSet.clear();
				newnbSet=mySet_Intersect_PRIMITIVE<int>(nbSet,Com);       //CollectionFuns.interSet(nbSet,Com);
				unexploredSet.insert(newnbSet.begin(),newnbSet.end());  //unexploredSet.addAll(newnbSet);
				Com=mySet_Diff_PRIMITIVE<int>(Com,newnbSet);                        //Com.removeAll(newnbSet);

				unexploredSet.erase(vid); //unexploredSet.remove(vid);
				exploredSet.insert(vid);  //exploredSet.add(vid);
			}


			//*****(use pointer)
			//get a connected component
			vector<int> *oneComponent_ptr=new vector<int>(exploredSet.begin(),exploredSet.end());
			newcpm.push_back(oneComponent_ptr);
		}
	}

	//------------------------
	//*****(use pointer)
	//**sort BOTH inside(node id) and among comms(comm size)!**
	sort_cpm_pointer(newcpm);

	//------------------------
	if(newcpm.size()!=cpm.size()){
		cout<<"before K="<<cpm.size()<<" after post_sameLabelDisconnectedComponents() K="<<newcpm.size()<<endl;
		//cout<<"------------before----------"<<endl;
		//printVectVect_INT_pointer(cpm);
		//cout<<"------------after----------"<<endl;
		//printVectVect_INT_pointer(newcpm);
	}



	//---------------------------
	//release old cpm memory
	//---------------------------
	for(int i=0;i<cpm.size();i++)
		delete cpm[i];

	return newcpm;
}

void Net::post_addSelfloop(){
	//***duplication-safe
	//cout<<"add selfloop (duplication-safe) ...."<<endl;
	if(isDebug)
		cout<<"INFO: add selfloop ...."<<endl;

	NODE *v;

	for(int i=0;i<N;i++){
		v=NODES[i];

		//Only add when not yet exist
		if(v->nbSet.count(v->ID)==0){
			v->numNbs++;

			//itself as nb
			v->nbList_P.push_back(v);  //***(no duplication-safe)
			v->nbSet.insert(v->ID);

			M++;
		}
	}
}

/*
vector<vector<int> >  Net::pre_findAllConnectedComponents_InOneCluster_CPM(vector<vector<int> >& cpm){
	//***The cluster could be a sub cluster or the whole network***
	//INPUT: CPM(line contains one cluster wiht node ids). if a com consists of separate components, they become
	//       separate comms.
	// Trick: using set instead of list

	//OUTPUT: CPM sorted in **decreasing** cluster size order
	map<int,NODE *>::iterator mit;
	NODE *v;

	SLPA::sort_cpm(cpm);

	vector<vector<int> > newcpm;

	for(int i=0;i<cpm.size();i++){
		//for each community
		set<int> Com(cpm[i].begin(),cpm[i].end());  //copy one com

		while(Com.size()>0){
			//for each component
			set<int>  exploredSet;
			set<int>  unexploredSet;

			//first node
			int vid=getFirstElemnet_Set_PRIMITIVE<int>(Com);

			//set<int> nbSet=NODESTABLE.get(vid).nbSet;
			mit=NODESTABLE.find(vid);
			v=mit->second;
			set<int> nbSet=v->nbSet;


			//Key**: confined to one cluster
			set<int> newnbSet=mySet_Intersect_PRIMITIVE<int>(nbSet,Com);       //CollectionFuns.interSet(nbSet,Com);
			unexploredSet.insert(newnbSet.begin(),newnbSet.end());  //unexploredSet.addAll(newnbSet);
			Com=mySet_Diff_PRIMITIVE<int>(Com,newnbSet);                        //Com.removeAll(newnbSet);

			exploredSet.insert(vid);   //exploredSet.add(vid);
			Com.erase(vid);            //Com.remove(vid);


			while(unexploredSet.size()>0){
				//first node
				vid=getFirstElemnet_Set_PRIMITIVE<int>(unexploredSet);  //vid=getFirstElemnetInSet(unexploredSet);
				mit=NODESTABLE.find(vid);					 //nbSet=NODESTABLE.get(vid).nbSet;
				v=mit->second;
				nbSet.clear();
				nbSet=v->nbSet;

				//***Key: confined to one cluster
				newnbSet.clear();
				newnbSet=mySet_Intersect_PRIMITIVE<int>(nbSet,Com);       //CollectionFuns.interSet(nbSet,Com);
				unexploredSet.insert(newnbSet.begin(),newnbSet.end());  //unexploredSet.addAll(newnbSet);
				Com=mySet_Diff_PRIMITIVE<int>(Com,newnbSet);                        //Com.removeAll(newnbSet);

				unexploredSet.erase(vid); //unexploredSet.remove(vid);
				exploredSet.insert(vid);  //exploredSet.add(vid);
			}

			//get a connected component
			vector<int> oneComponent(exploredSet.begin(),exploredSet.end());
			newcpm.push_back(oneComponent);
		}
	}

	//------------------------
	//sorting
	SLPA::sort_cpm(newcpm);   //	Collections.sort(newcpm,ComCPMsizedec);

	//------------------------
	if(newcpm.size()!=cpm.size()){
		cout<<"before K="<<cpm.size()<<" after post_sameLabelDisconnectedComponents() K="<<newcpm.size()<<endl;
		//System.out.println("------------before----------");
		//show_cpm(cpm);
		//System.out.println("------------after----------");
		//show_cpm(newcpm);
	}

	return newcpm;
}
void Net::post_UseLargestComponent(){
	//***suppose we can use both nbList and nbSet***
	//com contains node id
	time_t st=time(NULL);

	vector<int> largestCom;

	//1. put all nodes in one big com
	vector<vector<int> > cpm;
	vector<int> oneCom;

	for(int i=0;i<NODES.size();i++){
		oneCom.push_back(NODES[i]->ID);
	}
	cpm.push_back(oneCom);

	cout<<"all nodes----------"<<endl;
	//SLPA::show_cpm(cpm);
	//2.find all components(sorted)
	vector<vector<int> > allcomponents_cpm=pre_findAllConnectedComponents_InOneCluster_CPM(cpm);

	cout<<"all connected----------"<<endl;
	//SLPA::show_cpm(allcomponents_cpm);

	//***Key:sorting and get largest
	SLPA::sort_cpm(allcomponents_cpm); //Collections.sort(allcomponents_cpm,ComCPMsizedec);
	largestCom=allcomponents_cpm[0];

	//Trick
	set<int> largestCom_Set(largestCom.begin(),largestCom.end());

	//3.reset the network as only the largest largestCom
	//change NODES,NODESTABLE and N
	vector<NODE *> newNODES;   //ArrayList<NODE> newNODES=new ArrayList<NODE>();
	//map<int,NODE *> NODESTABLE;

	for(int i=0;i<NODES.size();i++){
		NODE* v=NODES[i];
		int vid=v->ID;

		if(isSetMember_PRIMITIVE<int>(largestCom_Set,vid)){
			newNODES.push_back(v);
		}else{
			NODESTABLE.erase(vid);
			delete v;                  //**
		}

	}

	NODES.clear();

	NODES=newNODES;
	N=NODES.size();
	M=getNumberofEdges();

	//4.**assuming that we do not need to handle the nbList and nbSet

	cout<<"Finding largest component in the network takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;
}*/



/*
void Net::post_UseLargestComponent_UnorderSet_cpmpointer(){
	//***suppose we can use both nbList and nbSet***
	//com contains node id
	time_t st=time(NULL);

	vector<int> largestCom;

	//1. put all nodes in one big com
	vector<vector<int> > cpm;
	vector<int> oneCom;

	for(int i=0;i<NODES.size();i++){
		oneCom.push_back(NODES[i]->ID);
	}
	cpm.push_back(oneCom);

	cout<<"all nodes----------xx"<<endl;
	//SLPA::show_cpm(cpm);
	//2.find all components(sorted)
	vector<vector<int> > allcomponents_cpm=pre_findAllConnectedComponents_InOneCluster_CPMpointer_UnorderSet(cpm);

	cout<<"all connected----------xx"<<endl;
	//SLPA::show_cpm(allcomponents_cpm);

	//***Key:sorting and get largest
	SLPA::sort_cpm(allcomponents_cpm); //Collections.sort(allcomponents_cpm,ComCPMsizedec);
	largestCom=allcomponents_cpm[0];

	//Trick
	set<int> largestCom_Set(largestCom.begin(),largestCom.end());

	//3.reset the network as only the largest largestCom
	//change NODES,NODESTABLE and N
	vector<NODE *> newNODES;   //ArrayList<NODE> newNODES=new ArrayList<NODE>();
	//map<int,NODE *> NODESTABLE;

	for(int i=0;i<NODES.size();i++){
		NODE* v=NODES[i];
		int vid=v->ID;

		if(isSetMember_PRIMITIVE<int>(largestCom_Set,vid)){
			newNODES.push_back(v);
		}else{
			NODESTABLE.erase(vid);
			delete v;                  //**
		}
	}

	NODES.clear();

	NODES=newNODES;
	N=NODES.size();
	M=getNumberofEdges();

	//4.**assuming that we do not need to handle the nbList and nbSet

	cout<<"Finding largest component in the network takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;
}
 */

void Net::pre_findGiantComponents(){
	time_t st=time(NULL);
	//------------------------------------------
	//		find gaint component
	//------------------------------------------
	vector<vector<NODE *> > coms; //all components

	UOrderedSet_NODEptr UnExpSet(NODES.begin(),NODES.end());   // unexported set
	UOrderedSet_NODEptr WorkingSet; 							// unexported set

	NODE *v;
	NODE *nbv;

	while(!UnExpSet.empty()){
		//new com
		vector<NODE *> com;

		v=*UnExpSet.begin(); // take the first node

		//mark this node
		WorkingSet.insert(v);
		UnExpSet.erase(v);
		v->status=1;
		com.push_back(v);

		//find one com
		while(!WorkingSet.empty()){
			v=*WorkingSet.begin(); // take the first nb node

			//explore the nbs
			for(int i=0;i<v->nbList_P.size();i++){
				nbv=v->nbList_P[i];

				if(nbv->status==0){
					//mark this node
					WorkingSet.insert(nbv);
					UnExpSet.erase(nbv);
					nbv->status=1;
					com.push_back(nbv);
				}
			}

			//remove this v
			WorkingSet.erase(v);
		}


		// add to the list
		coms.push_back(com);
	}

	//----------------------------
	//	et the giant one and  update the network
	//----------------------------
	//***Key:sorting and get largest (decreasing size)
	sortVecVec_bySize<NODE* >(coms);

	cout<<"check the sorting of sizes:"<<endl;
	int sum=0;
	for(int i=0;i<coms.size();i++){
		cout<<coms[i].size()<<endl;
		sum+=coms[i].size();
	}
	cout<<" sum size="<<sum<<" N="<<N<<endl;

	//------------------------------------------
	//3.reset the network as only the largest largestCom
	//change NODES,NODESTABLE and N
	NODES.clear();
	NODES=coms[0];  //copy pointer

	NODESTABLE.clear();
	for(int i=0;i<NODES.size();i++)
		NODESTABLE.insert(pair<int,NODE *>(v->ID,NODES[i]));

	N=NODES.size();
	M=getNumberofEdges();

	//------------------------------------------
	//		remove other NODES(***DELETE***)
	//------------------------------------------
	//start *1*
	for(int i=1;i<coms.size();i++){
		for(int j=0;j<coms[i].size();j++)
			delete coms[i][j];  //one node
	}


	//4.**assuming that we do not need to handle the nbList and nbSet
	cout<<"Finding largest component in the network takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;
}

void Net::write2txt_Net2EdgesList(string fileName) {
	//read the nodes and their nbs, output the EdgesList
	// (i,j,w), w is set to 1 by default
	NODE * v;
	int fromID,toID,cn=0;

	vector<string> data;
	for(int i=0;i<NODES.size();i++){
		v=NODES[i];

		fromID=v->ID;
		for(int j=0;j<v->nbList_P.size();j++){
			toID=v->nbList_P[j]->ID;

			//constructe on line (edge)
			string line=int2str(fromID)+" "+int2str(toID)+" 1";

			data.push_back(line);

			cn++;
		}
	}

	//fileOpts.writeToTxt(fileName, false, data);// **false:the first one
	writeToTxt(fileName, false, data);

	cout<<"write "<<fileName<<" (N="<<NODES.size()<<" M="<<cn<<")!"<<endl;
}

//------------------------------------------------------------
//				Growing networks (temporal)
//------------------------------------------------------------
void Net::read_DetNetwork_EdgesList_add2CurNet(string fileName, bool isUseLargestComp, bool isSymmetrize){
	//read in the change in the edges list, i.e., Det G
	// add to the current net and update corresponding variable,N,M etc.

	// ***Seem that even not PURE change, it is fine because
	//    use nbSet.
	//
	//---------------------------------------
	if(!isFileExist(fileName.c_str())){
		cout<< fileName << " not found!" <<endl;
		exit(1);
	}

	//****************************************
	//           ONLY Different
	//****************************************
	//NODES.clear();
	//NODESTABLE.clear();

	//---------------------------------------
	// a. read in 2 or 3 col network file
	//    initialize NODES,NODESTABLE,v->nbSet
	//---------------------------------------

	readNet(fileName,isSymmetrize);

	//------------------------------------
	//  b. initialize nbList using nbset (remove duplicated nbs)
	//------------------------------------
	pre_convert_nbSet2_nbList();
	N=NODES.size();
	M=getNumberofEdges();

	//------------------------------------
	//   b. Using only the largest connected component
	//      do not use this for very large network
	//------------------------------------
	if(isUseLargestComp){
		cout<<"Using largest connected component only.......\n";
		//post_UseLargestComponent_UnorderSet_cpmpointer();
		pre_findGiantComponents();
	}


	//------------------------------------
	//------------------------------------
	if(isSymmetrize)
		cout<<"INFO: N="<<N<< " M="<<M<<"(make symmetric)"<<endl;
	else
		cout<<"INFO: N="<<N<< " M="<<M<<endl;

	//if(isDebug)
	// cout<<"load "<<fileName<< " done.."<<endl;

	//net.showVertices();net->showVertices_Table();
}


void Net::printchangedNODESLIST(){
	cout<<"nodes that change due to the change in edges:"<<endl;
	for(set<NODE *>::iterator sit=changedNODESLIST.begin();sit!=changedNODESLIST.end();sit++){
		cout<<(*sit)->ID<<endl;
	}

	cout<<"total:"<<changedNODESLIST.size()<<endl;
}

void Net::getSecondNBhood_add2changedNODESLIST(){
	NODE *v;
	set<NODE *> secondNBSet;

	cout<<"total(1-nbs):"<<changedNODESLIST.size()<<endl;
	for(set<NODE *>::iterator sit=changedNODESLIST.begin();sit!=changedNODESLIST.end();sit++){
		v=(*sit);

		for (int k=0;k<v->nbList_P.size();k++){
			secondNBSet.insert(v->nbList_P[k]);
		}
	}

	//add secondNBSet to changedNODESLIST
	for(set<NODE *>::iterator sit=secondNBSet.begin();sit!=secondNBSet.end();sit++){
		v=(*sit);
		changedNODESLIST.insert(v);
	}

	cout<<"total(2-nbs):"<<changedNODESLIST.size()<<endl;
}

