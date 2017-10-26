// ***********************************************************************
//
//            Grappolo: A C++ library for graph clustering
//               Mahantesh Halappanavar (hala@pnnl.gov)
//               Pacific Northwest National Laboratory     
//
// ***********************************************************************
//
//       Copyright (2014) Battelle Memorial Institute
//                      All rights reserved.
//
// Redistribution and use in source and binary forms, with or without 
// modification, are permitted provided that the following conditions 
// are met:
//
// 1. Redistributions of source code must retain the above copyright 
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright 
// notice, this list of conditions and the following disclaimer in the 
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its 
// contributors may be used to endorse or promote products derived from 
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
// POSSIBILITY OF SUCH DAMAGE.
//
// ************************************************************************

#ifndef _INPUT_PARAMS_
#define _INPUT_PARAMS_

#include<iostream>
#include<cstdlib>
#include <unistd.h>   //Older version of getopt
//#include <getopt.h> //Newer version with long name support (does not work on XMT

struct input_parameters {
  
  const char *problemname;
  double alpha;
  double beta;
  double gamma;
  int maxiter;
  const char *alg;
  int dampingtype;
  int batchrounding;
  bool verbose;
  bool finalize;
  bool quiet; 
  bool approx;
  bool limitthreads;
  int chunk;
  const char *outFile;
  
  netalign_parameters();   
  void usage();    
  bool parse(int argc, char *argv[]);
};


input_parameters::input_parameters() 
: problemname(NULL), alpha(1.0), beta(2.0),
gamma(0.99), maxiter(1000), alg("mp"), dampingtype(1),
batchrounding(2), verbose(false), finalize(false), quiet(false),
approx(true),limitthreads(false),chunk(1000),outFile(NULL)
{}

void input_parameters::usage() {
  cout << "Driver <Options> Base-FileName\n";
  cout << "********************************************"<< endl;
  cout << "Input Options: \n";
  cout << "********************************************"<< endl;
  cout << "verbose:  -v" << endl;
  cout << "finalize: -f" << endl;
  cout << "alpha:    -a <value> -- default:" << alpha << endl;
  cout << "beta:     -b <value> -- default: " << beta << endl;
  cout << "gamma:    -g <value> -- default:" << gamma << endl;
  cout << "maxiter:  -n <value  -- default:" << maxiter << endl;
  cout << "batch:    -r <value> -- default:" << batchrounding << endl;
  cout << "damping:  -d <value> -- default:" << dampingtype << endl;
  cout << "problem: <Base-FileName>" << endl;
  cout << "********************************************"<< endl;
}

bool netalign_parameters::parse(int argc, char *argv[]) {
  cout<<"About to parse runtime parameters: "<<argc<<"\n";
  static const char *opt_string = "va:b:g:n:d:r:c:";
  int opt = getopt(argc, argv, opt_string);
  while (opt != -1) {
    switch (opt) {
    case 'v': verbose = true; break;
      
    case 'a': 
      alpha = atof(optarg);
      if (alpha < 0.) {
	cerr << "alpha must be non-negative, but alpha = " 
	     << alpha << " < 0." << endl;
	return false;
      }
      break;
      
    case 'b': 
      beta = atof(optarg);
      if (beta < 0.) {
	cerr << "beta must be non-negative, but beta = " 
	     << beta << " < 0." << endl;
	return false;
      }
      break;
      
    case 'g': 
      gamma = atof(optarg);
      if (gamma < 0. || gamma > 1.) {
	cerr << "gamma must be between 0 and 1 but gamma = "
	     << gamma << "." << endl;
	return false;
      }
      break;
      
    case 'c':
      chunk = atoi(optarg);
      if (chunk <= 0) {
	cerr << "chunk must be positive, but chunk = "
	     << chunk << " <= 0" << endl;
	return false;
      }
      break;
      
    case 'n':
      maxiter = atoi(optarg);
      if (maxiter <= 0) {
	cerr << "maxiter must be positive, but maxiter = "
	     << maxiter << " <= 0" << endl;
	return false;
      }
      break;
      
    case 'd':
      dampingtype = atoi(optarg);
      if (dampingtype <= 0 || dampingtype > 3) {
	cerr << "damping must 1, 2, or 3, but damping = "
	     << dampingtype << "." << endl;
	return false;
      }
      break;
      
    case 'r':
      batchrounding = atoi(optarg);
      if (batchrounding < 0) {
	cerr << "batch must be non-negative, but batch = "
	     << batchrounding << " <= 0" << endl;
	return false;
      }
      break;
   
    case 'o':
      outFile = optarg;
      cout<<outFile<<"DumpClusterIds"<<endl;
      if (outFile == NULL) {
	cerr << "Output file should be specified, but file ="
	     << "NULL" << endl;
	return false;
      }
      break;
      
    default:
      cerr << "unknown argument" << endl;
      return false;   

    }
    opt = getopt(argc, argv, opt_string);
  }
  if (argc - optind != 1) {
    cerr << "problem name not specified.  Exiting." << endl;
    return false;
  } else {
    problemname = argv[optind];
  }
  
  if (verbose) {
    cout << "********************************************"<< endl;
    cout << "Input Parameters: \n";
    cout << "********************************************"<< endl;    
    cout << "finalize: " << finalize << endl;
    cout << "alpha: " << alpha << endl;
    cout << "beta: " << beta << endl;
    cout << "gamma: " << gamma << endl;
    cout << "maxiter: " << maxiter << endl;
    cout << "algorithm: " << alg << endl;
    cout << "damping: " << dampingtype << endl;
    cout << "batch: " << batchrounding << endl;
    cout << "approx: " << approx << endl;
    cout << "limitthreads: " << limitthreads << endl;
    cout << "problem: " << problemname << endl;
    cout << "********************************************"<< endl;
  }
  
  return true;
}

#endif
