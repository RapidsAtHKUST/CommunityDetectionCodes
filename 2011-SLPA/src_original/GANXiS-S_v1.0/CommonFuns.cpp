//============================================================================
// Name        : CommonFuns.cpp
// Author      : Jierui Xie (xiej2@rpi.edu)
// Date        : Oct. 2011
// Version     :
// Copyright   : All rights reserved.
// Description : SLPA algorithm for community detection.
// Web Site    : https://sites.google.com/site/communitydetectionslpa/
// Publication:
//             J. Xie, B. K. Szymanski and X. Liu, "SLPA: Uncovering Overlapping Communities in Social Networks via A Speaker-listener Interaction Dynamic Process", IEEE ICDM workshop on DMCCI 2011, Vancouver, CA.
//============================================================================

#include "CommonFuns.h"

#include "MersenneTwister.h"

#include "CommonFuns_TMP.h"

using namespace std;


//----------------------------------------

void printVect_INT(vector<int>& vect){
	for(int i=0;i<vect.size();i++)
		cout<<vect[i]<<" ";
	cout<<endl;
}

void printVectVect_INT(vector<vector<int> >& cpm){
	for(int i=0;i<cpm.size();i++){
		//sort(cpm[i].begin(),cpm[i].end(),sort_INT_INC());

		cout<<"Vect"+int2str(i)<<":";
		printVect_INT(cpm[i]);
	}
}

void printVectVect_INT_pointer(vector<vector<int>* >& cpm_ptr){
	for(int i=0;i<cpm_ptr.size();i++){
		//sort(cpm[i].begin(),cpm[i].end(),sort_INT_INC());

		cout<<"Vect"+int2str(i)<<":";
		printVect_INT(*cpm_ptr[i]);
	}
}

void sort_cpm_pointer(vector<vector<int>* >& cpm){
	//**sort BOTH inside(node id) and among comms(comm size)!**
	//inside each com, sort ID by **increasing** order
	for(int i=0;i<cpm.size();i++){
		//sort(cpm[i].begin(),cpm[i].end(),sort_INT_INC());
		sort(cpm[i]->begin(),cpm[i]->end(),sort_INT_INC());
	}

	//each com, sort **decreasing** community size
	sortVecVec_bySize_pointer<int>(cpm);
}
//----------------------------------------------------
//				similarity distance
//----------------------------------------------------

double dist_euclidean(vector<double>& A,vector<double>& B){
	//Check the length first.
	if(A.size()!=B.size()){
		cout<<"sizes are not the same"<<endl;
		exit(1);
	}

	double res=0.0;
	for(int i=0;i<A.size();i++){
		res+=pow(A[i]-B[i],2);
	}

	return sqrt(res);
}

double dist_cosine(vector<double>& A,vector<double>& B){
	//Check the length first.
	if(A.size()!=B.size()){
		cout<<"sizes are not the same"<<endl;
		exit(1);
	}

	double resAB=0.0;
	double resAA=0.0;
	double resBB=0.0;
	for(int i=0;i<A.size();i++){
		resAB+=A[i]*B[i];
		resAA+=A[i]*A[i];
		resBB+=B[i]*B[i];
	}

	return 1.0-resAB/(sqrt(resAA)*sqrt(resBB));
}

/*
double dist_euclidean_sparse(map<int,double>& A,map<int,double>& B){
	//No need to check the length, A and B are of variable length
	double res=0.0;
	for(map<int,double>::iterator mit=A.begin();mit!=A.end();mit++){
		if(B.count(mit->first)>0)    //in both A and B
			res+=pow(mit->second - B[mit->first],2);
		else
			res+=pow(mit->second - 0.0,2);      //not found in B
	}
	for(map<int,double>::iterator mit=B.begin();mit!=B.end();mit++){
		if(A.count(mit->first)==0)           //not found in A
			res+=pow(mit->second - 0.0,2);
	}

	return sqrt(res);
}
double dist_cosine_sparse(map<int,double>& A,map<int,double>& B){
	//No need to check the length, A and B are of variable length

	double resAB=0.0;
	double resAA=0.0;
	double resBB=0.0;

	for(map<int,double>::iterator mit=A.begin();mit!=A.end();mit++){
		if(B.count(mit->first)>0){    //in both A and B
			resAB+=mit->second * B[mit->first];
			resAA+=mit->second * mit->second;
			resBB+=B[mit->first] * B[mit->first];
		}
		else{//not found in B
			resAB+=mit->second * 0;
			resAA+=mit->second * mit->second;
			resBB+=0 * 0;
		}
	}
	for(map<int,double>::iterator mit=B.begin();mit!=B.end();mit++){
		if(A.count(mit->first)==0){           //not found in A
			//res+=pow(mit->second - 0.0,2);
			resAB+=mit->second * 0;
			resAA+=0 * 0;
			resBB+=mit->second * mit->second;
		}
	}

	return 1.0-resAB/(sqrt(resAA)*sqrt(resBB));
}
 */

