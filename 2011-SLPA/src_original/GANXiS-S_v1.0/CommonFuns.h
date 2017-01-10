/*
 * CommonFuns.h
 *
 *  Created on: Oct 14, 2011
 *      Author: Jerry
 */

#ifndef COMMONFUNS_H_
#define COMMONFUNS_H_

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <utility>
#include <string>

#include <ctime> // time()
#include <cmath>
#include <tr1/unordered_set>
#include <tr1/unordered_map>

#include "MersenneTwister.h"

using namespace std;
typedef std::tr1::unordered_map<int, double> UOrderedH_INT_DBL;


struct sort_pair_INT_DBL {
	bool operator()(const std::pair<int,double> &i, const std::pair<int,double> &j) {
		return i.second > j.second;
	}
};//sort_pair_intint_dec;

struct sort_INT_DEC {
	bool operator()(int i, int j) {
		return i > j;
	}
};

struct sort_INT_INC {
	bool operator()(int i, int j) {
		return i < j;
	}
};//sort_int_inc;



void sort_cpm_pointer(vector<vector<int>* >& cpm);
void printVect_INT(vector<int>& vect);
void printVectVect_INT(vector<vector<int> >& cpm);

void printVectVect_INT_pointer(vector<vector<int>* >& cpm_ptr);

//----------------------------------------------------
//				similarity distance
//----------------------------------------------------
double dist_euclidean(vector<double>& A,vector<double>& B);
double dist_cosine(vector<double>& A,vector<double>& B);
double dist_euclidean_sparse_unorderMap(UOrderedH_INT_DBL& A,UOrderedH_INT_DBL& B);
double dist_cosine_sparse_unorderMap(UOrderedH_INT_DBL& A,UOrderedH_INT_DBL& B);

//-----------------
int rndPropotional2Length_INT(vector<int>& input);
double myround(double value);

//-----------------
void sortMapInt_DBL( map<int,double> & words, vector< pair<int,double> >& wordsvec);
void sortMapInt_DBL_unorderMap( UOrderedH_INT_DBL & words, vector< pair<int,double> >& wordsvec);
void createHistogram(map<int,double>& hist, const vector<int>& wordsList);


string int2str(int i);
string dbl2str(double f);
string bool2str(bool flag);

#endif /* COMMONFUNS_H_ */
