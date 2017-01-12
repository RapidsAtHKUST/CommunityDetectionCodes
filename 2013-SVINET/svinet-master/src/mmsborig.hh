#ifndef MMSBORIG_HH
#define MMSBORIG_HH

#include <list>
#include <utility>
#include <unistd.h>

#include "env.hh"
#include "matrix.hh"
#include "network.hh"

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_psi.h>
#include <gsl/gsl_sf.h>

class MMSBOrig {
public:
  MMSBOrig(Env &env, Network &network);
  void gen(double alpha);
  void ppc();
  void get_lc_stats();
  void gml();
  void obs_data();
  void draw_and_save(uint32_t id);
  void draw_membership_indicators(uint32_t p, uint32_t q);

private:
  Env &_env;
  Network &_network;
  gsl_rng *_r;
  bool _ppc;
  
  uint32_t _n;
  uint32_t _k;
  uint32_t _t;

  AdjMatrix _gen_y;
  uint32_t _gen_ones;

  Array _alpha;
  Matrix _eta;

  Matrix _pi;
  Matrix _beta;

  Matrix _gamma;
  Matrix _lambda;
  uint32_t _curr_id;

  uint32_t _total_pairs;
};

#endif
