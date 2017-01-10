/*
 * LabelRank.cpp
 *
 *  Created on: Nov 15, 2011
 *      Author: Jierui Xie (jierui.xie@gmail.com)
 */

#include "LabelRank.h"
#include "CommonFuns.h"
#include "CommonFuns_TMP.h"
#include "rndNumbers.h"
#include "fileOpts.h"

#include <pthread.h>

#include <cmath>

typedef std::tr1::unordered_map<int, int> UOrderedH_INT_INT;
typedef std::tr1::unordered_map<int, double> UOrderedH_INT_DBL;


LabelRank::LabelRank(string inputFileName,int maxT,\
		string outputDir,bool isUseLargestComp,bool isSymmetrize,int numThreads,bool isSyn,\
		int wflag,double win,int pFlag,bool isAddSelfloop,double inflation,bool isSaveicpm,\
		double thr,int majorflag,double mthr,int XTimes,\
		bool isTracing,bool isRecordP) {
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
	org_net=new Net(networkPath,netName,fileName_net);

	isTemporal=false; // need to clean the net when done.
	//---------------------------
	//		GLPA parameters
	//---------------------------
	this->maxT=maxT;

	this->isSyn=isSyn;
	this->isUseLargestComp=isUseLargestComp;
	this->isSymmetrize=isSymmetrize;

	this->isAddSelfloop=isAddSelfloop;
	this->inflation=inflation;
	//---------------------------
	//		more
	//---------------------------
	this->outputDir=outputDir;

	this->numThreads=numThreads;

	//---------------------------
	this->wflag=wflag;
	this->pFlag=pFlag;

	this->win=win;

	this->isSaveicpm=isSaveicpm;

	this->majorflag=majorflag;

	this->thr=thr;

	this->mthr=mthr;
	this->XTimes=XTimes;


	this->isTracing=isTracing;
	this->isRecordP=isRecordP;

	isComputeLastQ=true;      //default

	//start();
}


LabelRank::LabelRank(Net* net,string inputFileName,int maxT,\
		string outputDir,int numThreads,bool isSyn,\
		int wflag,double win,int pFlag,double inflation,bool isSaveicpm,\
		double thr,int majorflag,double mthr,int XTimes,\
		bool isTracing,bool isRecordP,bool isUseLargestComp,bool isSymmetrize,bool isAddSelfloop) {

	cout<<"*change size="<<net->changedNODESLIST.size()<<endl;

	//---------------------------
	//		***temporal****
	//---------------------------
	isTemporal=true;  //this net should be return!

	this->net=net;
	//this->org_net=org_net;

	//---------------------------
	//Extract the fileName
	//---------------------------
	string a,b;
	fileName_net=inputFileName;
	extractFileName_FullPath(inputFileName,netName,a,b);

	networkPath="";

	//---------------------------
	//		GLPA parameters
	//---------------------------
	this->maxT=maxT;
	this->thr=thr;
	this->mthr=mthr;
	this->inflation=inflation;

	this->majorflag=majorflag;
	this->isSyn=isSyn;


	//**isAddSelfloop is important:is used in isMarority()
	this->isAddSelfloop=isAddSelfloop;       //*not use for adding to net
	this->isUseLargestComp=isUseLargestComp; //*not use for loading net
	this->isSymmetrize=isSymmetrize;         //*not use for loading net
	//---------------------------
	//		more
	//---------------------------
	this->outputDir=outputDir;
	this->numThreads=numThreads;

	//---------------------------
	this->wflag=wflag;
	this->pFlag=pFlag;

	this->win=win;
	this->XTimes=XTimes;

	this->isTracing=isTracing;
	this->isRecordP=isRecordP;
	this->isSaveicpm=isSaveicpm;

	isComputeLastQ=true;      //default
}


LabelRank::~LabelRank() {
	if(!isTemporal){
		delete net;
		delete org_net;
	}
}

void LabelRank::init_ALL(){
	//1. initialize the label distribution(histogram)
	//2. w (that would be used to distribute label along each outgoing link)

	//1.init M: weight matrix
	init_W();

	//2. initial P matrix: label distiribution;
	init_P_BCHistgram();


	//3. set flag, alway compute v->newBC for t=1
	init_v_2updateFlag();
}

//========================================
//		***to do
//========================================
void LabelRank::init_temporal(){
	//only this ....

	//1.init M: weight matrix
	init_W_temporal();

	//2. initial P matrix: label distiribution;
	init_P_BCHistgram_temporal();


	//**3. set flag, alway compute v->newBC for t=1?
	init_v_2updateFlag_temporal();
}

void LabelRank::init_v_2updateFlag(){
	//set each node to be update in the next t
	NODE *v;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];
		v->isToUpdate=true; //**
	}
}

void LabelRank::init_v_2updateFlag_temporal(){
	//1. set ONLY changeNODES be update in the next t
	//*2. set other nodes to be false
	NODE *v;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];
		v->isToUpdate=false; //**temporal
	}

	//
	for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
		v=(*sit);  //**
		v->isToUpdate=true; //**temporal
	}
}

void LabelRank::init_P_BCHistgram(){
	//1. initialize the label(ID) distribution(histogram)
	//
	//   for each node
	if(isDebug)
		cout<<"init P matrix(BCHistgram) pFlag="<<pFlag<<"...."<<endl;

	NODE *v,*nbv;
	double sumnbk,r_sumnbk;
	//label is node id: one label
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		v->BCHistgram.clear();

		//------------------
		sumnbk=0.0;r_sumnbk=0.0;
		for(int k=0;k<v->nbList_P.size();k++){
			nbv=v->nbList_P[k];

			sumnbk+=nbv->numNbs;
			r_sumnbk+=1.0/nbv->numNbs;
		}

		//------------------
		for(int k=0;k<v->nbList_P.size();k++){
			nbv=v->nbList_P[k];

			if(pFlag==1){
				//initialized as the 1/v->deg(k)
				v->BCHistgram[nbv->ID]=1.0/v->numNbs; // **nbv->ID==label
			}
			else{
				cout<<"ERROR: No other pflag!!"<<endl;
			}
		}
	}
}

void LabelRank::init_P_BCHistgram_temporal(){
	//1. REinit/initialize the label(ID) distribution(histogram)
	//
	//   for changeNODE only
	cout<<"init P matrix(BCHistgram) pFlag="<<pFlag<<"....: changeNODE only"<<endl;

	NODE *v,*nbv;
	double sumnbk,r_sumnbk;
	//label is node id: one label

	//for(int i=0;i<net->N;i++){
	//	v=net->NODES[i];
	for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
		v=(*sit);  //**

		v->BCHistgram.clear();
		//------------------
		sumnbk=0.0;r_sumnbk=0.0;
		for(int k=0;k<v->nbList_P.size();k++){
			nbv=v->nbList_P[k];

			sumnbk+=nbv->numNbs;
			r_sumnbk+=1.0/nbv->numNbs;
		}

		//------------------
		for(int k=0;k<v->nbList_P.size();k++){
			nbv=v->nbList_P[k];

			if(pFlag==1){
				//initialized as the 1/v->deg(k)
				v->BCHistgram[nbv->ID]=1.0/v->numNbs; // **nbv->ID==label
			}

			else{
				cout<<"ERROR::NO other pflag!!"<<endl;
			}
		}
	}
}

void LabelRank::init_W(){
	//init M: weight matrix
	//w (that would be used to distribute label along each outgoing link)
	if(isDebug)
		cout<<"init W(out going link)...."<<endl;

	NODE *v,*nb;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		//outgoing weight
		if(wflag==1){        //copy, w=1
			v->w=1.0;
		}

		else{cout<<"ERROR::: NO other wflag!!"<<endl;}
		/*
		else if(wflag==2){   //lazy walk, w=1/deg
			//v->w=1.0/v->numNbs;
			v->w=pow(1.0/v->numNbs,win);


			//cout<<"v="<<v->ID<<" w="<<v->w<<" nb="<<v->numNbs<<endl;
		}*/
	}
}

