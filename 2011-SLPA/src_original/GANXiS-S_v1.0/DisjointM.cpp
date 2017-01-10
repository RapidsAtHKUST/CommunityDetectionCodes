/*
 * DisjointM.cpp
 *
 *  Created on: Nov 16, 2011
 *      Author: Jerry
 */

#include "DisjointM.h"
#include "fileOpts.h"
#include "Net.h"
#include "NODE.h"
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "CommonFuns_TMP_Measure.h"


//bool isUseLargestComp=false;
//bool isSymmetrize=true;
//string fileName_cpm="c:\\lou_l2.txt";   //Q=0.418803
//string fileName_cpm="c:\\karate.icpm";
//string fileName_net="c:\\karate.ipairs";


DisjointM::~DisjointM() {
	//delete net;
}

Net* DisjointM::INIT_readNetwork(string fileName_net,bool isUseLargestComp,bool isSymmetrize) {
	//read in net (***NEED TO BE RELEASED SOMEWHERE***)
	//---------------------------
	//Extract the fileName
	//---------------------------
	string a,b,netName,networkPath="";
	extractFileName_FullPath(fileName_net,netName,a,b);

	//---------------------------
	//	load network
	//---------------------------
	Net* net=new Net(networkPath,netName,fileName_net);
	net->readNetwork_EdgesList(fileName_net,isUseLargestComp,isSymmetrize);

	return net;
}

void DisjointM::INIT_readcpm_vp(string fileName_cpm,vector<vector<int> >& cpm, vector<vector<NODE *> >& cpm_vp,Net* net) {
	//return cpm and *cpm_vp*

	if(!fileName_cpm.empty()){
		//---------------------------
		//	Read in cpm
		//---------------------------
		readcpm(fileName_cpm,cpm);
		//printVectVect_PRIMITIVE<int>(cpm);
	}

	//---------------------------
	//****this checking is important***
	checkN(cpm,net->N);
	//cout<<"NO CHeckN()......Usually it should!!..."<<endl;
	//---------------------------
	convert2cpm_vp(cpm_vp,cpm,net);
}





void DisjointM::INIT_readcpm(string fileName_cpm,vector<vector<int> >& cpm) {
	//return cpm
	//---------------------------
	//	Read in cpm
	//---------------------------
	readcpm(fileName_cpm,cpm);
	printVectVect_PRIMITIVE<int>(cpm);
	//---------------------------
	//checkN(cpm,N);
}


void DisjointM::checkN(vector<vector<int> >& cpm,int N){
	int sum=0;
	//printVectVect_PRIMITIVE<int>(cpm);
	for(int i=0;i<cpm.size();i++)
		sum+=cpm[i].size();

	if(sum!=N){
		cout<<"ERROR: the N is not consistent (N="<<N<<",sum="<<sum<<")!"<<endl;
		exit(1);
	}
}

void DisjointM::convert2cpm_vp(vector<vector<NODE *> >& cpm_vp,vector<vector<int> >& cpm,Net* net){
	//---------------------------
	//	create cpm_vp
	//---------------------------
	//same ORDER as cpm

	for(int i=0;i<cpm.size();i++){
		vector<NODE *> oneCom;
		for(int j=0;j<cpm[i].size();j++){
			if(net->NODESTABLE.count(cpm[i][j])>0)
				oneCom.push_back(net->NODESTABLE[cpm[i][j]]);//get pointer
			else{
				cout<<"ERROR: ID="<<cpm[i][j]<<" is not found in the network!"<<endl;
				exit(1);
			}
		}

		cpm_vp.push_back(oneCom);
	}
}


void DisjointM::readcpm(string fileName,vector<vector<int> >& cpm){
	// Lines not starting with a number are ignored

	string oneLine, whiteSpace=" \t\n";
	fstream fp;
	fp.open(fileName.c_str(),fstream::in); //|fstream::out|fstream::app

	//cout<<"x="<<fileName<<endl;
	if(fp.is_open()){//if it does not exist, then failed

		// repeat until all lines is read
		while ( fp.good()){
			getline (fp,oneLine);

			//--------------------------
			//skip empty line
			if(oneLine.find_first_not_of(whiteSpace)==string::npos) continue;

			//skip any line NOT starting with digit number
			if(!isdigit(oneLine.at(oneLine.find_first_not_of(whiteSpace)))) continue;

			//cout<<"Line:"<<oneLine<<endl;
			//--------------------------
			//   get a new line (one comm)
			//--------------------------
			vector<int> oneCom;  //TO REMOVE

			int nodeId;
			stringstream linestream(oneLine);
			while(linestream>>nodeId){
				oneCom.push_back(nodeId);
			}

			cpm.push_back(oneCom);  //**copy to the

		}//while

		fp.close();
	}//if
	else{
		cout<<"read icpm:open failed"<<endl;
		exit(1);
	}
}

