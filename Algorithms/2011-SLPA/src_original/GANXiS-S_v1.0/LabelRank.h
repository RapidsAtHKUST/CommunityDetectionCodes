/*
 * LabelRank.h
 *
 *  Created on: Nov 15, 2011
 *      Author: Jerry
 */

#ifndef LabelRank_H_
#define LabelRank_H_

#include "Net.h"
#include "NODE.h"
#include <map>
#include <vector>
#include <utility>
#include <tr1/unordered_map>

#include "MersenneTwister.h"
#include "DisjointM.h"


typedef std::tr1::unordered_map<int, int> UOrderedH_INT_INT;
typedef std::tr1::unordered_map<int, double> UOrderedH_INT_DBL;

//---------------------------
//		Multi-threading
//---------------------------
struct thread_data{
	int  startind;
	int  endind;

	int *pIndicator;

	//expect to do shallow copy of the pointers
	vector<vector<int>* > cpm;
	vector<UOrderedH_INT_INT* > vectHTable;
};


class LabelRank {
public:
	//---------------------------
	//		network parameters
	//---------------------------
	Net* net;        //could be added with selfloop
	Net* org_net;    //original

	string netName,fileName_net,networkPath;

	bool isUseLargestComp; //***
	bool isSymmetrize;
	bool isAddSelfloop;
	//---------------------------
	//		LabelRank parameters
	//---------------------------
	bool isSyn;  			  //is synchronous version?
	int maxT;

	int wflag;
	int pFlag;

	double win;              //the inflation on w
	double inflation;        //P inflation

	bool isSaveicpm;

	vector<double> THRS;      //thr
	double thr;

	int majorflag;
	double mthr;        //majority threshold
	int XTimes;
	UOrderedH_INT_INT numChangeTable; //stop criterium
	//---------------------------
	//		more
	//---------------------------
	string shortfileName;
	string outputDir;

	MTRand mtrand1,mtrand2;


	bool isRecordP;
	//---------------------------
	//		Q
	//---------------------------
	vector<vector<double> > tracingTable;//(t,k,Q)
	double bestQ;
	int    bestT;
	int    bestK;       //number of comms

	int    stopT;       //the time when itereation terminates

	bool isTracing;
	bool isComputeLastQ;


	bool isDebug;

	LabelRank(string inputFileName,int maxT,\
			string outputDir,bool isUseLargestComp,bool isSymmetrize,int numThreads,bool isSyn,\
			int wflag,double win,int pFlag,bool isAddSelfloop,\
			double inflation,bool isSaveicpm,double thr,\
			int majorflag,double mthr,int XTimes,\
			bool isTracing,bool isRecordP);
	virtual ~LabelRank();


	//-------------------------------
	//		LabelRank
	//-------------------------------
	void start();
	void init_ALL();
	void init_P_BCHistgram();
	void init_W();
	void init_v_2updateFlag();

	void LabelRank_syn_pointer();
	void more_process(int t,int numchange_t);


	//void keepThr_allowTie(double thr);
	//void keepThr_allowTie_apply2newBC(double thr);
	void keepThr_allowTie_apply2newBC_isToUpdate(double thr);

	void getlabels_thr_allMax(vector<pair<int,double> >& pairList, double thr, vector<int>& WS);

	bool isMajorityAgree(NODE *v,bool isExcludeSelf,double mr);
	void getAll_maxLabels_MapList(int flag);


	void AllNODES_CheckMajority_UpdateisToUpdate(bool &cflag_t,int &numchange_t);
	//-------------------------------
	//		Histogram
	//-------------------------------
	UOrderedH_INT_DBL calNewBelongingCoefficients_w(NODE* v,int t);
	void normHistogram_INI_DBL(UOrderedH_INT_DBL& hist);

	//void AllNODES_normHistogram_INI_DBL_apply2newBC();
	void AllNODES_normHistogram_INI_DBL_apply2newBC_isToUpdate();

	void AllNODES_normHistogram_INI_DBL();
	bool HistogramIncreasing_U_INT_INT_stopCritirion(UOrderedH_INT_INT &hist,int key,int inc,int threshold);

	void createBCHistogram_MapEntryList();
	//void createBCHistogram_MapEntryList_apply2newBC();
	void createBCHistogram_MapEntryList_apply2newBC_isToUpdate();

	void printBCHistMapEntryList();
	void printAllBCHistogram();
	//-------------------------------
	//		post processing
	//-------------------------------
	void dothreshold_createCPM_pointer_thr(double thr,vector<vector<int>* >& cpm);
	void post_threshold_createCPM_pointer_thr(double thr,string fileName);
	void post_threshold_createCPM_pointer_thr_exp(int t,double thr,string fileName,double &Q,int &K);
	void thresholding_thr_smallestC(vector<pair<int,double> >& pairList, double thr, vector<int>& WS);

	void thresholding_thr_random(vector<pair<int,double> >& pairList, double thr, vector<int>& WS);
	static void sort_cpm(vector<vector<int> >& cpm);


	void computeQt(int t,int numchange);
	void computeQ(double &Q,int &K);
	//void computeQ_orgNet(double &Q,int &K);

	void convert_cpmVectPtr2cpm(vector<vector<int>* >& cpmptr, vector<vector<int> >& cpm);
	void write2txt_CPM_pointer(string fileName,vector<vector<int>* >& cpm);


	void write2txt_Pt(string fileName);
	void write2txt_traceTable(string fileName);

	void printAllW();
	void printAllUpdateFlag();
	void printAllmaxLabels();
	//---------------------------
	//		temporal
	//---------------------------
	//double interSnap_smoothFactor;
	//Net *prevNet;   //network in previous snapshot
	//int sID;        //current snapshop id (starting from 0)

	//void applySmoothing(NODE* v,UOrderedH_INT_DBL& BCHistgram);
	bool isTemporal;

	LabelRank(Net* net,string inputFileName,int maxT,\
			string outputDir,int numThreads,bool isSyn,\
			int wflag,double win,int pFlag,double inflation,bool isSaveicpm,\
			double thr,int majorflag,double mthr,int XTimes,\
			bool isTracing,bool isRecordP,bool isUseLargestComp,bool isSymmetrize,bool isAddSelfloop);

	void init_temporal();
	void init_v_2updateFlag_temporal();
	void init_P_BCHistgram_temporal();
	void init_W_temporal();
	void start_temporal();

	void LabelRank_syn_pointer_temporal();


	void AllNODES_CheckMajority_UpdateisToUpdate_temporal(bool &cflag_t,int &numchange_t);
	void keepThr_allowTie_apply2newBC_isToUpdate_temporal(double thr);
	void createBCHistogram_MapEntryList_apply2newBC_isToUpdate_temporal();

	void AllNODES_normHistogram_INI_DBL_apply2newBC_isToUpdate_temporal();

	void changeNODES_CheckMajority_UpdateisToUpdate_temporal(bool &cflag_t,int &numchange_t);	//---------------------------
	void getchangeNODES_maxLabels_MapList(int flag);
	void createBCHistogram_MapEntryList_changeNODES();

	//------------------------------------------
	//no Q, no org_net
	void saveCPM_noQ_fast_OnlyforThisSnapshop(double thr,string fileName,set<int> &allNodeIDs_forThisSnapshot);



	//		Multi-threading
	//---------------------------
	int numThreads;

	void decomposeTasks(int numTasks,int numThd,int stInds[],int enInds[]);
	static void *removesubset_onethread(void *threadarg);




};



#endif /* LabelRank_H_ */