void LabelRank::init_W_temporal(){
	//init M: weight matrix
	//w (that would be used to distribute label along each outgoing link)
	if(isDebug)
		cout<<"Init W(out going link)....: changeNODE only"<<endl;

	NODE *v;
	for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
		v=(*sit);  //**

		//outgoing weight
		if(wflag==1){        //copy, w=1
			v->w=1.0;
		}

		else{cout<<"ERROR::: NO other wflag!!"<<endl;}
	}
}


void LabelRank::start(){
	if(isDebug){
		cout<<"****maxT="<<maxT<<endl;
		cout<<"****wflag="<<wflag<<"("<<win<<")"<<endl;
		cout<<"****pFlag="<<pFlag<<endl;
		cout<<"****majorflag="<<majorflag<<"("<<mthr<<")"<<endl;
		cout<<"****isAddSelfloop="<<isAddSelfloop<<endl;
		cout<<"****XTimes="<<XTimes<<endl;
		cout<<"****isSyn="<<isSyn<<endl;
		cout<<"****r="<<thr<<endl;
		cout<<"****inflation="<<inflation<<endl;
	}

	shortfileName.clear();
	//shortfileName="syn_LabelRank_"+netName+"_w"+ \
	//		int2str(wflag)+"_win"+dbl2str(win)+"_S"+bool2str(isAddSelfloop)+"_P"+int2str(pFlag)\
	//		+"_T"+"_major" +dbl2str(majorflag)\
	//		+"_thr"+dbl2str(thr)+"_I"+dbl2str(inflation)\
	//		+"_mthr"+dbl2str(mthr);

	//change-Jerry for LabelRank_release(Apr. 28, 2013)
	shortfileName="LabelRank_"+netName  \
			+"_thr"+dbl2str(thr)
			+"_I" +dbl2str(inflation)\
			+"_q"+dbl2str(mthr);

	//---------------------------------------
	//	   LOAD network:
	//	(*LabelRank*)-READ IN as it is, Only make it symmetric
	//           Keep the original IDs
	//----------------------------------------
	net->readNetwork_EdgesList(fileName_net,isUseLargestComp,isSymmetrize);

	org_net->readNetwork_EdgesList(fileName_net,isUseLargestComp,isSymmetrize);
	//---------------------------
	//	self-loop
	//---------------------------

	if(isAddSelfloop) net->post_addSelfloop();

	cout<<"INFO: before adding selfloop: N="<<org_net->N<<" M="<<org_net->M<<endl;
	cout<<"INFO: after  adding selfloop: N="<<net->N<<" M="<<net->M<<endl;
	cout<<endl;

	//---------------------------
	//  	game
	//---------------------------
	//initialization P/M and clean network
	init_ALL();
	LabelRank_syn_pointer();
}

//========================================
//		***to do
//========================================
void LabelRank::start_temporal(){
	cout<<"****maxT="<<maxT<<endl;
	cout<<"****isSyn="<<isSyn<<endl;
	cout<<"****wflag="<<wflag<<"("<<win<<")"<<endl;
	cout<<"****pFlag="<<pFlag<<endl;
	cout<<"****P inflation="<<inflation<<endl;
	cout<<"****isAddSelfloop="<<isAddSelfloop<<endl;
	cout<<"****thr="<<thr<<endl;
	cout<<"****majorflag="<<majorflag<<"("<<mthr<<")"<<endl;
	cout<<"****XTimes="<<XTimes<<endl;
	cout<<"****isUseLargestComp="<<isUseLargestComp<<endl;
	cout<<"****isSymmetrize="<<isSymmetrize<<endl;

	shortfileName.clear();
	shortfileName="syn_LabelRank_"+netName+"_w"+ \
			int2str(wflag)+"_win"+dbl2str(win)+"_S"+bool2str(isAddSelfloop)+"_P"+int2str(pFlag)\
			+"_T"+"_major" +dbl2str(majorflag)\
			+"_thr"+dbl2str(thr)+"_I"+dbl2str(inflation)\
			+"_mthr"+dbl2str(mthr);
	//---------------------------
	cout<<"net.N="<<net->N<<" M="<<net->M<<endl;

	//---------------------------
	//  	game
	//---------------------------
	//initialization P/M and clean network
	init_temporal();
	LabelRank_syn_pointer_temporal();

	//
	//if(isSaveicpm){
	//	string fileName=outputDir+shortfileName+".icpm";
	//	saveCPM_noQ_fast(0.5,fileName);
	//}
}


//Auxiliary function:
//  printAllBCHistogram();
//  cout<<"*"<<flush;
//  printSet_PRIMITIVE<int>(v->maxLabels);
void LabelRank::LabelRank_syn_pointer(){
	//three stop critera:
	//  a. matT; b. no change in the network (major); c. XTime>5(major)
	if(isDebug)
		cout<<"Start iteration (SYN):"<<endl;
	NODE *v;
	int t,numchange_t;
	bool cflag_t;
	time_t st=time(NULL);
	double totalBCSize_t;

	tracingTable.clear();
	numChangeTable.clear();   //stop criterion
	bestQ=-10000;                  //best among all T

	//-------------------------------------------------
	//1. speedup1: compute newBC for node with isToUpdate=true and re-check majority at the end

	//cout<<"-------------------init------------------"<<endl;
	//printAllBCHistogram();
	//printAllW();
	//printAllUpdateFlag();


	//-------------------------------------------------
	//*******BUG WARNNING********:be careful, and check if normalized
	//-------------------------------------------------
	//**Make sure BC is normalized at t=0***
	AllNODES_normHistogram_INI_DBL();

	//-------------------------------------------------
	//if not do this, then every node is updated due to init
	//-------------------------------------------------
	AllNODES_CheckMajority_UpdateisToUpdate(cflag_t,numchange_t);
	//if(numchange_t!=15){
	//cout<<"-------------------t=0------------------"<<endl;
	//cout<<"numchange_t="<<numchange_t<<endl;}
	//printAllBCHistogram();
	//printAllW();
	//printAllUpdateFlag();

	for(t=1;t<=maxT;t++){
		//-----------------------
		//1.collect nbv->BChist(t-1) from nbs, accumulate(weighted),
		//  and *normalize* in v->newBCHistgram(t)
		//-----------------------
		for(int i=0;i<net->N;i++){
			v=net->NODES[i];

			//SpeedUp1:
			if(v->isToUpdate)
				v->newBCHistgram=calNewBelongingCoefficients_w(v,t);
		}

		//-----------------------
		////2.keep only labels above thr in *newBC*,
		//  *without* normalize newBCHistgram
		//  this will be copied to BC
		//-----------------------
		//*******BUG********:be careful, and check if normalized
		if(thr>0)
			keepThr_allowTie_apply2newBC_isToUpdate(thr);

		AllNODES_normHistogram_INI_DBL_apply2newBC_isToUpdate();

		//--------------------------------------
		//3.**(SYN) ** Do the delayed update
		//--------------------------------------
		for(int i=0;i<net->N;i++){
			v=net->NODES[i];

			if(v->isToUpdate)
				v->BCHistgram=v->newBCHistgram;
		}

		//make sure that now all v->BCHistgram are still normalized

		//======================================================
		//  4. go through **ALL** nodes and check majority-rule
		//     set the v->isToUpdate
		//	   some nodes may want to update in t+1 due to the
		//	   changes in its nbs.
		//======================================================
		AllNODES_CheckMajority_UpdateisToUpdate(cflag_t,numchange_t);

		//--------------------------------------
		//4. compute Q(t) and save if it is >bestQ
		//--------------------------------------
		if(isTracing) computeQt(t,numchange_t);

		//--------------------------------------
		//5. check stop criteria (total 3)
		//--------------------------------------
		if(majorflag>0){
			if(HistogramIncreasing_U_INT_INT_stopCritirion(numChangeTable,numchange_t,1,XTimes)){
				if(isDebug)
					cout<<"t="<<t<<" ***stop***: cout(numChange) repeats >"<<XTimes<<endl;
				break;
			}

			if(cflag_t==false){//if any change according to Major (i.e.,numchange_t=0)? if not then,do..
				if(isDebug)
					cout<<"t="<<t<<" ***stop***: no change in label"<<endl;
				break;
			}
		}
	}//end of t

	stopT=t;

	more_process(t,numchange_t);
	if(isDebug){
		cout<<endl;	cout<<"Iteration is over (takes "<<difftime(time(NULL),st)<< " seconds)"<<endl;}
}

