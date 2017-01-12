//============================================================================
// Name        : fileOpts.cpp
// Author      : Jierui Xie (xiej2@rpi.edu)
// Date        : Oct. 2011
// Version     : v1.0
// Copyright   : All rights reserved.
// Description : SLPA algorithm for community detection.
// Web Site    : https://sites.google.com/site/communitydetectionslpa/
// Publication:
//             J. Xie, B. K. Szymanski and X. Liu, "SLPA: Uncovering Overlapping Communities in Social Networks via A Speaker-listener Interaction Dynamic Process", IEEE ICDM workshop on DMCCI 2011, Vancouver, CA.
//============================================================================

#include "fileOpts.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
using namespace std;

bool isFileExist(const char* fileName){
	fstream fp;
	fp.open(fileName,fstream::in);//|fstream::out|fstream::app

	if(fp.is_open()){
		fp.close();
		return true;
	}else
		return false;
}

void writeToTxt(string fileName, bool isappend, vector<string>& data){
	//it is good way to do and similar to the c style.

	fstream fp;

	if(isappend)
		fp.open(fileName.c_str(),fstream::app);//|fstream::out|fstream::app
	else
		fp.open(fileName.c_str(),fstream::out);//|fstream::out|fstream::app

	if(fp.is_open()){//if it does not exist, then failed

		for(int i=0;i<data.size();i++){
			fp<<data[i]<<endl;
		}
		fp.close();

		//cout<<"write to "<<fileName<<endl;
	}
	else{
		cout<<"open failed"<<endl;
	}
}

void extractFileName_Extention(const string& str,string& file,string& ext){
	size_t found;

	found=str.find_last_of(".");  //extention
	if(found==string::npos){
		file=str;
		ext="";
	}
	else{
		file=str.substr(0,found);
		ext=str.substr(found+1);
	}
}
void extractFileName_FullPath (const string& str,string& file, string& ext, string& path)
{cout<<"str="<<str<<endl;
	size_t found;
	found=str.find_last_of("/\\"); //any character

	//get the path and full filename
	if(found==string::npos){
		path="";
		extractFileName_Extention(str,file,ext);
	}
	else{
		//path=str.substr(0,found);
		path=str.substr(0,found+1);  //?????????????

		//cout<<"path="<<path<<endl;
		//file=str.substr(found+1);
		extractFileName_Extention(str.substr(found+1),file,ext);
	}

	/*
	//get the short file name
	found=file.find_last_of("."); //any character
	if(found==string::npos){
		shortfile=file;
	}
	else{
		shortfile=file.substr(0,found);
	}
	*/

	//cout<<"p="<<path<<" f="<<file<<" sf="<<shortfile<<endl;
}




