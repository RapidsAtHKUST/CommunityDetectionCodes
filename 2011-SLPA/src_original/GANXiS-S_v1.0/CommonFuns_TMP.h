/*
 * CommonFuns_TMP.h
 *
 *
 *      Author: Jerry
 */

#ifndef COMMONFUNS_TMP_H_
#define COMMONFUNS_TMP_H_

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

using namespace std;




//void printSet_INT(set<int>& st);
template <class T>
void printVect_PRIMITIVE(vector<T>& vect){
	for(int i=0;i<vect.size();i++)
		cout<<vect[i]<<" ";
	cout<<endl;
}


//void printVect_INT(vector<int>& vect);
//void printVectVect_INT(vector<vector<int> >& cpm);
//void sortVecVec_bySize_INT(vector<vector<int> >& vect);
template <class T>
void printSet_PRIMITIVE(set<T>& st){
	typename set<T>::iterator it;    //***
	for (it=st.begin(); it!=st.end(); it++)
		cout << " " << *it;
	cout << endl;
}

template <class T>
void printVectVect_PRIMITIVE(vector<vector<T> >& cpm){
	for(int i=0;i<cpm.size();i++){
		//sort(cpm[i].begin(),cpm[i].end(),sort_INT_INC());

		cout<<"Vect"+int2str(i)<<":";
		printVect_PRIMITIVE<T> (cpm[i]);
	}
}

template <class T>
struct sort_Vec_bySize_DEC {
	//bool operator()(vector<int>& i, vector<int>& j) {
	//	return i.size() > j.size();
	//} **ERROR**:

	bool operator()(vector<T> i, vector<T> j) {
		return i.size() > j.size();
	}
};

template <class T>
void sortVecVec_bySize(vector<vector<T> >& vect){
	//sort the vector of vector by the **decreasing** vector size

	//-------------------
	//cout<<"org-"<<endl;
	//printVectVect_INT(vect);

	//-------------------

	sort(vect.begin(),vect.end(),sort_Vec_bySize_DEC<T>()); //***

	//-------------------
	//cout<<"-sort"<<endl;
	//printVectVect_INT(vect);
}

template <class T>
struct sort_Vec_bySize_DEC_pointer {

	bool operator()(vector<T>* i, vector<T>* j) {
		return i->size() > j->size();
	}
};

template <class T>
void sortVecVec_bySize_pointer(vector<vector<T>* >& vect){
	//sort the vector of vector by the **decreasing** vector size

	//-------------------
	//cout<<"org-"<<endl;
	//printVectVect_INT(vect);

	//-------------------

	sort(vect.begin(),vect.end(),sort_Vec_bySize_DEC_pointer<T>()); //***

	//-------------------
	//cout<<"-sort"<<endl;
	//printVectVect_INT(vect);
}

//----------------------------------------
//bool isSubset_VET_INT(vector<int> vlarge,vector<int> vsmall);
template <class T>
bool isSubset_VET_PRIMITIVE(vector<T>& vlarge,vector<T>& vsmall){
	//check if vlarge contains vsmall

	//---------------------------
	sort (vlarge.begin(),vlarge.end()); //using default
	sort (vsmall.begin(),vsmall.end());

	// using default comparison:
	if (includes(vlarge.begin(),vlarge.end(),vsmall.begin(),vsmall.end()))
		//cout << "container includes continent!" << endl;
		return true;
	else
		return false;
}

//void mySet_Union_Vect_INT(vector<int>& first,vector<int>& second,vector<int>& output);
//void mySet_Intersect_Vect_INT(vector<int>& first,vector<int>& second,vector<int>& output);
//void mySet_Difference_Vect_INT(vector<int>& first,vector<int>& second,vector<int>& output);


//set<int>  mySet_Union_INT(set<int>& st1,set<int>& st2);
//set<int>  mySet_Intersect_INT(set<int>& st1,set<int>& st2);
//set<int>  mySet_Diff_INT(set<int>& st1,set<int>& st2);
//int getFirstElemnet_Set_INT(set<int>& st);
//bool isSubset_INT(set<int> vlarge,set<int> vsmall);
//bool isSubset_INT(set<int> st1,set<int> st2);
//bool isSetMember_INT(set<int> st,int a);