//========================================
//		***to do
//========================================
void LabelRank::LabelRank_syn_pointer_temporal(){
	//three stop critera:
	//  a. matT; b. no change in the network (major); c. XTime>5(major)

	cout<<"Start iteration (SYN):"<<endl;
	NODE *v;
	int t,numchange_t;
	bool cflag_t;
	time_t st=time(NULL);
	double totalBCSize_t;

	tracingTable.clear();
	numChangeTable.clear();   //stop criterion
	bestQ=-10000;                  //best among all T

	//-------------------------------------------------
	//1. speedup1: compute newBC for node with isToUpdate=true and re-check majority at the end

	if(false){
		cout<<"-------------------init------------------"<<endl;
		printAllBCHistogram();
		printAllW();
		printAllUpdateFlag();}


	//-------------------------------------------------
	//*******BUG WARNNING********:be careful, and check if normalized
	//-------------------------------------------------
	//**Make sure BC is normalized at t=0***

	AllNODES_normHistogram_INI_DBL();


	//************************************************
	//For temporal, we should force it to be done at least once???
	//************************************************
	//if not do this, then every node is updated due to init flag in 2update
	// ***temporal: comment it out to force at least compute once for
	//              these changeNOdes
	//***MUST keep on because we use changeNODES_CheckMajority_UpdateisToUpdate_temporal()

	AllNODES_CheckMajority_UpdateisToUpdate_temporal(cflag_t,numchange_t);
	//cout<<"ATT:::::::::::: it is no commment out.................."<<endl;

	if(false){
		cout<<"-------------------t=0------------------"<<endl;
		printAllBCHistogram();
		printAllW();
		printAllUpdateFlag();
	}



	//========================================
	for(t=1;t<=maxT;t++){
		//cout<<"-------------------t="<<t<<"------------------"<<endl;
		//-----------------------
		//1.collect nbv->BChist(t-1) from nbs, accumulate(weighted),
		//  and *normalize* in v->newBCHistgram(t)
		//-----------------------
		//for(int i=0;i<net->N;i++){
		//	v=net->NODES[i];
		//st=time(NULL);
		for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
			v=(*sit); //***temporal

			//SpeedUp1:
			if(v->isToUpdate)
				v->newBCHistgram=calNewBelongingCoefficients_w(v,t);
		}
		//cout<<endl;	cout<<"for loop (takes "<<difftime(time(NULL),st)<< " seconds)"<<endl;
		//-----------------------
		////2.keep only labels above thr in *newBC*,
		//  *without* normalize newBCHistgram
		//  this will be copied to BC
		//-----------------------
		//*******BUG********:be careful, and check if normalized
		//st=time(NULL);
		if(thr>0)
			keepThr_allowTie_apply2newBC_isToUpdate_temporal(thr);
		//cout<<endl;	cout<<"keepThr_allowTie() (takes "<<difftime(time(NULL),st)<< " seconds)"<<endl;

		//st=time(NULL);
		AllNODES_normHistogram_INI_DBL_apply2newBC_isToUpdate_temporal();
		//cout<<endl;	cout<<"AllNODES_normHistogram_INI() (takes "<<difftime(time(NULL),st)<< " seconds)"<<endl;
		//--------------------------------------
		//3.**(SYN) ** Do the delayed update
		//--------------------------------------
		//for(int i=0;i<net->N;i++){
		//	v=net->NODES[i];
		//st=time(NULL);
		for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
			v=(*sit); //***temporal

			if(v->isToUpdate)
				v->BCHistgram=v->newBCHistgram;
		}
		//cout<<endl;	cout<<"delayed update (takes "<<difftime(time(NULL),st)<< " seconds)"<<endl;
		//make sure that now all v->BCHistgram are still normalized

		//======================================================
		//  4. go through **ALL** nodes and check majority-rule
		//     set the v->isToUpdate
		//	   some nodes may want to update in t+1 due to the
		//	   changes in its nbs.
		//======================================================
		//***
		if(false){
			cout<<"-------------------t="<<t<<"(before)------------------"<<endl;
			printAllBCHistogram();
			printAllW();
			printAllUpdateFlag();}


		//st=time(NULL);
		//AllNODES_CheckMajority_UpdateisToUpdate_temporal(cflag_t,numchange_t);
		changeNODES_CheckMajority_UpdateisToUpdate_temporal(cflag_t,numchange_t);
		//cout<<endl;	cout<<"**changeNODES_CheckMajority() (takes "<<difftime(time(NULL),st)<< " seconds)"<<endl;

		if(false){
			cout<<"-------------------t="<<t<<"(after)------------------"<<endl;
			printAllBCHistogram();
			printAllW();
			printAllUpdateFlag();}


		//--------------------------------------
		//4. compute Q(t) and save if it is >bestQ
		//--------------------------------------

		//--------------------------------------
		//5. check stop criteria
		//--------------------------------------
		//st=time(NULL);
		if(majorflag>0){
			if(HistogramIncreasing_U_INT_INT_stopCritirion(numChangeTable,numchange_t,1,XTimes)){
				cout<<"t="<<t<<" ***stop***: cout(numChange) repeats >"<<XTimes<<endl;
				break;
			}

			if(cflag_t==false){
				cout<<"t="<<t<<" ***stop***: no change in label"<<endl;
				break;
			}
		}
	}//end of t

	stopT=t;

	//more_process(t,numchange_t);
	cout<<endl;	cout<<"Iteration is over (takes "<<difftime(time(NULL),st)<< " seconds)"<<endl;
}


void LabelRank::AllNODES_CheckMajority_UpdateisToUpdate(bool &cflag_t,int &numchange_t){
	NODE *v,*nbv;
	cflag_t=false;      //**set for each t
	numchange_t=0;      //**set for each t
	if(majorflag>0){
		//compute the v->maxLabels
		//  majorflag=1: compare only the labelS with max pro
		//  majorflag=2: compare all labels in v
		//  use only v->BCHistogram
		//net->showVertices_Table();


		getAll_maxLabels_MapList(majorflag);

		//printAllmaxLabels();
	}

	//set the v->isToUpdate
	if(majorflag>0){
		for(int i=0;i<net->N;i++){
			v=net->NODES[i];

			if(!isMajorityAgree(v,true,mthr)){
				//true: exclude myself
				//use only v->maxLabels
				v->isToUpdate=true;

				//cout<<"changing ID="<<v->ID<<endl;
				//printSet_PRIMITIVE<int>(v->maxLabels);
				cflag_t=true;
				numchange_t++;

			}else
				v->isToUpdate=false;
		}
	}

	//(cflag_t=false) == (numchange_t=0)
}

void LabelRank::AllNODES_CheckMajority_UpdateisToUpdate_temporal(bool &cflag_t,int &numchange_t){
	NODE *v,*nbv;
	cflag_t=false;      //**set for each t
	numchange_t=0;      //**set for each t

	//************************************
	//****temporal (should we recompute all nbs)
	//   or compute once in Init_tempor()????????
	//************************************
	if(majorflag>0){
		//compute the v->maxLabels
		//  majorflag=1: compare only the labelS with max pro
		//  majorflag=2: compare all labels in v
		//  use only v->BCHistogram
		//net->showVertices_Table();


		getAll_maxLabels_MapList(majorflag);

		//printAllmaxLabels();
	}

	//set the v->isToUpdate
	if(majorflag>0){
		//for(int i=0;i<net->N;i++){
		//	v=net->NODES[i];
		for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
			v=(*sit); //***temporal

			if(!isMajorityAgree(v,true,mthr)){
				//true: exclude myself
				//use only v->maxLabels
				v->isToUpdate=true;

				//cout<<"changing ID="<<v->ID<<endl;
				//printSet_PRIMITIVE<int>(v->maxLabels);
				cflag_t=true;
				numchange_t++;

			}else
				v->isToUpdate=false;
		}
	}
}

