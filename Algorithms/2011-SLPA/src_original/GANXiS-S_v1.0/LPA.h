/*
 * LPA
 *
 *  Created on: Feb 28, 2012
 *      Author: Jerry
 */

#ifndef LPA_H_
#define LPA_H_

#include "Net.h"
#include "NODE.h"
#include <map>
#include <vector>
#include <utility>
#include <tr1/unordered_map>

#include "MersenneTwister.h"
#include "TieLabel.h"

#include "DisjointM.h"
//---------------------------
//		Multi-threading
//---------------------------
typedef std::tr1::unordered_map<int, int> UOrderedH_INT_INT;
typedef std::tr1::unordered_map<int, double> UOrderedH_INT_DBL;

class LPA {
public:
	static bool isDEBUG;

	//---------------------------
	//		network parameters
	//---------------------------
	Net* net;        //could be added with selfloop

	string netName;
	string fileName_net;
	string networkPath;

	bool isUseLargestComp; //***
	bool isSymmetrize;

	//---------------------------
	//		SLPA parameters
	//---------------------------
	vector<double> THRS;      //thr
	bool isSyn;  			  //is synchronous version?
	int maxT;

	bool isSaveicpm;
	//---------------------------
	//		more
	//---------------------------
	string outputDir;

	MTRand mtrand1;
	MTRand mtrand2;

	LPA(string inputFileName,int maxT,\
			string outputDir,bool isUseLargestComp,bool isSymmetrize,bool isSyn,\
			bool isSaveicpm,int run);
	virtual ~LPA();


	double bestQ;
	int    bestT;
	int    bestK; //number of comms


	string shortfileName;

	int run;

	//-------------------------------
	//		LPA
	//-------------------------------

	void start();
	void init();

	void LPA_syn_pointer();
	void LPA_Asyn_pointer();
	void createCPM_pointer(vector<vector<int>* >& cpm);
	void LPA_createCPM_pointer_computeQ(string fileName);


	int ceateHistogram_selRandMax(vector<int>& topLabels,const vector<int>& wordsList,int oldlabel,bool & amIok);
	static void sort_cpm(vector<vector<int> >& cpm);

	void convert_cpmVectPtr2cpm(vector<vector<int>* >& cpmptr, vector<vector<int> >& cpm);
	void write2txt_CPM_pointer(string fileName,vector<vector<int>* >& cpm);

	void printAllLabels();
	void printHistogram_INT_DBL(map<int,double> &hist);

	bool isConverge();
	//---------------------------
	//		temporal
	//---------------------------
	double interSnap_smoothFactor;
	Net *prevNet;   //network in previous snapshot
	int sID;        //current snapshop id (starting from 0)

	void applySmoothing(NODE* v,UOrderedH_INT_DBL& BCHistgram);


	//---------------------------
	//		Multi-threading
	//---------------------------
	int numThreads;
};



#endif /* LPA_H_ */