template <class T>
void mySet_Union_Vect_PRIMITIVE(vector<T>& first,vector<T>& second,vector<T>& output){
	typename vector<T>::iterator it,eit;

	output.clear();
	output=	vector<T>(first.size()+second.size()); // 0  0 ...
	//-------
	sort (first.begin(),first.end());     //  5 10 15 20 25
	sort (second.begin(),second.end());   // 10 20 30 40 50

	//-------
	//eit point to the last position
	eit=set_union (first.begin(),first.end(), second.begin(),second.end(), output.begin());
	//-------
	output.erase(eit,output.end());
	//printVect_INT(output);
}
template <class T>
void mySet_Intersect_Vect_PRIMITIVE(vector<T>& first,vector<T>& second,vector<T>& output){
	typename vector<T>::iterator it,eit;

	output.clear();
	output=	vector<T>(first.size()+second.size()); // 0  0 ...
	//-------
	sort (first.begin(),first.end());     //  5 10 15 20 25
	sort (second.begin(),second.end());   // 10 20 30 40 50
	//-------
	//eit point to the last position
	eit=set_intersection (first.begin(),first.end(), second.begin(),second.end(), output.begin());

	//-------
	output.erase(eit,output.end());
	//printvect_INT(output);
}

template <class T>
void mySet_Difference_Vect_PRIMITIVE(vector<T>& first,vector<T>& second,vector<T>& output){
	//setdiff first and second
	typename vector<T>::iterator it,eit;

	output.clear();
	output=	vector<T>(first.size()+second.size()); // 0  0 ...
	//-------
	sort (first.begin(),first.end());     //  5 10 15 20 25
	sort (second.begin(),second.end());   // 10 20 30 40 50
	//-------
	//eit point to the last position
	eit=set_difference (first.begin(),first.end(), second.begin(),second.end(), output.begin());

	//-------
	output.erase(eit,output.end());
	//printvect_INT(output);
}


//------------------------------------------------
template <class T>
set<T>  mySet_Union_PRIMITIVE(set<T>& st1,set<T>& st2){
	set<T> st3;
	typename set<T>::iterator it;

	//cout<<"--union-----"<<endl;
	st3.insert(st1.begin(),st1.end());
	st3.insert(st2.begin(),st2.end());
	//printset_INT2(st3);

	return st3;
}

template <class T>
set<T>  mySet_Intersect_PRIMITIVE(set<T>& st1,set<T>& st2){
	set<T> st3;
	typename set<T>::iterator it;

	//cout<<"--intersect-----"<<endl;
	//check if ele in st1 also in st2
	for(it=st1.begin();it!=st1.end();it++){
		if(st2.count(*it)>0)  //**
			st3.insert(*it);
	}

	return st3;
}

template <class T>
set<T>  mySet_Diff_PRIMITIVE(set<T>& st1,set<T>& st2){
	set<T> st3;
	typename set<T>::iterator it;

	//cout<<"--difference-----"<<endl;
	st3.clear();
	//check the element that is in st1 but not in st2
	for(it=st1.begin();it!=st1.end();it++){
		if(st2.count(*it)==0) //**
			st3.insert(*it);
	}

	return st3;
}

template <class T>
bool isSubset_PRIMITIVE(set<T> st1,set<T> st2){
	//if st1 inclues st2

	bool flag=true;
	typename set<T>::iterator it;

	//cout<<"is subset:"<<endl;
	for(it=st2.begin();it!=st2.end();it++){
		if(st1.count(*it)==0){ //**
			flag=false;
			break;
		}
	}

	return flag;
}

template <class T>
bool isSetMember_PRIMITIVE(set<T> st,T a){
	//if st1 inclues a
	bool flag=true;

	if(st.count(a)==0){ //**
		flag=false;
	}

	return flag;
}

template <class T>
int getFirstElemnet_Set_PRIMITIVE(set<T>& st){
	//set.size()>0
	//int v=-1;
	typename set<T>::iterator it;

	it=st.begin();

	return (*it);
}




#endif /* COMMONFUNS_TMP_H_ */