void LabelRank::changeNODES_CheckMajority_UpdateisToUpdate_temporal(bool &cflag_t,int &numchange_t){
	//version: only chagne nodes update the v->maxLabels
	NODE *v,*nbv;
	cflag_t=false;      //**set for each t
	numchange_t=0;      //**set for each t

	//************************************
	//****temporal (should we recompute all nbs)
	//   or compute once in Init_tempor()????????
	//************************************
	if(majorflag>0){
		//compute the v->maxLabels
		//  majorflag=1: compare only the labelS with max pro
		//  majorflag=2: compare all labels in v
		//  use only v->BCHistogram
		//net->showVertices_Table();


		getchangeNODES_maxLabels_MapList(majorflag);

		//printAllmaxLabels();
	}

	//set the v->isToUpdate
	if(majorflag>0){
		//for(int i=0;i<net->N;i++){
		//	v=net->NODES[i];
		for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
			v=(*sit); //***temporal

			if(!isMajorityAgree(v,true,mthr)){
				//true: exclude myself
				//use only v->maxLabels
				v->isToUpdate=true;

				//cout<<"changing ID="<<v->ID<<endl;
				//printSet_PRIMITIVE<int>(v->maxLabels);
				cflag_t=true;
				numchange_t++;

			}else
				v->isToUpdate=false;
		}
	}
}

bool LabelRank::isMajorityAgree(NODE *v,bool isExcludeSelf,double mr){
	//**ONLY compare the c with max pro in each node**
	//do not count it self
	//mr is the threshold
	double count=0;
	NODE *nbv;
	for(int k=0;k<v->nbList_P.size();k++){
		nbv=v->nbList_P[k];
		if(isSubset_PRIMITIVE<int>(nbv->maxLabels,v->maxLabels))
			count++;
	}

	//***
	if(isAddSelfloop && isExcludeSelf) count--;

	//***threshold
	//if(count>=ceil(v->nbList_P.size()*0.5))
	if(count>=ceil(v->nbList_P.size()*mr))
		return true;
	else
		return false;
}

UOrderedH_INT_DBL LabelRank:: calNewBelongingCoefficients_w(NODE* v, int t){
	//**DO NOT update v->WQHistgram** in the function (SYN)
	UOrderedH_INT_DBL newBCHistgram;   //should be EMPTY,could handle self-loop

	//-----------------------------------------------------
	// label propagation: accumulate the weighted BC
	// You may want to differentiate nbs and self

	NODE* nbv;
	for(int k=0;k<v->numNbs;k++){
		nbv=v->nbList_P[k];        //nbs (**+ self**)

		for(UOrderedH_INT_DBL::iterator mit=nbv->BCHistgram.begin();mit!=nbv->BCHistgram.end();mit++){
			int label=mit->first;
			double bc=mit->second;

			if(newBCHistgram.count(label)>0)
				newBCHistgram[label]+=bc * nbv->w ;     //accumulate*w
			else
				newBCHistgram[label]=bc  * nbv->w ;
		}
	}

	//-----------------------------------------------------
	// *****BUG:::should we do the normalization here (before inflation)????
	//       before the testing small example Apr.2, it is not used.
	//-----------------------------------------------------
	//normalize, better than /numNb
	normHistogram_INI_DBL(newBCHistgram);

	//-----------------------------------------------------
	//inflation
	for(UOrderedH_INT_DBL::iterator mit=newBCHistgram.begin();mit!=newBCHistgram.end();mit++){
		mit->second=pow(mit->second,inflation);
	}

	//-----------------------------------------------------
	//cutoff
	if(t%10==0){
		for(UOrderedH_INT_DBL::iterator mit=newBCHistgram.begin();mit!=newBCHistgram.end();mit++){
			if(mit->second<0.0000000001){
				mit->second=0.0;
				cout<<"chopped"<<endl;
			}
		}
	}

	//-----------------------------------------------------
	//normalize, better than /numNb
	normHistogram_INI_DBL(newBCHistgram);

	return newBCHistgram;
}

//--------------------------------------------------
//
//--------------------------------------------------
void LabelRank::saveCPM_noQ_fast_OnlyforThisSnapshop(double thr,string fileName,set<int> &allNodeIDs_forThisSnapshot){
	//version:One output the ids that in this snapshot
	time_t st;

	//CPM: the index is the **commID(0~k-1)**, and vales is node **ID**
	vector<vector<int>* > cpm; //***CPMP, TO REMOVE

	//=========================================
	//1.threshold + createCPM
	//=========================================
	dothreshold_createCPM_pointer_thr(thr,cpm);


	//=========================================
	//****remove node that is not in this snapshot
	//=========================================
	vector<vector<int>* > netcpm; //***CPMP, TO REMOVE
	for(int i=0;i<cpm.size();i++){
		vector<int>* OneCom=cpm[i];

		vector<int>* newcom=new vector<int>;

		for(int j=0;j<OneCom->size();j++){
			int id=(*OneCom)[j];
			//for node *in* this,copy
			if(allNodeIDs_forThisSnapshot.count(id)>0){
				newcom->push_back(id);
			}
		}

		//remove empty com
		if(newcom->size()>0)
			netcpm.push_back(newcom);
		else
			delete newcom;
	}
	//clean old cpm,and replace by the newone
	for(int i=0;i<cpm.size();i++)
		delete cpm[i];
	cpm.clear();
	cpm=netcpm;

	//=========================================
	cout<<"ATTN:no reassign sameLabel disconnected subcomponent"<<endl;
	cout<<"ATTN:NO removeSubset!!!!!" <<endl;

	//---------------------------
	//4. save cpm
	//---------------------------
	st=time(NULL);
	sort_cpm_pointer(cpm);  //sort each com by increasing ID for; and by decrasing community size

	write2txt_CPM_pointer(fileName,cpm);
	int K=cpm.size();
	//---------------------------
	//release memory
	//---------------------------
	for(int i=0;i<cpm.size();i++)
		delete cpm[i];

	cout<<"sorting takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;
}

void LabelRank::dothreshold_createCPM_pointer_thr(double thr,vector<vector<int>* >& cpm){
	//return cpm
	//cout<<"start dothreshold_createCPM_pointer_thr()....."<<endl;
	time_t st=time(NULL);

	//the map of distinct label and community id
	map<int,int> w_comidTable;
	map<int,int>::iterator mit;

	int comid=-1; //**-1, such that we can access via vector[comid]

	NODE *v;
	int ov_cn=0;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		//v->printBCHistogram();
		//v->printBCHistMapEntryList();
		//1.get the world list after thresholding
		vector<int> WS;  //w list that beyond the thrc

		//cout<<"random....."<<endl;
		//thresholding_thr_random(v->BCHistMapEntryList,thr,WS); //***TO IMP
		thresholding_thr_smallestC(v->BCHistMapEntryList,thr,WS); //***TO IMP

		if(WS.size()<1) cout<<"ERROR:empty WS"<<endl;
		if(WS.size()>1) ov_cn++;

		//2. create CPM:put each membership to a community
		for(int j=0;j<WS.size();j++){
			int label=WS[j];

			//------------
			if(w_comidTable.count(label)==0){//not in yet
				comid++;
				w_comidTable.insert(pair<int,int>(label, comid)); //**

				//cpm.push_back(vector<int>());  //copy to the (an empty vector)

				//***CPMPP
				vector<int>* avector=new vector<int>();  //**TO REMOVE
				cpm.push_back(avector);  //copy to the (an empty vector)
			}

			//------------
			mit=w_comidTable.find(label);
			int v_comid=mit->second;

			//cpm[v_comid].push_back(v->ID);  //add one id
			cpm[v_comid]->push_back(v->ID);  //add one id
		}
	}

	//cout<<"Creating CPM takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;
}

