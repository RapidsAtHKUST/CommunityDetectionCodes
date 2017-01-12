//============================================================================
// Author      : Jierui Xie (jierui.xie@gmail.com)
//             : Boleslaw K. Szymanski  (szymab@rpi.edu)
// Date        : Apr. 28th, 2013
// Version     : v1.0 (**Unweighted, Undirected**)
// Copyright   : All rights reserved.
// Description : LabelRank algorithm for stabilized community detection.
// Web Site    : https://sites.google.com/site/communitydetectionLabelRank/
//Publication:
//             J. Xie, B. K. Szymanski, "LabelRank: A Stabilized Label Propagation Algorithm for Community Detection in Networks", IEEE NSW 2013, West point, NY
//============================================================================
//First attempt to stabilize the LabelRank

#include <iostream>
#include <string>
#include "fileOpts.h"
#include "Net.h"
#include "NODE.h"
#include "LabelRank.h"
#include <time.h>
#include <vector>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "CommonFuns.h"

#include "DisjointM.h"
#include "LPA.h"
#include "LRTPrama.h"

using namespace std;

//To compile: make
//To run:
//  ./LabelRank netName cutoff_r inflation_in NBDisimilarity_q
//  ./LabelRank test.ipairs 0.1 4 0.7

int main(int argc, char* argv[]) {
	//cout<<"Process id="<<getpid()<<endl;
	time_t st=time(NULL);

	LabelRank* LRank;

	if(argc<5) {
		cout<<"Incorrect command!"<<endl;
		cout<<"Usage:"<<endl;
		cout<<"       ./LabelRank netName cutoff_r inflation_in NBDisimilarity_q"<<endl;
		cout<<" e.g., ./LabelRank test.ipairs 0.1 2 0.6"<<endl;
		cout<<" "<<endl;
		cout<<" "<<endl;
		exit(1);
	}

	string fileName_net=argv[1];

	//----------------------------------
	//initialize default parameters
	//----------------------------------
	LRTPrama PP;

	PP.thr=atof(argv[2]);        //param: r (segment fault if not enter)
	PP.inflation=atof(argv[3]);  //param: in
	PP.mthr=atof(argv[4]);       //param: q

	PP.isSaveicpm=true;          //to output communities file and compute Q
	if(argc==6){
		if(atoi(argv[5])==1)
			PP.isSymmetrize = false;
	}

	//----------------------------------
	//initialize default parameters
	//----------------------------------
	if(fileName_net.size()==0 || !isFileExist(fileName_net.c_str())){
		cout<<"ERROR: input network "+fileName_net+" not found!"<<endl;
		exit(1);
	}else{
		time_t st=time(NULL);

		LRank=new LabelRank(fileName_net,PP.maxT,PP.outputDir,\
				PP.isUseLargestComp,PP.isSymmetrize,PP.numThreads,PP.isSyn,\
				PP.wflag,PP.win,PP.pFlag,PP.isAddSelfloop,PP.inflation,PP.isSaveicpm,\
				PP.thr,PP.majorflag,PP.mthr,PP.XTimes,PP.isTracing,PP.isRecordP);

		LRank->isDebug=false;     //disable displaying debug info
		LRank->net->isDebug=false;
		LRank->start();
	}

	cout<<"\nINFO: Running Time is :" <<difftime(time(NULL),st)<< " seconds."<<endl;
	cout<<"INFO: Check the output directory for your detection results."<<endl;
}

