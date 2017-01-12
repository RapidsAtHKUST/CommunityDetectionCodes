/*
 * TieLabel.h
 *
 *  Created on: Nov 12, 2011
 *      Author: Jerry
 */
//---------------------------
//		Tie breaking
//---------------------------

#ifndef TIELABEL_H_
#define TIELABEL_H_

#include "NODE.h"
#include <iostream>
#include <vector>
#include "MersenneTwister.h"

//class TieLabel;



class TieLabel {
public:
	int label;
	vector<NODE*> nbsv;  //nbs nodes with this label

	int measure_INT;       // **this would be any of the measures
	//--------------------
	TieLabel(int c){label=c;}

	//--------------------
	static int tiebreak_sumK(vector<TieLabel>& vect,bool isDEC,MTRand& mtrand1,bool isProportional);


	//--------------------
	static int tiebreaking_rndMax_INT(vector<TieLabel>& vect,bool isDEC,MTRand& mtrand1);
	static int tiebreaking_propotional_INT(vector<TieLabel>& vect);
};

#endif /* TIELABEL_H_ */