void LabelRank::thresholding_thr_smallestC(vector<pair<int,double> >& pairList, double thr, vector<int>& WS){
	//**Version: when tie, select the one with min label value (among these with max pro)
	//For each node, check one-by-one in the ordered map list:
	//For label with pro<=THRESHOULD
	//	we remove it, because all labels could be removed
	//some nodes may become ***unlabeled***.if a node becomes unlabeled,
	//   keep the most frequent label in its list
	//   RETURN: the labels after thresholding (at least one)
	int label;

	//cout<<"------------------thrc="<<thrc<<endl;
	//for(int i=0;i<pairList.size();i++)
	//	cout<<"  "<< pairList[i].first<<" cout="<< pairList[i].second<<endl;

	//*****list MUST BE already ordered in **decreasing count order.****
	double maxv=pairList[0].second; //first one is max count

	int minc=pairList[0].first;
	int ind_minc=0;

	//cout<<"all max("<<maxv<<")"<<"::"<<pairList[0].first;

	if(maxv<=thr){//keep one label to avoid unlabeled node randomly
		// collect the max count
		int cn=1;
		for(int i=1;i<pairList.size();i++){              //start from the **second**
			if(pairList[i].second>=maxv){//multiple max
				cn++;

				//cout<<"pairList["<<i<<"].second"<<pairList[i].second<<" max=("<<maxv<<")"<<endl;
				//cout<<" "<<pairList[i].first;

				//**
				if(pairList[i].first<minc){
					minc=pairList[i].first;
					ind_minc=i;
				}
			}
			else
				break; //stop when first v<maxv
		}

		//**handle the multiple max counts
		//because the label is unique
		label=pairList[ind_minc].first;  //key randInt->[0~n]

		//cout<<" ("<<label<<")"<<endl;

		//add one
		WS.push_back(label);
	}
	else{
		//go down the list until below the thrc
		for(int i=0;i<pairList.size();i++){              //start from the **first**
			if(pairList[i].second>thr){                 //cout**Threshold**
				label=pairList[i].first;				 //key

				//cout<<" ->"<<pairList[i].first;
				WS.push_back(label);
			}
			else
				break;									//stop when first v<thrc
		}
	}
}


void LabelRank::keepThr_allowTie_apply2newBC_isToUpdate(double thr){
	//**Version::apply2newBC && only for v->isToUpdate=true
	//keep only labels that exceed thr (if none,keep all max) at each iteration for each node or not(allow tie)
	//1.convert BCHistogram to *ordered(descending)* MapEntryList
	//2.find the kth value p(k)

	//4.change v->BCHistgram
	//

	//1. convert BCHistogram to *ordered* MapEntryList(***decreasing*** order of value(pro))
	//   i.e., no change to BCHistogram,create a MapEntryList

	//*****Version
	createBCHistogram_MapEntryList_apply2newBC_isToUpdate();


	// 2, 3 and 4 (BCHistgram is updated if necessary)
	NODE *v;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		//*****Version
		if(v->isToUpdate){
			//cout<<"----------before thr="<<thr<<"-------"<<endl;
			//v->printnewBCHistogram();

			vector<int> WS;
			getlabels_thr_allMax(v->newBCHistMapEntryList, thr, WS);

			if(WS.size()<1) cout<<"ERROR::WS size <1........"<<endl;

			UOrderedH_INT_DBL BCHistgram;
			for(int j=0;j<WS.size();j++){
				BCHistgram.insert(*v->newBCHistgram.find(WS[j]));
			}

			v->newBCHistgram=BCHistgram;


			//cout<<"after thr="<<thr<<endl;
			//v->printnewBCHistogram();
		}
	}
}



void LabelRank::keepThr_allowTie_apply2newBC_isToUpdate_temporal(double thr){
	//**Version::apply2newBC && only for v->isToUpdate=true
	//keep only labels that exceed thr (if none,keep all max) at each iteration for each node or not(allow tie)
	//1.convert BCHistogram to *ordered(descending)* MapEntryList
	//2.find the kth value p(k)

	//4.change v->BCHistgram
	//

	//1. convert BCHistogram to *ordered* MapEntryList(***decreasing*** order of value(pro))
	//   i.e., no change to BCHistogram,create a MapEntryList

	//*****Version
	createBCHistogram_MapEntryList_apply2newBC_isToUpdate_temporal();


	// 2, 3 and 4 (BCHistgram is updated if necessary)
	NODE *v;
	//for(int i=0;i<net->N;i++){
	//	v=net->NODES[i];
	for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
		v=(*sit); //***temporal


		//*****Version
		if(v->isToUpdate){
			//cout<<"----------before thr="<<thr<<"-------"<<endl;
			//v->printnewBCHistogram();

			vector<int> WS;
			getlabels_thr_allMax(v->newBCHistMapEntryList, thr, WS);

			if(WS.size()<1) cout<<"ERROR::WS size <1........"<<endl;

			UOrderedH_INT_DBL BCHistgram;
			for(int j=0;j<WS.size();j++){
				BCHistgram.insert(*v->newBCHistgram.find(WS[j]));
			}

			v->newBCHistgram=BCHistgram;


			//cout<<"after thr="<<thr<<endl;
			//v->printnewBCHistogram();
		}
	}
}
//-----------------------------------------------------
//			Histogram
//-----------------------------------------------------

void LabelRank::createBCHistogram_MapEntryList_apply2newBC_isToUpdate_temporal(){
	//**version: apply to newBC && v->isToUpdate=true
	NODE *v;

	//for(int i=0;i<net->N;i++){
	//	v=net->NODES[i];
	for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
		v=(*sit); //***temporal

		if(v->isToUpdate){ //***
			//0.we already have the BCHistogram after iterations
			//1.convert Histogram to a pair list sorted
			//  in the ***decreasing*** order of value(pro).
			v->newBCHistMapEntryList.clear();
			sortMapInt_DBL_unorderMap(v->newBCHistgram, v->newBCHistMapEntryList);

			//*keep v->BCHistgram for the inter-snap soomthing in t+1
		}
	}
}


void LabelRank::createBCHistogram_MapEntryList_apply2newBC_isToUpdate(){
	//**version: apply to newBC && v->isToUpdate=true
	NODE *v;

	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		if(v->isToUpdate){ //***
			//0.we already have the BCHistogram after iterations
			//1.convert Histogram to a pair list sorted
			//  in the ***decreasing*** order of value(pro).
			v->newBCHistMapEntryList.clear();
			sortMapInt_DBL_unorderMap(v->newBCHistgram, v->newBCHistMapEntryList);

			//*keep v->BCHistgram for the inter-snap soomthing in t+1
		}
	}
}

void LabelRank::createBCHistogram_MapEntryList(){
	//*sorted order
	NODE *v;

	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		//0.we already have the BCHistogram after iterations
		//1.convert Histogram to a pair list sorted
		//  in the ***decreasing*** order of value(pro).
		v->BCHistMapEntryList.clear();
		sortMapInt_DBL_unorderMap(v->BCHistgram, v->BCHistMapEntryList);

		//*keep v->BCHistgram for the inter-snap soomthing in t+1
	}

	//cout<<"Progress: Created mapEntryList ......."<<endl;
	//printBCHistMapEntryList();
}

