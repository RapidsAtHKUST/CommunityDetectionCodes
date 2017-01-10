/*
 * LPA.cpp
 *
 *  Created on: Feb 28, 2012
 *      Author: Jerry
 */

//Using copra's idea of averaging the belonging coefficients to
// stabilize the algorithm

#include "LPA.h"
#include "CommonFuns.h"
#include "CommonFuns_TMP.h"
#include "rndNumbers.h"
#include "fileOpts.h"

#include <pthread.h>
#include "TieLabel.h"

#include <cmath>

typedef std::tr1::unordered_map<int, int> UOrderedH_INT_INT;
typedef std::tr1::unordered_map<int, double> UOrderedH_INT_DBL;

bool LPA::isDEBUG=false;

LPA::LPA(string inputFileName,int maxT,\
		string outputDir,bool isUseLargestComp,bool isSymmetrize,bool isSyn,\
		bool isSaveicpm,int run) {
	//inputFileName: the full path
	//netName: short filename(non-suf)

	//---------------------------
	//Extract the fileName
	//---------------------------
	string a,b;
	fileName_net=inputFileName;
	extractFileName_FullPath(inputFileName,netName,a,b);

	networkPath="";
	net=new Net(networkPath,netName,fileName_net); //could be added with self-loop

	//---------------------------
	//		GLPA parameters
	//---------------------------
	this->maxT=maxT;

	this->isSyn=isSyn;
	this->isUseLargestComp=isUseLargestComp;
	this->isSymmetrize=isSymmetrize;

	//---------------------------
	//		more
	//---------------------------
	this->outputDir=outputDir;

	this->numThreads=1; //**not used

	//---------------------------
	this->run=run;
	this->isSaveicpm=isSaveicpm;

	start();
}

LPA::~LPA() {
	delete net;
}

void LPA::start(){
	cout<<"****isSyn="<<isSyn<<endl;
	cout<<"****maxT="<<maxT<<endl;
	cout<<"****run="<<run<<endl;

	if(isSyn)
		shortfileName="syn_LPA_"+netName+"_T"+int2str(maxT)+"_run"+ int2str(run);
	else
		shortfileName="Asyn_LPA_"+netName+"_T"+int2str(maxT)+"_run"+ int2str(run);

	//---------------------------------------
	//	   LOAD network:
	//	(*LPA*)-READ IN as it is, Only make it symmetric
	//           Keep the original IDs
	//----------------------------------------
	net->readNetwork_EdgesList(fileName_net,isUseLargestComp,isSymmetrize);
	cout<<"net.N="<<net->N<<" M="<<net->M<<endl;

	//---------------------------
	//  	game
	//---------------------------
	//initialization P/M and clean network
	init();

	if(isSyn)
		LPA_syn_pointer();
	else
		LPA_Asyn_pointer();
}

void LPA::init(){
	//1. initialize the label
	cout<<"init label=ID...."<<endl;

	NODE *v;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];
		v->label=v->ID; //***
	}
}

void LPA::LPA_syn_pointer(){
	cout<<"Start iteration (SYN):"<<endl;

	NODE *v,*nbv;
	vector<int> notused;  //not used
	bool notusedB;        //not used
	int t;

	for(t=1;t<=maxT;t++){
		//cout<<"-------------t="<<t<<"---------------------"<<endl;

		//-----------------------------------
		//1.do update(compute v->newlabel(t) based on nb's v->label(t-1))
		//-----------------------------------
		for(int i=0;i<net->N;i++){
			v=net->NODES[i];

			//adopt the majority label in nbs (break tie by random)
			vector<int> WS;
			for(int k=0;k<v->nbList_P.size();k++){
				WS.push_back(v->nbList_P[k]->label); //use *old* label
			}

			//update *new* label
			v->newlabel=ceateHistogram_selRandMax(notused,WS,0,notusedB);
		}

		//--------------------------------------
		//2.**(SYN) ** Do the delayed update
		//--------------------------------------
		for(int i=0;i<net->N;i++){
			v=net->NODES[i];
			v->label=v->newlabel;
		}

		//-----------------------------------
		//3.check convergence (v->label(t))
		//-----------------------------------
		if(isConverge()) break;   //no one need to change,stop
	}

	bestT=t;
	//--------------------------------------
	//create cpm, compute Q and save icpm
	string fileName_icpm;
	fileName_icpm=outputDir+shortfileName+".icpm";

	LPA_createCPM_pointer_computeQ(fileName_icpm);

	cout<<"Q("<<t<<")="<<bestQ<<" K="<<bestK<<endl;
}

