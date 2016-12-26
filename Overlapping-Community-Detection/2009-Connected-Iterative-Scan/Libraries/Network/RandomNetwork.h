#ifndef RNET_RPI
#define RNET_RPI

#include "Network.h"
#include "../Random/PowerLaw.h"

void RandomNetwork ( const int& V, const double& dist, const int& min_deg, const int& max_deg, const double& mix, const vector < set < int > >& communities ){
  vector < int > degrees ( V.size() );
  shared_ptr < PowerLaw > PL ( new PowerLaw ( dist, min_deg, max_deg ) );

  for ( int i = 0; i < degrees.size(); i++ ){
    degrees.push_back( );
  }
}

#endif