void LabelRank::createBCHistogram_MapEntryList_changeNODES(){
	//version: only changeNODES re-creates MapList
	//*sorted order
	NODE *v;


	//for(int i=0;i<net->N;i++){
	//	v=net->NODES[i];
	for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
		v=(*sit); //***temporal

		//0.we already have the BCHistogram after iterations
		//1.convert Histogram to a pair list sorted
		//  in the ***decreasing*** order of value(pro).
		v->BCHistMapEntryList.clear();
		sortMapInt_DBL_unorderMap(v->BCHistgram, v->BCHistMapEntryList);

		//*keep v->BCHistgram for the inter-snap soomthing in t+1
	}

	//cout<<"Progress: Created mapEntryList ......."<<endl;
	//printBCHistMapEntryList();
}

void LabelRank::printBCHistMapEntryList(){
	NODE *v;
	cout<<"BCHistMapEntryList:"<<endl;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		cout<<"	ID="<<v->ID<<" (k="<<v->numNbs<<"):";
		for(int j=0;j<v->BCHistMapEntryList.size();j++){
			cout<<v->BCHistMapEntryList[j].first<<"("<<v->BCHistMapEntryList[j].second<<") ";
		}
		cout<<endl;
	}
}

void LabelRank::AllNODES_normHistogram_INI_DBL(){
	//apply to BC
	NODE *v;

	//cout<<"net->NODES="<<net->NODES.size()<<endl;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		//v->printBCHistogram();

		//normalize, better than /numNb
		normHistogram_INI_DBL(v->BCHistgram);
	}
}

void LabelRank::AllNODES_normHistogram_INI_DBL_apply2newBC_isToUpdate_temporal(){
	//apply to newBC && v->isToUpdate=true
	NODE *v;

	//for(int i=0;i<net->N;i++){
	//	v=net->NODES[i];
	for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
		v=(*sit); //***temporal

		//**Version:
		if(v->isToUpdate){
			//normalize, better than /numNb
			normHistogram_INI_DBL(v->newBCHistgram);
		}
	}
}


void LabelRank::AllNODES_normHistogram_INI_DBL_apply2newBC_isToUpdate(){
	//apply to newBC && v->isToUpdate=true
	NODE *v;

	for(int i=0;i<net->N;i++){
		v=net->NODES[i];

		//**Version:
		if(v->isToUpdate){
			//normalize, better than /numNb
			normHistogram_INI_DBL(v->newBCHistgram);
		}
	}
}

void LabelRank::normHistogram_INI_DBL(UOrderedH_INT_DBL& hist){
	double sum=0;
	for(UOrderedH_INT_DBL::iterator mit=hist.begin();mit!=hist.end();mit++)
		sum+=mit->second;

	for(UOrderedH_INT_DBL::iterator mit=hist.begin();mit!=hist.end();mit++)
		mit->second=mit->second/sum;
}


bool LabelRank::HistogramIncreasing_U_INT_INT_stopCritirion(UOrderedH_INT_INT &hist,int key,int inc,int threshold){
	//1.hist[key]+=inc, if the first one, then hist[key]=inc
	//2.if the value after increasing over some threshold, return true;
	if(hist.count(key)==0){//not in yet
		hist.insert(pair<int,int>(key,inc));
	}else{
		hist[key]+=inc;  //add one id
	}

	if(hist[key]>=threshold) return true;
	else return false;
}

void LabelRank::getlabels_thr_allMax(vector<pair<int,double> >& pairList, double thr, vector<int>& WS){
	//**Version: when maxv is <thr, all label with max pro are return.
	//For each node, check one-by-one in the ordered map list:
	//For label with pro<=THRESHOULD
	//	we remove it, because all labels could be removed
	//some nodes may become ***unlabeled***.if a node becomes unlabeled,
	//   keep the most frequent label in its list

	//   RETURN: WS the labels after thresholding (at least one)

	//cout<<"------------------thrc="<<thrc<<endl;
	//for(int i=0;i<pairList.size();i++)
	//	cout<<"  "<< pairList[i].first<<" cout="<< pairList[i].second<<endl;

	//*****list MUST BE already ordered in **decreasing count order.****
	double maxv=pairList[0].second; //first one is max count

	//cout<<"all max("<<maxv<<")"<<"::"<<pairList[0].first;

	WS.clear();
	if(maxv<=thr){//keep all labelS with max(pro)
		for(int i=0;i<pairList.size();i++){        //start from the **first**
			if(pairList[i].second>=maxv){
				//add one
				WS.push_back(pairList[i].first);    //label: could be multiple max
			}
			else
				break;                             //stop when first v<maxv
		}
	}
	else{
		//go down the list until below the thrc
		for(int i=0;i<pairList.size();i++){              //start from the **first**
			if(pairList[i].second>thr){                 //cout**Threshold**
				WS.push_back(pairList[i].first);        //label:
			}
			else
				break;									//stop when first v<thrc
		}
	}
}

void LabelRank::getAll_maxLabels_MapList(int flag){
	//allow ties of .lable.
	//****must call post_createBCHistogram_MapEntryList();
	//*****BCHistMapEntryList MUST BE already ordered in **decreasing count order.****


	createBCHistogram_MapEntryList();


	//flag=1:get only all the labels with max pro
	//flag=2:get all labels
	//RETURN: v->maxLabels
	//cout<<"getAll_maxLabels_MapList="<<flag<<endl;


	NODE *v;
	double maxv;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];
		v->maxLabels.clear();

		//v->printBCHistMapEntryList();

		if(flag==1){
			maxv=v->BCHistMapEntryList[0].second; //first one is max count
			//printf("ID=%d maxv=%.20f\n",v->ID,maxv);
			//go down the list until below the maxv
			for(int i=0;i<v->BCHistMapEntryList.size();i++){  //start from the **first**
				//printf("[%d]=%.20f\n",i,v->BCHistMapEntryList[i].second);
				if(v->BCHistMapEntryList[i].second>=maxv){
					v->maxLabels.insert(v->BCHistMapEntryList[i].first); //label
					//cout<<"add c="<<v->BCHistMapEntryList[i].first<<endl;
				}
				else
					break;									//stop when first v<maxv
			}
		}
		else{cout<<"ERROR::no other major flag"<<endl;}
		/*
		else if(flag==2){
			//all labels
			for(int i=0;i<v->BCHistMapEntryList.size();i++){  //start from the **first**
				v->maxLabels.insert(v->BCHistMapEntryList[i].first); //label
			}
		}*/

		//cout<<"ID="<<v->ID<<endl;
		//printSet_PRIMITIVE<int>(v->maxLabels);
	}
}

void LabelRank::getchangeNODES_maxLabels_MapList(int flag){
	//Version: only changeNodes update the v->maxLabels
	//allow ties of .lable.
	//****must call createBCHistogram_MapEntryList_changeNODES();
	//*****BCHistMapEntryList MUST BE already ordered in **decreasing count order.****


	//***only change nodes
	createBCHistogram_MapEntryList_changeNODES();


	//flag=1:get only all the labels with max pro
	//flag=2:get all labels
	//RETURN: v->maxLabels
	//cout<<"getAll_maxLabels_MapList="<<flag<<endl;


	//***only change nodes
	NODE *v;
	double maxv;
	//for(int i=0;i<net->N;i++){
	//	v=net->NODES[i];

	for(set<NODE *>::iterator sit=net->changedNODESLIST.begin();sit!=net->changedNODESLIST.end();sit++){
		v=(*sit); //***temporal

		v->maxLabels.clear();

		//v->printBCHistMapEntryList();

		if(flag==1){
			maxv=v->BCHistMapEntryList[0].second; //first one is max count
			//printf("ID=%d maxv=%.20f\n",v->ID,maxv);
			//go down the list until below the maxv
			for(int i=0;i<v->BCHistMapEntryList.size();i++){  //start from the **first**
				//printf("[%d]=%.20f\n",i,v->BCHistMapEntryList[i].second);
				if(v->BCHistMapEntryList[i].second>=maxv){
					v->maxLabels.insert(v->BCHistMapEntryList[i].first); //label
					//cout<<"add c="<<v->BCHistMapEntryList[i].first<<endl;
				}
				else
					break;									//stop when first v<maxv
			}
		}
		else{cout<<"ERROR::no other major flag"<<endl;}
		/*
		else if(flag==2){
			//all labels
			for(int i=0;i<v->BCHistMapEntryList.size();i++){  //start from the **first**
				v->maxLabels.insert(v->BCHistMapEntryList[i].first); //label
			}
		}*/

		//cout<<"ID="<<v->ID<<endl;
		//printSet_PRIMITIVE<int>(v->maxLabels);
	}
}

