/*
 * TieLabel.cpp
 *
 *  Created on: Nov 12, 2011
 *      Author: Jerry
 */

#include "TieLabel.h"
#include "NODE.h"
#include <iostream>
#include <vector>
#include <algorithm>

#include "CommonFuns.h"

struct sort_TieLabel_by_measureINT_DEC {
	bool operator()(TieLabel i, TieLabel j) {
		return i.measure_INT > j.measure_INT ;
	}
};
struct sort_TieLabel_by_measureINT_INC {
	bool operator()(TieLabel i, TieLabel j) {
		return i.measure_INT < j.measure_INT ;
	}
};

int TieLabel::tiebreak_sumK(vector<TieLabel>& vect,bool isDEC,MTRand& mtrand1,bool isProportional){
	//1.cal the total number of degree and **update the .measure
	int label;

	TieLabel* tl;
	for(int i=0;i<vect.size();i++){
		tl=&vect[i];

		int sumK=0;
		for(int k=0;k<tl->nbsv.size();k++){
			sumK+=(tl->nbsv[k])->numNbs;
		}

		//*Update(COMM)
		tl->measure_INT=sumK;
	}

	//2.rand select one from topX (COMM)
	if(isProportional)
		label=tiebreaking_propotional_INT(vect);
	else
		label=tiebreaking_rndMax_INT(vect, isDEC,mtrand1);

	return  label;
}


//------------------------------------------
//2. randomly select label from the topX nodes.
//------------------------------------------
int TieLabel::tiebreaking_rndMax_INT(vector<TieLabel>& vect,bool isDEC,MTRand& mtrand1){
	int label;

	//1. sort in the desired order(DEC)
	if(isDEC)
		sort(vect.begin(),vect.end(),sort_TieLabel_by_measureINT_DEC());
	else
		sort(vect.begin(),vect.end(),sort_TieLabel_by_measureINT_INC());


	//cout<<"sorted Measure(label):"<<endl;
	//for(int i=0;i<vect.size();i++){
	//		cout<<vect[i].measure<<"("<<vect[i].label<<")"<<endl;
	//}


	//2. cal the "X" of these tied top nodes
	int cn=1;

	for(int i=1;i<vect.size();i++){          //start from the **second**
		if(vect[i].measure_INT==vect[0].measure_INT)       //multiple max
			cn++;
		else
			break; //stop when first v<maxv
	}

	//3. randly pick one from these topX nodes
	if(cn==1)
		label=vect[0].label;         //key-label
	else{
		int wind=mtrand1.randInt(cn-1); //**[0~n]
		label=vect[wind].label;
	}

	return label;
}
//------------------------------------------
//2. randomly select label proportional to the measures
//------------------------------------------
int TieLabel::tiebreaking_propotional_INT(vector<TieLabel>& vect){
	int label;

	vector<int> input;
	for(int i=0;i<vect.size();i++)
		input.push_back(vect[i].measure_INT);

	int wind=rndPropotional2Length_INT(input);			//**
	label=vect[wind].label;


	return label;
}

