/*
 * ComputeSnapshotChange.h
 *
 *  Created on: Mar 23, 2012
 *      Author: Jerry
 */

#ifndef COMPUTESNAPSHOTCHANGE_H_
#define COMPUTESNAPSHOTCHANGE_H_

#include <iostream>
#include <map>
#include <set>
#include <vector>
#include <fstream>
#include <sstream>
//#include <math.h>   //sqrt()

#include "CommonFuns.h"
#include "fileOpts.h"
#include "NODE.h"
#include <tr1/unordered_set>

#include "CommonFuns_TMP.h"

using namespace std;

//typedef std::tr1::unordered_set<string> UOrderedSet_S;
typedef std::tr1::unordered_set<int> UOrderedSet_INT;

class ComputeSnapshotChange {


public:
	set<string> EdgesSet1; //edges list at t-1
	set<string> EdgesSet2; //edges list at t
	set<string> EdgesSet12; //Union(E1,E2)

	set<string> de1;       //add:E(t)-E(t-1)
	set<string> de2;       //del:E(t-1)-E(t)

	UOrderedSet_INT ds1;    //add: node ids involved in an addition of an edges
	UOrderedSet_INT ds2;    //del: node ids involved in a deletion of an edges
	UOrderedSet_INT DSet;   // Union(ds1,ds2)

	UOrderedSet_INT SN1;    //all nodes in t-1
	UOrderedSet_INT SN2;    //all nodes in t
	UOrderedSet_INT SN12;    //Union(SN1,SN2)


	ComputeSnapshotChange(string fileName1,string fileName2);
	void read_snapshot_2set(string fileName,set<string>& EdgesSet);
	void setdiff_S(set<string>& EdgesSet1,set<string>& EdgesSet2,set<string>& dff);
	void setdiff_Int(set<int> &S,set<int>& S2,set<int>& dff);
	UOrderedSet_INT mySet_Union_int(UOrderedSet_INT &st1,UOrderedSet_INT &st2);
	void printSetS(set<string> S);
	void printSetInt(UOrderedSet_INT S);
	void parseNodeIDs_fromEdgesSet(set<string> &S,UOrderedSet_INT &Nodes);
	set<string> mySet_Union_string(set<string> &st1,set<string> &st2);
};

#endif /* COMPUTESNAPSHOTCHANGE_H_ */