//------------------------------------------------
//			cpm
//------------------------------------------------
void LabelRank::write2txt_CPM_pointer(string fileName,vector<vector<int>* >& cpm) {
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


void LabelRank::convert_cpmVectPtr2cpm(vector<vector<int>* >& cpmptr, vector<vector<int> >& cpm){
	cpm.clear();
	for(int i=0;i<cpmptr.size();i++){
		vector<int> a(*cpmptr[i]);

		cpm.push_back(a);
	}
	//cout<<"cpm.size="<<cpm.size()<<endl;
}

//------------------------------------------------
//			write/ print
//------------------------------------------------

void LabelRank::write2txt_traceTable(string fileName) {
	vector<string> data;
	string line;
	for(int i=0;i<tracingTable.size();i++){
		line.clear();
		for(int j=0;j<tracingTable[i].size();j++){
			line+=dbl2str(tracingTable[i][j]);
			line+=" ";
		}

		data.push_back(line);
	}

	//fileOpts.writeToTxt(fileName, false, data);// **false:the first one
	writeToTxt(fileName, false, data);

}


void LabelRank::write2txt_Pt(string fileName) {
	//ONLY for UWUD
	//output the P matrix at time t:P(i,c)
	//output is a sparse matrix: i j pro (becare ful i,j start from 0)
	//but since using topK, it is problematic to use
	// matlab's sparse conver function


	vector<string> data;

	NODE *v;
	for(int i=0;i<net->NODES.size();i++){
		v=net->NODES[i];

		string line;
		for(UOrderedH_INT_DBL::iterator mit=v->BCHistgram.begin();mit!=v->BCHistgram.end();mit++){
			int label=mit->first;  //**assume label==id
			double bc=mit->second;


			line.clear();          //**each (i,j,c) is a line
			line+=int2str(v->ID);  //from id
			line+=" ";
			line+=int2str(label);  //from id
			line+=" ";
			line+=dbl2str(bc);  //from id

			data.push_back(line);
		}
	}

	//fileOpts.writeToTxt(fileName, false, data);// **false:the first one
	writeToTxt(fileName, false, data);
}

void LabelRank::printAllBCHistogram(){
	NODE* v;
	cout<<"CHistogram:"<<endl;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];
		v->printBCHistogram();
	}
}

void LabelRank::printAllW(){
	NODE* v;
	cout<<"-----W------"<<endl;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];
		cout<<"ID="<<v->ID<<" w="<<v->w<<endl;
	}
}
void LabelRank::printAllUpdateFlag(){
	NODE* v;
	cout<<"-----updateFlag------"<<endl;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];
		cout<<"ID="<<v->ID<<" updateFlag="<<v->isToUpdate<<endl;
	}
}

void LabelRank::printAllmaxLabels(){
	NODE *v,*nbv;
	cout<<"-----maxLabels------"<<endl;

	for(int i=0;i<net->N;i++){
		v=net->NODES[i];
		cout<<"ID="<<v->ID<<endl;

		for(set<int>::iterator sit=v->maxLabels.begin();sit!=v->maxLabels.end();sit++){
			cout<<(*sit)<<" ";
		}
		cout<<endl;;
	}
}

//------------------------------------------------
//			org cpm functions
//------------------------------------------------
void LabelRank::sort_cpm(vector<vector<int> >& cpm){
	//inside each com, sort ID by **increasing** order
	for(int i=0;i<cpm.size();i++){
		sort(cpm[i].begin(),cpm[i].end(),sort_INT_INC());
	}

	//each com, sort **decreasing** community size
	sortVecVec_bySize<int>(cpm);
}

//------------------------------------------------
//			backup
//------------------------------------------------
void LabelRank::thresholding_thr_random(vector<pair<int,double> >& pairList, double thr, vector<int>& WS){
	//For each node, check one-by-one in the ordered map list:
	//For label with pro<=THRESHOULD
	//	we remove it, because all labels could be removed
	//some nodes may become ***unlabeled***.if a node becomes unlabeled,
	//   keep the most frequent label in its list
	//   RETURN: the labels after thresholding (at least one)
	int label;

	//cout<<"------------------thrc="<<thrc<<endl;
	//for(int i=0;i<pairList.size();i++)
	//	cout<<"  "<< pairList[i].first<<" cout="<< pairList[i].second<<endl;

	//*****list MUST BE already ordered in **decreasing count order.****

	//--------------
	//****BUGS**************
	// it should be DOUBLE!!!!!!!!!!
	//--------------
	//int maxv=pairList[0].second; //first one is max count
	double maxv=pairList[0].second; //first one is max count


	if(maxv<=thr){//keep one label to avoid unlabeled node randomly
		// collect the max count
		int cn=1;
		for(int i=1;i<pairList.size();i++){              //start from the **second**
			if(pairList[i].second==maxv)//multiple max
				cn++;
			else
				break; //stop when first v<maxv
		}

		// handle the multiple max counts
		if(cn==1)
			label=pairList[0].first;  //key
		else{
			//mtrand2.randInt(n); 0~n
			int wind=mtrand2.randInt(cn-1);
			//cout<<"wind="<<wind<<endl;
			label=pairList[wind].first;  //key randInt->[0~n]
		}

		//add one
		WS.push_back(label);

	}
	else{
		//go down the list until below the thrc
		for(int i=0;i<pairList.size();i++){              //start from the **first**
			if(pairList[i].second>thr){                 //cout**Threshold**
				label=pairList[i].first;				 //key
				WS.push_back(label);
			}
			else
				break;									//stop when first v<thrc
		}
	}
}

//---------------------------------------------------
//-----------------------------------------------
void LabelRank::computeQt(int t,int numchange){
	//compute Q at time t;
	//should not change BC for t+1
	//---------------------------
	//  	threshold and post-processing
	//---------------------------
	// the SLPA routine can apply here by setting thr=0.5
	//a. convert BCHistogram to *ordered* MapEntryList
	createBCHistogram_MapEntryList();

	string fileName;
	//if(isSyn)
	//	fileName=outputDir+"syn_LabelRank_"+netName+"_w"+int2str(wflag)\
	//	+"_win"+dbl2str(win)+"_S"+bool2str(isAddSelfloop)\
	//	+"_P"+int2str(pFlag)\
	//	+"_T"+"_major" +dbl2str(majorflag)\
	//	+"_thr"+dbl2str(thr)+"_I"+dbl2str(inflation)\
	//	+"_mthr"+dbl2str(mthr)+".icpm";

	if(isSyn)
		fileName=outputDir+"LabelRank_"+netName\
		+"_thr"+dbl2str(thr)
		+"_I"+dbl2str(inflation)\
		+"_q"+dbl2str(mthr)+".icpm";

	//compute Q(t) and save if it is >bestQ
	double Q;int K;

	post_threshold_createCPM_pointer_thr_exp(t,0.5,fileName,Q,K);

	//-------------------------------
	double totalBCSize_t=0;
	NODE *v;
	for(int i=0;i<net->N;i++){
		v=net->NODES[i];
		totalBCSize_t+=v->BCHistgram.size();
	}

	//-------------------------------
	//	tracing
	//-------------------------------
	vector<double> trace;
	trace.push_back(t);
	trace.push_back(K);
	trace.push_back(Q);
	trace.push_back(numchange);
	trace.push_back(totalBCSize_t/net->N);
	tracingTable.push_back(trace);

	cout<<"Q("<<t<<")="<<Q<<" K="<<K<<" numChange="<<numchange<<" avgBCSize="<<totalBCSize_t/net->N<<endl;
}