void LPA::LPA_Asyn_pointer(){
	cout<<"Start iteration (ASYN):"<<endl;

	NODE *v,*nbv;
	vector<int> notused;   //not used
	bool notusedB;         //not used
	int t;


	for(t=1;t<=maxT;t++){
		//cout<<"-------------t="<<t<<"---------------------"<<endl;

		//0.**shuffle** //BUG:????why the first element not shuffled????
		srand (time(NULL)); // ***YOU need to use this, such that you can get a new one each time!!!!! seed the random number with the system clock
		random_shuffle (net->NODES.begin(), net->NODES.end());
		//net->showVertices();

		//-----------------------------------
		//1.do update(compute v->label(t))
		//-----------------------------------
		for(int i=0;i<net->N;i++){
			v=net->NODES[i];

			//adopt the majority label in nbs (break tie by random)
			vector<int> WS;
			for(int k=0;k<v->nbList_P.size();k++){
				WS.push_back(v->nbList_P[k]->label); //use *current* label
			}

			//update **label
			v->label=ceateHistogram_selRandMax(notused,WS,0,notusedB);
		}

		//-----------------------------------
		//2.check convergence (v->label(t))
		//-----------------------------------
		if(isConverge()) break;   //no one need to change,stop

	}

	bestT=t;
	//--------------------------------------
	//create cpm, compute Q and save icpm
	string fileName_icpm;
	fileName_icpm=outputDir+shortfileName+".icpm";

	LPA_createCPM_pointer_computeQ(fileName_icpm);

	cout<<"Q("<<t<<")="<<bestQ<<" K="<<bestK<<endl;

}

bool LPA::isConverge(){
	//no change==converge
	bool ischanged=false,amIok;
	NODE *v,*nbv;
	vector<int> notused;   //not used
	//-----------------------------------
	//2.check convergence (v->label(t))
	//-----------------------------------
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		vector<int> WS;
		for(int k=0;k<v->nbList_P.size();k++){
			WS.push_back(v->nbList_P[k]->label);
		}

		//check majority: if my label is in any of the max groups
		ceateHistogram_selRandMax(notused,WS,v->label,amIok);
		if(!amIok){
			ischanged=true;  //need to move on to t+1
			break;
		}
	}

	return !ischanged;
}

//**MODIFIED version** since LPAt(i.e., >LPAv1.3)
int LPA::ceateHistogram_selRandMax(vector<int>& topLabels,const vector<int>& wordsList,int oldlabel,bool & amIok){
	//return the random select label,and
	//**optionally, get all the top lables topLabels.

	//****becare full the ***BUG*** in **double maxv**

	int label;
	map<int,double> hist;
	//------------------------------------------
	//	    1. create the histogram
	//------------------------------------------
	//count the number of Integer in the wordslist
	createHistogram(hist, wordsList);

	//printHistogram_INT_DBL(hist);
	//------------------------------------------
	//2. randomly select label(key) that corresponding to the max *values*.
	//	    sort the key-value pairs, find the multiple max
	//		randomly select one and return the label(key)
	//------------------------------------------
	//***list is int he decreasing order of value.
	vector<pair<int,double> > pairlist;
	sortMapInt_DBL(hist, pairlist);

	//****BUG***
	double maxv=pairlist[0].second;
	int cn=1;

	for(int i=1;i<pairlist.size();i++){    //start from the **second**
		if(pairlist[i].second==maxv)       //multiple max
			cn++;
		else
			break; //stop when first v<maxv
	}

	if(cn==1)
		label=pairlist[0].first;         //key-label
	else{
		int wind=mtrand1.randInt(cn-1); //**[0~n]
		label=pairlist[wind].first;

		//cout<<" random[0,"<<cn-1<<"]->"<<wind<<" ";
	}
	//-------------------------------
	//		get these top labels
	//-------------------------------
	//cout<<" topLabel::";
	topLabels.clear();
	for(int i=0;i<cn;i++){
		topLabels.push_back(pairlist[i].first);       //key-label
		//cout<<pairlist[i].first<<" ";
	}
	//-------------------------------
	//		check majority rule
	//-------------------------------
	amIok=false;
	for(int i=0;i<cn;i++){
		if(oldlabel==pairlist[i].first) {
			amIok=true;
			//cout<<"ok"<<endl;
			break;
		}
	}

	//cout<<" Selected::"<<label<<endl;

	return label;
}


void LPA::createCPM_pointer(vector<vector<int>* >& cpm){
	//return cpm for LPA (each node has only one label)

	//the map of distinct label and community id
	map<int,int> w_comidTable;
	int comid=-1; //**-1, such that we can access via vector[comid(0+)]

	NODE *v;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		//create CPM:put each membership to a community
		int label=v->label;  //***

		//------------
		if(w_comidTable.count(label)==0){//not in yet
			comid++;
			w_comidTable.insert(pair<int,int>(label, comid)); //**

			//***CPMPP
			vector<int>* avector=new vector<int>();  //**TO REMOVE
			cpm.push_back(avector);  //copy to the (an empty vector)
		}

		//------------
		int v_comid=w_comidTable[label];
		cpm[v_comid]->push_back(v->ID);  //add one *id*
	}
}

