/*
 * rndNumbers.cpp
 *
 *  Created on: Oct 14, 2011
 *      Author: Jierui Xie
 */
//http://www.cs.hmc.edu/~geoff/
#include "rndNumbers.h"

#include <iostream>
#include <ctime> // time()
#include <stdio.h>
#include <stdlib.h>

double rndDblBtw01(){
	// ***YOU need to use this, such that you can get a new
	// one each time!!!!! seed the random number with the system clock

	srand (time(NULL));
	return (double)rand()/RAND_MAX;
}
int rndDblBtw0Nminus1(int N){ //****NOT GOOD ***
	// ***YOU need to use this, such that you can get a new
	// one each time!!!!! seed the random number with the system clock

	srand (time(NULL));
	return rand() % N;
}