//--------------------------------------------------
//
//--------------------------------------------------
void LabelRank::post_threshold_createCPM_pointer_thr_exp(int t,double thr,string fileName,double &Q,int &K){
	bool isDEBUG=true;
	time_t st;

	//CPM: the index is the **commID(0~k-1)**, and vales is node **ID**
	vector<vector<int>* > cpm; //***CPMP, TO REMOVE

	//=========================================
	//1.threshold + createCPM
	//=========================================
	//new vector<int>(), need to remove
	dothreshold_createCPM_pointer_thr(thr,cpm);

	//=========================================
	//2.***post process*** the communities CPM
	//=========================================
	//a. reassign sameLabel disconnected subcomponent
	//(handle the same label disconnected components problems in general)

	//before label rank, it is disable (not easy to implement a cpm-pointer versin)
	if(false) {
		//if(isDEBUG) cout<<"---before reassign disconnected subcomponent()---"<<endl;
		//if(isDEBUG) printVectVect_PRIMITIVE<int>(cpm);
		//cpm=post_sameLabelDisconnectedComponents(cpm); //**TO IMPROVE

		//in the function: the cpm is deleted and re-assignment
		//*****need to use the original structure*****
		cpm=org_net->pre_findAllConnectedComponents_InOneCluster_CPM_cpmpointer(cpm);


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
	//**working single thread version
	//if(numThreads==0)
	//before laberank, it is open
	//*****do not use network structure*****
	cpm=org_net->post_removeSubset_UorderedHashTable_cpmpointer(cpm);

	//else	// multi threads
	//	cpm=post_removeSubset_UorderedHashTable_cpmpointer_MultiThd(cpm);
	//before laberank, it is open
	//cout<<"removeSubset takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;

	//if(isDEBUG) cout<<"---After---"<<endl;
	//if(isDEBUG) printVectVect_PRIMITIVE<int>(cpm);

	//---------------------------
	//4. save cpm
	//---------------------------
	st=time(NULL);
	sort_cpm_pointer(cpm);  //sort each com by increasing ID for; and by decrasing community size
	//cout<<"sorting takes :" <<difftime(time(NULL),st)<< " seconds."<<endl;

	//---------------------------------
	//		compute Q
	//---------------------------------
	//*****A with selfloop may increase the Q!!!!!!!******
	DisjointM mq;
	vector<vector<int> > cpmIDs;
	convert_cpmVectPtr2cpm(cpm, cpmIDs);

	//**need the org_net***
	Q=mq.calQ_unw_givenCpm(cpmIDs,"",org_net,isUseLargestComp,isSymmetrize);
	//cout<<"Q("<<t<<")="<<Q<<endl;
	if(bestQ<Q){
		bestQ=Q;
		bestT=t;
		bestK=cpm.size();

		if(isSaveicpm) write2txt_CPM_pointer(fileName,cpm);
	}
	if(t==maxT)
		if(isSaveicpm) write2txt_CPM_pointer(fileName,cpm);


	K=cpm.size();

	//---------------------------
	//release memory
	//---------------------------
	for(int i=0;i<cpm.size();i++)
		delete cpm[i];
}

void LabelRank::more_process(int t,int numchange_t){
	//--------------------------------------
	//4. compute final Q
	//--------------------------------------
	if(isComputeLastQ) computeQt(t,numchange_t);

	//--------------------------------------
	// 	save the tracing file
	//--------------------------------------
	//tracing
	if(isTracing){
		string tracefileName="trace/trace_"+shortfileName+".txt";
		write2txt_traceTable(tracefileName);
	}

	//--------------------------------------
	//	 save the P matrix
	//--------------------------------------
	if(isRecordP){
		string fileName_Pt;
		fileName_Pt=outputDir+"Pt_"+shortfileName+"_t"+int2str(t)+".txt";
		write2txt_Pt(fileName_Pt);
	}
}


//------------------------------------------------
//			   Multi-threading
//------------------------------------------------
/*
void LabelRank::decomposeTasks(int numTasks,int numThd,int stInds[],int enInds[]){
	int rem=numTasks%numThd;
	int step=(numTasks-rem)/numThd;  //**TO IMPROVE

	for(int i=0;i<numThd;i++){
		stInds[i]=i*step;
		enInds[i]=(i+1)*step-1;
	}
	enInds[numThd-1]+=rem;

	if(false){
		cout<<"----------------decomposeTasks---------------"<<endl;
		cout<<"rem="<<rem<<" step="<<step<<endl;
		for(int i=0;i<numThd;i++){
			cout<<stInds[i]<<" "<<enInds[i]<<endl;
		}
	}

}

void *LabelRank::removesubset_onethread(void *threadarg){
	// set the corresponding element in indicators
	// and return the pointer

	//We use pointers:
	//***ASSUMING the my_data->cpm do the shallow copy(**pointers**) from the original one
	//   then we can the following

	struct thread_data *my_data;
	my_data = (struct thread_data *) threadarg;

	if(false) cout<<"startind="<<my_data->startind<<" endind="<<my_data->endind<<endl;

	//----------------------
	//references of the **shallow** copy in thread_data
	vector<vector<int>* >& cpm=my_data->cpm;
	vector<UOrderedH_INT_INT* >& vectHTable=my_data->vectHTable;
	int *indicators=my_data->pIndicator;    //*the array name as pointer


	//2.2 find the subset (compare smaller cpmi to largest H(j) first)
	bool issubset;
	//for(int i=cpm.size()-1;i>0;i--){
	//**ONLY in stinds~eninds**
	for(int z=my_data->endind;z>=my_data->startind;z--){
		int i=z;

		//------same as before----
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

			if(issubset){          //remove this cpm
				indicators[i]=0;   //**change i**
				break;
			}
		}
	}

	//----------------------
	//for(int i=my_data->startind;i<=my_data->endind;i++)
	//		my_data->pIndicator[i]=i;

	pthread_exit(NULL);
}



vector<vector<int>* > LabelRank::post_removeSubset_UorderedHashTable_cpmpointer_MultiThd(vector<vector<int>* >& cpm){
	time_t st;
	bool isDEBUG=false;
	cout<<"removeSubset (Multiple threads)............."<<endl;

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

	//===========================================
	int indicators[cpm.size()];
	for(int i=0;i<cpm.size();i++)
		indicators[i]=1;  		  //1-default ok

	int numTasks=cpm.size();
	int numThd=numThreads;       //****

	int stInds[numThd];
	int enInds[numThd];

	decomposeTasks(numTasks, numThd, stInds, enInds);
	//------------------------------------------------
	struct thread_data thread_data_array[numThd];

	pthread_t threads[numThd];    //**
	pthread_attr_t attr;
	void *status;

	// Initialize and set thread detached attribute
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int rc;
	long t;
	for( t=0; t<numThd; t++){
		if(isDEBUG) cout<<"creating thread "<<t<<endl;
		thread_data_array[t].startind=stInds[t];
		thread_data_array[t].endind=enInds[t];
		thread_data_array[t].pIndicator=indicators;   //**TO change in function**
		thread_data_array[t].cpm=cpm;                //**shallow copy**
		thread_data_array[t].vectHTable=vectHTable;   //**shallow copy**

		rc = pthread_create(&threads[t], NULL, removesubset_onethread, (void *) &thread_data_array[t]);
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(-1);
		}
	}

	// Free attribute and wait for the other threads
	pthread_attr_destroy(&attr);

	//**This determines the order
	for(t=numThd-1; t>=0; t--) {
		rc=pthread_join(threads[t], &status);
		if (rc) {
			cout<<"ERROR; return code from pthread_join() is "<<rc<<endl;
			exit(-1);
		}
	}

	//------------------------------------------------
	if(isDEBUG) for(int i=0;i<cpm.size();i++)
		cout<<"indicator["<<i<<"]="<<indicators[i]<<endl;

	//===========================================
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
 */