double dist_euclidean_sparse_unorderMap(UOrderedH_INT_DBL& A,UOrderedH_INT_DBL& B){
	//No need to check the length, A and B are of variable length
	double res=0.0;
	for(UOrderedH_INT_DBL::iterator mit=A.begin();mit!=A.end();mit++){
		if(B.count(mit->first)>0)    //in both A and B
			res+=pow(mit->second - B[mit->first],2);
		else
			res+=pow(mit->second - 0.0,2);      //not found in B
	}
	for(UOrderedH_INT_DBL::iterator mit=B.begin();mit!=B.end();mit++){
		if(A.count(mit->first)==0)           //not found in A
			res+=pow(mit->second - 0.0,2);
	}

	return sqrt(res);
}
double dist_cosine_sparse_unorderMap(UOrderedH_INT_DBL& A,UOrderedH_INT_DBL& B){
	//No need to check the length, A and B are of variable length

	double resAB=0.0;
	double resAA=0.0;
	double resBB=0.0;

	for(UOrderedH_INT_DBL::iterator mit=A.begin();mit!=A.end();mit++){
		if(B.count(mit->first)>0){    //in both A and B
			resAB+=mit->second * B[mit->first];
			resAA+=mit->second * mit->second;
			resBB+=B[mit->first] * B[mit->first];
		}
		else{//not found in B
			resAB+=mit->second * 0;
			resAA+=mit->second * mit->second;
			resBB+=0 * 0;
		}
	}
	for(UOrderedH_INT_DBL::iterator mit=B.begin();mit!=B.end();mit++){
		if(A.count(mit->first)==0){           //not found in A
			//res+=pow(mit->second - 0.0,2);
			resAB+=mit->second * 0;
			resAA+=0 * 0;
			resBB+=mit->second * mit->second;
		}
	}

	return 1.0-resAB/(sqrt(resAA)*sqrt(resBB));
}

//----------------------------------------------------



int rndPropotional2Length_INT(vector<int>& input){
	//assuming all int is **possitive>0**
	// return the *index* in the input vector
	int index;

	vector<int> culsums; //accummulated sum

	//cout<<"input:";
	//for(int i=0;i<input.size();i++)
	//	cout<<input[i]<<" ";
	//cout<<endl;
	//------------------
	culsums.push_back(input[0]);
	for(int i=1;i<input.size();i++) //**start from1
		culsums.push_back(culsums[i-1]+input[i]);

	//cout<<"culsums:";
	//for(int i=0;i<culsums.size();i++)
	//	cout<<culsums[i]<<" ";
	//cout<<endl;

	//------------------
	MTRand mtrand1;
	int total=culsums.back();

	int rndint=mtrand1.randInt(total); //**[0~n]

	for(int i=0;i<culsums.size();i++)
		if(rndint<=culsums[i]){
			index=i;
			break;
		}

	//cout<<"rndint="<<rndint<<" ind="<<index<<endl;

	return index;
}


//-------------------------------------

void sortMapInt_DBL(map<int,double> & words, vector<pair<int,double> > & wordsvec){
	//Use the map to create a sorted pair vector
	//-------------------------------------
	//  map->vector->sort pair
	//-------------------------------------
	for ( map<int,double>::iterator it=words.begin() ; it != words.end(); it++ ){
		wordsvec.push_back(*it);//**
	}
	//-------------------
	sort (wordsvec.begin(), wordsvec.begin()+wordsvec.size(), sort_pair_INT_DBL());
}

void sortMapInt_DBL_unorderMap( UOrderedH_INT_DBL & words, vector<pair<int,double> > & wordsvec){
	//Use the map to create a sorted pair vector
	//-------------------------------------
	//  map->vector->sort pair
	//-------------------------------------
	for (UOrderedH_INT_DBL::iterator it=words.begin() ; it != words.end(); it++ ){
		wordsvec.push_back(*it);//**
	}
	//-------------------
	sort (wordsvec.begin(), wordsvec.begin()+wordsvec.size(), sort_pair_INT_DBL());
}


//----------------------------------------
double myround( double value ){
	return floor( value + 0.5 );
}

void createHistogram(map<int,double>& hist, const vector<int>& wordsList){
	//create histogram hist from a wordsList
	hist.clear();
	map<int,double>::iterator mit;

	for(int i=0;i<wordsList.size();i++){
		int key=wordsList[i];

		if(hist.count(key)>0){
			//increasing the count
			mit=hist.find(key);
			double count=mit->second+1.0; //count

			//reinsert into the hist
			hist.erase (mit);		//**insert() do not change the value
			hist.insert(pair<int,double>(key,count));
		}
		else{
			//set the count to 1.0
			hist.insert(pair<int,double>(key,1.0));
		}
	}

}

string int2str(int i){
	string s;
	stringstream out;
	out << i;
	s = out.str();

	return s;
}

string dbl2str(double f){
	string s;
	stringstream out;
	out << f;
	s = out.str();

	return s;
}


string bool2str(bool flag){
	if(flag)
		return "1";
	else
		return "0";
}

