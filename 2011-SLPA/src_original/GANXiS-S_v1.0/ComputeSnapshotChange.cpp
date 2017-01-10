/*
 * ComputeSnapshotChange.cpp
 *
 *  Created on: Mar 23, 2012
 *      Author: Jerry
 */

#include "ComputeSnapshotChange.h"


ComputeSnapshotChange::ComputeSnapshotChange(string fileName1,string fileName2) {
	//---------------------------------
	// read in two edgelist
	read_snapshot_2set(fileName1,EdgesSet1);
	read_snapshot_2set(fileName2,EdgesSet2);

	//---------------------------------
	//compute the difference in edges
	setdiff_S(EdgesSet2,EdgesSet1,de1);
	setdiff_S(EdgesSet1,EdgesSet2,de2);

	//cout<<"-------add edges------"<<endl;
	//printSetS(de1);
	//cout<<"-------delete edges------"<<endl;
	//printSetS(de2);

	//---------------------------------
	//get the nodes involved
	parseNodeIDs_fromEdgesSet(de1,ds1);
	parseNodeIDs_fromEdgesSet(de2,ds2);

	DSet=mySet_Union_int(ds1,ds2);

	//cout<<"-------add nodes------"<<endl;
	//printSetInt(ds1);
	//cout<<"-------delete nodes------"<<endl;
	//printSetInt(ds2);
	//cout<<"-------changed nodes------"<<endl;
	//printSetInt(DSet);

	parseNodeIDs_fromEdgesSet(EdgesSet1,SN1);
	parseNodeIDs_fromEdgesSet(EdgesSet2,SN2);


	SN12=mySet_Union_int(SN1,SN2);

	EdgesSet12=mySet_Union_string(EdgesSet1,EdgesSet2);

}

void ComputeSnapshotChange::parseNodeIDs_fromEdgesSet(set<string> &S,UOrderedSet_INT &Nodes){
	//get all the involved nodes distinct ids
	Nodes.clear();
	for(set<string>::iterator it=S.begin();it!=S.end();it++){
		string oneLine=(*it);

		//=======================================
		int fromID, toID;  //**int or double may make a diffence

		stringstream linestream(oneLine);
		if((linestream>>fromID) && (linestream>>toID)){
			//printf("fromID=%d toID=%d\n",fromID,toID);

			//-----------------------------------
			if(Nodes.count(fromID)==0){
				Nodes.insert(fromID);
			}
			if(Nodes.count(toID)==0){
				Nodes.insert(toID);
			}

		}else{
			cout<<oneLine<<endl;
			cout<<"error when reading edge!"<<endl;
		}

	} //for
}

void ComputeSnapshotChange::printSetS(set<string> S){
	cout<<"------------------"<<endl;
	for(set<string>::iterator it=S.begin();it!=S.end();it++){
		cout<<(*it)<<endl;
	}
	cout<<"size="<<S.size()<<endl;
}


void ComputeSnapshotChange::printSetInt(UOrderedSet_INT S){
	cout<<"------------------"<<endl;
	for(UOrderedSet_INT::iterator it=S.begin();it!=S.end();it++){
		cout<<(*it)<<endl;
	}
	cout<<"size="<<S.size()<<endl;
}

/*
void write2txt_UNDiredted(string fileName,set<string> &EdgesSet) {
	//output a file for each file,

	vector<string> data;
	for(set<string>::iterator it=EdgesSet.begin();it!=EdgesSet.end();it++){
		string line=(*it);  //a string
		data.push_back(line);
	}

	//fileOpts.writeToTxt(fileName, false, data);// **false:the first one
	writeToTxt(fileName, false, data);
}
 */

//----------------------------------------
void ComputeSnapshotChange::read_snapshot_2set(string fileName,set<string>& EdgesSet){
	//only take (fromID,toID....)
	// read in whateven it is
	EdgesSet.clear();

	string oneLine, whiteSpace=" \t\n";
	fstream fp;
	fp.open(fileName.c_str(),fstream::in); //|fstream::out|fstream::app

	if(fp.is_open()){//if it does not exist, then failed

		// repeat until all lines is read
		int cn=0;
		while ( fp.good()){
			getline (fp,oneLine);

			//--------------------------
			//skip empty line
			if(oneLine.find_first_not_of(whiteSpace)==string::npos) continue;

			//skip any line NOT starting with digit number
			if(!isdigit(oneLine.at(oneLine.find_first_not_of(whiteSpace)))) continue;

			//=======================================
			int fromID, toID;  //**int or double may make a diffence

			stringstream linestream(oneLine);
			if((linestream>>fromID) && (linestream>>toID)){
				//printf("fromID=%d toID=%d\n",fromID,toID);

				//-----------------------------------
				string e=int2str(fromID)+" "+int2str(toID);  //**Key
				if(EdgesSet.count(e)==0){
					EdgesSet.insert(e);
					cn++;
				}

			}else{
				cout<<oneLine<<endl;
				cout<<"error when reading edge!"<<endl;
			}
		} //while

		//-----------------------------------
		//cout<<"# of reading="<<cn<<endl;
		fp.close();
	}
	else{
		cout<<"open failed"<<endl;
		exit(1);
	}
}


void ComputeSnapshotChange::setdiff_S(set<string> &EdgesSet1,set<string>& EdgesSet2,set<string>& dff){
	// compute the ele in 1, but not in 2
	// setdiff(s1,s2)
	dff.clear();


	for(set<string>::iterator it=EdgesSet1.begin();it!=EdgesSet1.end();it++){
		string line=(*it);      //a string

		//if S2 does not contains ele in S1
		if(EdgesSet2.count(line)==0)
			dff.insert(line);
	}
}

void ComputeSnapshotChange::setdiff_Int(set<int> &S1,set<int>& S2,set<int>& dff){
	// compute the ele in S1, but not in S2
	// setdiff(s1,s2)
	dff.clear();

	for(set<int>::iterator it=S1.begin();it!=S1.end();it++){

		//if S2 does not contains ele in S1
		if(S2.count(*it)==0)
			dff.insert(*it);
	}
}

UOrderedSet_INT ComputeSnapshotChange:: mySet_Union_int(UOrderedSet_INT &st1,UOrderedSet_INT &st2){
	UOrderedSet_INT st3;

	//cout<<"--union-----"<<endl;
	st3.insert(st1.begin(),st1.end());
	st3.insert(st2.begin(),st2.end());
	//printset_INT2(st3);

	return st3;
}

set<string> ComputeSnapshotChange:: mySet_Union_string(set<string> &st1,set<string> &st2){
	set<string> st3;

	//cout<<"--union-----"<<endl;
	st3.insert(st1.begin(),st1.end());
	st3.insert(st2.begin(),st2.end());
	//printset_INT2(st3);

	return st3;
}