//--------------------------------------------------------------------
//				    Q: given network and cpm
//"Q_Finding Community Structure in Very Large Networks.pdf"
//--------------------------------------------------------------------
//double DisjointM::calQ_unw(string fileName_cpm,string fileName_net,bool isUseLargestComp,bool isSymmetrize){
double DisjointM::calQ_unw(string fileName_cpm,string fileName_net,Net* net,bool isUseLargestComp,bool isSymmetrize){
	//provide the network file or the net object(fileName_net="")

	//0. Read in cpm,cpm_vp and network
	vector<vector<int> > cpm;
	vector<vector<NODE *> > cpm_vp;

	if(!fileName_net.empty())
		net=INIT_readNetwork(fileName_net, isUseLargestComp, isSymmetrize);

	INIT_readcpm_vp(fileName_cpm, cpm, cpm_vp,net);

	//--------------------------------------
	//**We assume the Aij=1.0 or 0.0 (UNWEIGHTED)
	NODE* vi;
	NODE* vj;

	double Q;
	double M=net->M; // that is 2m in the Q definition


	double sum=0,ki,kj,Aij;
	for(int c=0;c<cpm_vp.size();c++){
		vector<NODE *>& oneCom=cpm_vp[c];

		for(int i=0;i<oneCom.size();i++){
			vi=oneCom[i];

			ki=vi->numNbs; //**convert int to double
			for(int j=0;j<oneCom.size();j++){
				vj=oneCom[j];
				kj=vj->numNbs;

				if(vi->nbSet.count(vj->ID)>0)
					Aij=1.0;
				else
					Aij=0;

				sum+=Aij - ki*kj/M;
			}
		}
	}//c

	Q=(1.0/M)*sum;


	//release memory
	if(!fileName_net.empty())
		delete net;

	return Q;
}


double DisjointM::calQ_unw_givenCpm(vector<vector<int> >& cpm,string fileName_net,Net* net,bool isUseLargestComp,bool isSymmetrize){
	//provide the network file or the net object(fileName_net="")

	//0. Given cpm,convert to cpm_vp and network
	//vector<vector<int> > cpm;
	vector<vector<NODE *> > cpm_vp;

	if(!fileName_net.empty())
		net=INIT_readNetwork(fileName_net, isUseLargestComp, isSymmetrize);

	INIT_readcpm_vp("", cpm, cpm_vp,net);

	//--------------------------------------
	//**We assume the Aij=1.0 or 0.0 (UNWEIGHTED)
	NODE* vi;
	NODE* vj;

	double Q;
	double M=net->M; // that is 2m in the Q definition


	double sum=0,ki,kj,Aij;
	for(int c=0;c<cpm_vp.size();c++){
		vector<NODE *>& oneCom=cpm_vp[c];

		for(int i=0;i<oneCom.size();i++){
			vi=oneCom[i];

			ki=vi->numNbs; //**convert int to double
			for(int j=0;j<oneCom.size();j++){
				vj=oneCom[j];
				kj=vj->numNbs;

				if(vi->nbSet.count(vj->ID)>0)
					Aij=1.0;
				else
					Aij=0;

				sum+=Aij - ki*kj/M;
			}
		}
	}//c

	Q=(1.0/M)*sum;


	//release memory
	if(!fileName_net.empty())
		delete net;

	return Q;
}


//--------------------------------------------------------------------
//				NMI: given true cpm and found cpm
//         "Comparing community structure identification"
//--------------------------------------------------------------------
void DisjointM::get_confusionMatrix(vector<vector<double> >& M,vector<double>& NI,vector<double>& NJ,double& N,vector<vector<int> >& cpmT,vector<vector<int> >& cpmF){
	//return NI,NJ,M, N(**double**) total number of nodes

	double n1=0,n2=0; //**double
	//3. N.i, N.j
	int sizeT=cpmT.size(),sizeF=cpmF.size();
	//vector<double> NI; //row sum
	for(int i=0;i<sizeT;i++){
		NI.push_back(cpmT[i].size());
		n1+=cpmT[i].size();
	}

	//vector<double> NJ; //column sum
	for(int i=0;i<sizeF;i++){
		NJ.push_back(cpmF[i].size());
		n2+=cpmF[i].size();
	}


	cout<<"NI.size="<<NI.size()<<endl;
	cout<<"NJ.size="<<NJ.size()<<endl;
	if(n1!=n2){
		cout<<"ERROR::n1!=n2"<<endl;
		exit(1);
	}else{
		N=n1;
		cout<<"N="<<N<<endl;
	}

	//-----------------------------------------
	//4. compute the confusion matrix
	//-----------------------------------------
	//vector<vector<double> > M;

	for(int i=0;i<sizeT;i++){
		vector<double> oneRow;  	 //***ORDER***
		for(int j=0;j<sizeF;j++){
			vector<int> intersect;	 //*new*;
			mySet_Intersect_orderedVect_PRIMITIVE<int>(cpmT[i],cpmF[j],intersect);

			oneRow.push_back(intersect.size());//*size*

			if(false){
				cout<<"------------"<<endl;
				printVect_PRIMITIVE<int>(cpmT[i]);
				printVect_PRIMITIVE<int>(cpmF[j]);
				printVect_PRIMITIVE<int>(intersect);
				cout<<"-size="<<intersect.size()<<endl;
			}
		}

		M.push_back(oneRow);
	}
}


