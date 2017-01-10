//============================================================================
// Name        : NODE.cpp
// Author      : Jierui Xie (xiej2@rpi.edu)
// Date        : Oct. 2011
// Version     : v1.0
// Copyright   : All rights reserved.
// Description : SLPA algorithm for community detection.
// Web Site    : https://sites.google.com/site/communitydetectionslpa/
// Publication:
//             J. Xie, B. K. Szymanski and X. Liu, "SLPA: Uncovering Overlapping Communities in Social Networks via A Speaker-listener Interaction Dynamic Process", IEEE ICDM workshop on DMCCI 2011, Vancouver, CA.
//============================================================================


#include "NODE.h"

NODE::NODE() {
	// TODO Auto-generated constructor stub

}

NODE::~NODE() {
	// TODO Auto-generated destructor stub
}


void NODE::printBCHistogram(){
	cout<<"ID="<<ID<<":";
	for(UOrderedH_INT_DBL::iterator mit=BCHistgram.begin();mit!=BCHistgram.end();mit++){
		//cout<<mit->first<<" :"<<mit->second<<endl;
		cout<<mit->first<<"("<<mit->second<<")";
	}
	cout<<endl;
}

void NODE::printBCHistMapEntryList(){
	cout<<"ID="<<ID<<":(mapEntryList)";

	for(int i=0;i<BCHistMapEntryList.size();i++)
		cout<<BCHistMapEntryList[i].first<<"("<<BCHistMapEntryList[i].second<<")";

	cout<<endl;
}


void NODE::printnewBCHistogram(){
	cout<<"ID="<<ID<<"(newBC):";
	for(UOrderedH_INT_DBL::iterator mit=newBCHistgram.begin();mit!=newBCHistgram.end();mit++){
		//cout<<mit->first<<" :"<<mit->second<<endl;
		cout<<mit->first<<"("<<mit->second<<")";
	}
	cout<<endl;
}

void NODE::printnewBCHistMapEntryList(){
	cout<<"ID="<<ID<<":(newBC)";

	for(int i=0;i<newBCHistMapEntryList.size();i++)
		cout<<newBCHistMapEntryList[i].first<<"("<<newBCHistMapEntryList[i].second<<")";

	cout<<endl;
}