void LPA::convert_cpmVectPtr2cpm(vector<vector<int>* >& cpmptr, vector<vector<int> >& cpm){
	cpm.clear();
	for(int i=0;i<cpmptr.size();i++){
		vector<int> a(*cpmptr[i]);

		cpm.push_back(a);
	}
	//cout<<"cpm.size="<<cpm.size()<<endl;
}


void LPA::LPA_createCPM_pointer_computeQ(string fileName){
	bool isDEBUG=true;
	time_t st;

	//CPM: the index is the **commID(0~k-1)**, and vales is node **ID**
	vector<vector<int>* > cpm; //***CPMP, TO REMOVE

	//=========================================
	//1.createCPM
	//=========================================
	createCPM_pointer(cpm);

	//printAllLabels();
	//sort_cpm_pointer(cpm);
	//printVectVect_INT_pointer(cpm);
	//=========================================
	//2.***post process*** the communities CPM
	//=========================================
	//a. reassign sameLabel disconnected subcomponent
	//(handle the same label disconnected components problems in general)

	//before label rank, it is disable (not easy to implement a cpm-pointer versin)
	if(true) {
		cout<<"reassign sameLabel disconnected subcomponent...."<<endl;
		//if(isDEBUG) cout<<"---before reassign disconnected subcomponent()---"<<endl;
		//if(isDEBUG) printVectVect_PRIMITIVE<int>(cpm);

		//in the function: the cpm is deleted and re-assignment
		cpm=net->pre_findAllConnectedComponents_InOneCluster_CPM_cpmpointer(cpm);

		//if(isDEBUG) cout<<"---After reassign---"<<endl;
		//if(isDEBUG) printVectVect_PRIMITIVE<int>(cpm);
	}

	//---------------------------
	//b. remove subset
	//---------------------------
	//if(isDEBUG) cout<<"---before---"<<endl;
	//if(isDEBUG) printVectVect_PRIMITIVE<int>(cpm);

	st=time(NULL);

	//---------------------------
	//b.  option for multi-thread
	//---------------------------
	//*****do not use network structure*****
	cpm=net->post_removeSubset_UorderedHashTable_cpmpointer(cpm);

	cout<<"removeSubset takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;

	//if(isDEBUG) cout<<"---After---"<<endl;
	//if(isDEBUG) printVectVect_PRIMITIVE<int>(cpm);
	//---------------------------
	//4. save cpm
	//---------------------------
	sort_cpm_pointer(cpm);  //sort each com by increasing ID for; and by decrasing community size

	//---------------------------------
	//		compute Q
	//---------------------------------
	//*****A with selfloop may increase the Q!!!!!!!******

	DisjointM mq;
	vector<vector<int> > cpmIDs;
	convert_cpmVectPtr2cpm(cpm, cpmIDs);

	//**need the org_net***
	bestQ=mq.calQ_unw_givenCpm(cpmIDs,"",net,isUseLargestComp,isSymmetrize);
	bestK=cpm.size();

	if(isSaveicpm) write2txt_CPM_pointer(fileName,cpm);


	//---------------------------
	//release memory
	//---------------------------
	for(int i=0;i<cpm.size();i++)
		delete cpm[i];
}

void LPA::write2txt_CPM_pointer(string fileName,vector<vector<int>* >& cpm) {
	vector<string> data;

	for(int i=0;i<cpm.size();i++){
		//vector<int>& oneComm=cpm[i]; //ref
		vector<int>& oneComm=*cpm[i]; //**CPMP ref

		string line;
		for(int j=0;j<oneComm.size();j++){
			line+=int2str(oneComm[j]);
			line+=" ";
		}
		data.push_back(line);
	}

	//fileOpts.writeToTxt(fileName, false, data);// **false:the first one
	writeToTxt(fileName, false, data);
}

void LPA::printAllLabels(){
	cout<<"-----------------------"<<endl;
	NODE *v;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		cout<<"ID="<<v->ID<<" Label="<<v->label<<endl; //***
	}
}

void LPA::printHistogram_INT_DBL(map<int,double> &hist){
	cout<<"----------:";
	for(map<int,double>::iterator mit=hist.begin();mit!=hist.end();mit++){
		cout<<mit->first<<"("<<mit->second<<")";
	}
	cout<<endl;
}
//------------------------------------------------
//			org cpm functions
//------------------------------------------------
void LPA::sort_cpm(vector<vector<int> >& cpm){
	//inside each com, sort ID by **increasing** order
	for(int i=0;i<cpm.size();i++){
		sort(cpm[i].begin(),cpm[i].end(),sort_INT_INC());
	}

	//each com, sort **decreasing** community size
	sortVecVec_bySize<int>(cpm);
}

