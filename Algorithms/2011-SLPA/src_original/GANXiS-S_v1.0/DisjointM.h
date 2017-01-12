/*
 * DisjointM.h
 *
 *  Created on: Nov 16, 2011
 *      Author: Jerry
 */

#ifndef DISJOINTM_H_
#define DISJOINTM_H_

#include <iostream>
#include <string>
#include "fileOpts.h"
#include "Net.h"
#include "NODE.h"
#include <time.h>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <sys/types.h>
#include <unistd.h>

class DisjointM {
public:
	//Net* net;
	//vector<vector<NODE *> > cpm_vp;
	//vector<vector<int> > cpm;

	virtual ~DisjointM();

	void INIT_readcpm_vp(string fileName_cpm,vector<vector<int> >& cpm, vector<vector<NODE *> >& cpm_vp,Net* net);
	void INIT_readcpm(string fileName_cpm,vector<vector<int> >& cpm);
	Net* INIT_readNetwork(string fileName_net,bool isUseLargestComp,bool isSymmetrize);


	void convert2cpm_vp(vector<vector<NODE *> >& cpm_vp,vector<vector<int> >& cpm, Net* net);
	void checkN(vector<vector<int> >& cpm,int N);
	void readcpm(string fileName,vector<vector<int> >& cpm);
	//------------------
	void INIT_ConfusionMatrix(vector<vector<double> >& M,vector<double>& NI,vector<double>& NJ,\
			double& N,int& sizeT,int& sizeF, \
			string fileName_cpmT,string fileName_cpmF);
	void get_confusionMatrix(vector<vector<double> >& M,vector<double>& NI,vector<double>& NJ,double& N,vector<vector<int> >& cpmT,vector<vector<int> >& cpmF);

	double calQ_unw(string fileName_cpm,string fileName_net,Net* net,bool isUseLargestComp,bool isSymmetrize);
	double calNMI(string fileName_cpmT,string fileName_cpmF);
	double calARI(string fileName_cpmT,string fileName_cpmF);


	//the cpm is given
	double calQ_unw_givenCpm(vector<vector<int> >& cpm,string fileName_net,Net* net,bool isUseLargestComp,bool isSymmetrize);
};

#endif /* DISJOINTM_H_ */
