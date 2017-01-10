/*
 * LabelRankT
 *
 */

#ifndef LRTPrama_H_
#define LRTPrama_H_

#include "Net.h"
#include "NODE.h"
#include <map>
#include <vector>
#include <utility>
#include <tr1/unordered_map>


class LRTPrama{
public:
	//----------------------------------------
	//	   LabelRankT parameters:
	//----------------------------------------
	int restart;
	int restart_accStopTime;
	int accStopTime;

	int snapShotMode;
	int changeRange;

	//----------------------------------------
	//	   LabelRankT parameters (default)
	//----------------------------------------
	double thr;
	double inflation;
	double mthr;

	int XTimes;

	int maxT;
	int wflag;
	int pFlag;
	double win;

	bool isSyn;
	bool isAddSelfloop;
	int majorflag;
	int algV;

	//vector<double> THRS;
	//vector<double> MTHRS;
	//vector<double> infList;

	//----------------------------------------
	//			Network parameters (default)
	//	LabelRank: c++ version do not convert to ipair.it reads in the original ID
	//----------------------------------------
	bool isUseLargestComp;
	bool isSymmetrize;

	//----------------------------------------
	//  	 other
	//----------------------------------------
	int numThreads;

	//----------------------------------------
	//	  fileName and output
	//----------------------------------------
	string inputFileName;
	string outputDir;

	bool isTracing;
	bool isSaveicpm;
	bool isRecordP;


	bool isDebug;
	//=======================================
	LRTPrama(){
		//----------------------------------------
		//	   LabelRankT parameters
		//----------------------------------------
		restart=1;                //0-no,1-restart when sum(t')>Threshold,2--restart when sum(maxT-t')>Threshold
		restart_accStopTime=200;  //**[]as long as the consecutive stop time larger then this, restart.
		accStopTime=0;

		snapShotMode=2;           //1-load change Det(G); 2- complete static network
		changeRange=1;            //1-use first nbshood,2-second nbshood

		//----------------------------------------
		//	   LabelRankT parameters (default)
		//----------------------------------------
		thr=0.1;           //**
		inflation=2;       //**
		mthr=0.6;          //**

		//----------------------------------------
		maxT=100;
		wflag=1;           //for M: 2=1/k(lazy walk); 1=1(copy)
		pFlag=1;           //init of p: *1-1/ki, 2-normalized-by_row(M), 3-M (Mcl like), 4-stabtionary (k1/sumk,k2/sumk)
		win=1.0;           //the inflation of w

		isSyn=true;        //**true for LabelRank
		isAddSelfloop=true;
		majorflag=1;
		algV=1;            //1-LabelRank,2-LPA

		XTimes=5;          //stop criterion
		//----------------------------------------
		//			Network parameters (default)
		//	LabelRank: c++ version do not convert to ipair.it reads in the original ID
		//----------------------------------------
		isUseLargestComp=false;   //slow.*LabelRankt:not used!
		isSymmetrize=true;

		//----------------------------------------
		//  	 other
		//----------------------------------------
		numThreads=0;

		//----------------------------------------
		//	  fileName and output
		//----------------------------------------
		inputFileName="";
		outputDir="output//";

		isTracing=false;
		isSaveicpm=false; //save clustering
		isRecordP=false;



        isDebug=true;
	};

};


#endif