void DisjointM::INIT_ConfusionMatrix(vector<vector<double> >& M,vector<double>& NI,vector<double>& NJ,\
		double& N,int& sizeT,int& sizeF, \
		string fileName_cpmT,string fileName_cpmF){
	//return M, NI, NJ, N,sizeT,sizeF

	//1. To use the c++ function, need to first soft the cpm
	vector<vector<int> > cpmT,cpmF;
	INIT_readcpm(fileName_cpmT, cpmT);
	INIT_readcpm(fileName_cpmF, cpmF);

	//2. **sort**
	sort_cpm_vectINT(cpmT);
	sort_cpm_vectINT(cpmF);

	sizeT=cpmT.size();
	sizeF=cpmF.size();

	get_confusionMatrix(M, NI, NJ, N, cpmT, cpmF);
}

double DisjointM::calNMI(string fileName_cpmT,string fileName_cpmF){
	//with single com, NMI=0;
	//pefect matching, NMI=1;

	double NMI;
	//-------------------------------------------
	int sizeT,sizeF;
	vector<double> NI,NJ;          //row sum,column sum//
	vector<vector<double> > M;     //confusion matrix
	double N;                      //**double**total number of nodes

	INIT_ConfusionMatrix(M, NI, NJ, N, sizeT, sizeF, fileName_cpmT, fileName_cpmF);

	cout<<"N="<<N<<" M.size="<<M.size()<<" NI="<<NI.size()<<" NJ="<<NJ.size()\
			<<" sizeT="<<sizeT<<" sizeF="<<sizeF<<endl;

	//-------------------------------------------
	double U=0;
	for(int i=0;i<sizeT;i++){
		for(int j=0;j<sizeF;j++){

			if(M[i][j]==0)//fix the log(0),let it=0;
				U+=0.0;
			else //**ALL should be double, log is natural log
				U+=M[i][j]*log( (M[i][j]*N) / (NI[i]*NJ[j]) );
		}
	}

	U*=-2;

	double V=0;
	for(int i=0;i<sizeT;i++){
		V+=NI[i]*log(NI[i]/N);

	}

	for(int j=0;j<sizeF;j++){
		V+=NJ[j]*log(NJ[j]/N);

	}

	NMI=U/V;

	return NMI;
}

//---------------------------------------
//
//"Identifying and evaluating community structure in complex networks"
//wiki
double DisjointM::calARI(string fileName_cpmT,string fileName_cpmF){
	//with single com, ARI=0; ?
	//pefect matching, ARI=1; ?

	double ARI; //all should be **double**

	//-------------------------------------------
	int sizeT,sizeF;
	vector<double> NI,NJ;          //row sum,column sum//
	vector<vector<double> > M;     //confusion matrix
	double N;                      //**double**total number of nodes

	INIT_ConfusionMatrix(M, NI, NJ, N, sizeT, sizeF, fileName_cpmT, fileName_cpmF);

	//-------------------------------------------
	double A=0;
	for(int i=0;i<sizeT;i++){
		for(int j=0;j<sizeF;j++){
			if(M[i][j]>=2) //***
				A+=nchoosek(M[i][j],2);
		}
	}

	double b1=0;
	for(int i=0;i<sizeT;i++){
		if(NI[i]>=2) //***
			b1+=nchoosek(NI[i],2);
	}

	double b2=0;
	for(int j=0;j<sizeF;j++){
		if(NJ[j]>=2) //***
			b2+=nchoosek(NJ[j],2);
	}

	double B=b1*b2/nchoosek(N,2);
	double C=(b1+b2)/2.0;

	ARI=(A-B)/(C-B);

	return ARI;
}





