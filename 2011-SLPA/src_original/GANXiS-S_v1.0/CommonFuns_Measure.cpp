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

#include "CommonFuns_Measure.h"
#include "CommonFuns_TMP_Measure.h"

using namespace std;

//-------------------------------------
// see SLPA and comTemp...
void sort_cpm_vectINT(vector<vector<int> >& cpm){
	//inside each com, sort ID by **increasing** order
	for(int i=0;i<cpm.size();i++){
		sort(cpm[i].begin(),cpm[i].end(),sort_INT_INC());
	}

	//each com, sort **decreasing** community size
	sortVecVec_bySize<int>(cpm);
}
//-------------------------------------

void sortMapInt_Int(map<int,int> & words, vector<pair<int,int> > & wordsvec){
	//Use the map to create a sorted pair vector
	//-------------------------------------
	//  map->vector->sort pair
	//-------------------------------------
	for ( map<int,int>::iterator it=words.begin() ; it != words.end(); it++ ){
		wordsvec.push_back(*it);//**
	}
	//-------------------
	sort (wordsvec.begin(), wordsvec.begin()+wordsvec.size(), sort_pair_INT_INT());
}

//----------------------------------------
/*DU double myround( double value ){
	return floor( value + 0.5 );
}*/

void createHistogram(map<int,int>& hist, const vector<int>& wordsList){
	//create histogram hist from a wordsList
	hist.clear();
	map<int,int>::iterator mit;

	for(int i=0;i<wordsList.size();i++){
		int key=wordsList[i];

		if(hist.count(key)>0){
			//increasing the count
			mit=hist.find(key);
			int count=mit->second+1; //count

			//reinsert into the hist
			hist.erase (mit);		//**insert() do not change the value
			hist.insert(pair<int,int>(key,count));
		}
		else{
			//set the count to 1
			hist.insert(pair<int,int>(key,1));
		}
	}

}

/*DU
string int2str(int i){
	string s;
	stringstream out;
	out << i;
	s = out.str();

	return s;
}*/

/*DU
string dbl2str(double f){
	string s;
	stringstream out;
	out << f;
	s = out.str();

	return s;
}*/

double nchoosek(double n,double k){
	// need to use the Multiplicative formula to avoid the overflow
	//http://en.wikipedia.org/wiki/Binomial_coefficient

	double res=1.0;
	//if(n==0) res=0;
	//if(k==0) res=1;

	for(int i=1;i<=k;i++)
		res*=(n-(k-i))/i;

	return res;
}




