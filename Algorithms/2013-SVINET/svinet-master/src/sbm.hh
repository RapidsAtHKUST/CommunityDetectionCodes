#ifndef SBM_HH
#define SBM_HH

#include <list>
#include <utility>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#include "env.hh"
#include "matrix.hh"
#include "network.hh"
#include "thread.hh"
#include "tsqueue.hh"
#include "fastqueue.hh"

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_psi.h>
#include <gsl/gsl_sf.h>

//#define GLOBALPHIS 1

class SBM {
public:
  SBM(Env &env, Network &network);
  ~SBM();

  void batch_infer();
  void infer();
  void save_model();
  
  static void set_dir_exp(const Matrix &u, Matrix &exp);
  static void set_dir_exp(const Array &u, Array &exp);
  static void set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp);

  static const unsigned int THRESH = 0.99;

private:
  void init_heldout();
  void set_heldout_sample(int sz);
  void set_validation_sample(int sz);
  void set_precision_sample();

  double heldout_likelihood();
  double validation_likelihood();
  void compute_precision(uint32_t &c10, uint32_t &c100, uint32_t &c1000);
  void gml();
  uint32_t most_likely_group(uint32_t p);

  void get_random_edge(bool link, Edge &e) const;
  void init_gamma();
  int  init_lambda();
  void init_phi();

  void update_phi(const vector<uint32_t> &nodes);
  void update_phi2(uint32_t a);
  void update_phit(uint32_t p, const vector<Edge> &pairs, 
		   double gamma_scale, double phi_scale);
  void update_phi_helper(const Matrix &m, 
			 uint32_t p, uint32_t q, uint32_t k, 
			 double &sum);

  void update_gamma_lambda();
  void update_gammat_lambdat(const vector<Edge> &pairs, 
			     double gamma_scale, 
			     double lambda_scale);
  void get_inf_sample(vector<Edge> &sample,
		      vector<uint32_t> &nodes);
  void get_noninf_sample(vector<Edge> &sample, 
			 vector<uint32_t> &nodes);
  void opt_process(vector<uint32_t> &nodes);
  void opt_process_noninf(vector<uint32_t> &nodes);

  void batch_update_phi(uint32_t p); // batch
  
  yval_t get_y(uint32_t p, uint32_t q);
  uint32_t precision_ones() const;
  uint32_t precision_zeros() const;
  
  void mark_seen(uint32_t a, uint32_t b);
  void mark_seen(yval_t **yd, uint32_t a, uint32_t b);

  string edgelist_s(EdgeList &elist);
  uint32_t duration() const;
  
#ifdef GLOBALPHIS
  double approx_log_likelihood();
#endif

  double edge_likelihood(uint32_t p, uint32_t q, yval_t y);
  double edge_likelihood2(uint32_t p, uint32_t q, yval_t y);
  double estimate_bernoulli_rate(uint32_t k) const;
  void estimate_beta(Array &beta) const;
  void estimate_pi();
  bool edge_ok(const Edge &e) const;

  Env &_env;
  Network &_network;

  SampleMap _heldout_map;
  SampleMap _validation_map;
  SampleMap _precision_map;
  
  uint64_t _n;
  uint32_t _k;
  uint32_t _K;
  uint32_t _t;
  uint32_t _s;
  uint32_t _iter;
  Array _alpha;

  yval_t _family;
  Matrix _eta;

  double _ones_prob;
  double _zeros_prob;
  uint32_t _hitcurve_id;
  double _inf_epsilon;

  EdgeList _heldout_pairs;
  EdgeList _validation_pairs;
  EdgeList _precision_pairs;
  gsl_rng *_r;

  Array _gamma;
  Matrix _lambda;
  Matrix _phi;

  Array _gammat;
  Matrix _lambdat;
  Matrix _phit;

  double _tau0;
  double _kappa;
  double _nodetau0;
  double _nodekappa;
  
  double _rhot;
  Array _noderhot;
  Array _nodec;

  time_t _start_time;
  struct timeval _last_iter;
  FILE *_lf;

  Array _Elogpi;
  Matrix _Elogbeta;
  Array _Epi;

  FILE *_cf;
  FILE *_hf;
  FILE *_vf;
  FILE *_pf;
  FILE *_hcf;
  FILE *_tf;
  FILE *_trf;
  FILE *_mf;
  FILE *_hef;
  FILE *_vef;
  FILE *_tef;

  uint64_t _total_pairs;
  double _max_t, _max_h, _max_v, _prev_h, _prev_w, _prev_t;
  mutable uint32_t _nh, _nt;

  uint32_t _start_node;
};

inline uint32_t
SBM::duration() const
{
  time_t t = time(0);
  return t - _start_time;
}

inline void
SBM::set_dir_exp(const Matrix &u, Matrix &exp)
{
  const double ** const d = u.data();
  double **e = exp.data();
  for (uint32_t i = 0; i < u.m(); ++i) {
    // psi(e[i][j]) - psi(sum(e[i]))
    double s = .0;
    for (uint32_t j = 0; j < u.n(); ++j) 
      s += d[i][j];
    //debug("set_dir_exp: s = %f\n",s);
    double psi_sum = gsl_sf_psi(s);
    for (uint32_t j = 0; j < u.n(); ++j) {
      //debug("set_dir_exp: d[i][j] = %f\n",d[i][j]);
      e[i][j] = gsl_sf_psi(d[i][j]) - psi_sum;
    }
  }
}

inline void 
SBM::set_dir_exp(const Array &u, Array &exp)
{
  const double * const d = u.data();
  double *e = exp.data();
  double s = .0;
  for (uint32_t i = 0; i < u.n(); ++i)
    s += d[i];
  double psi_sum = gsl_sf_psi(s);
  for (uint32_t i = 0; i < u.n(); ++i) 
    e[i] = gsl_sf_psi(d[i]) - psi_sum;
}

inline void
SBM::set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp)
{
  const double ** const d = u.data();
  double **e = exp.data();

  double s = .0;
  for (uint32_t j = 0; j < u.n(); ++j) 
    s += d[a][j];
  double psi_sum = gsl_sf_psi(s);
  for (uint32_t j = 0; j < u.n(); ++j) 
    e[a][j] = gsl_sf_psi(d[a][j]) - psi_sum;
}

inline void
SBM::estimate_pi()
{
  const double * const gd = _gamma.const_data();
  double *epid = _Epi.data();
  double s = .0;
  for (uint32_t k = 0; k < _k; ++k)
    s += gd[k];
  assert(s);
  for (uint32_t k = 0; k < _k; ++k)
    epid[k] = gd[k] / s;
}

inline double
SBM::estimate_bernoulli_rate(uint32_t k) const
{
  const double ** const ld = _lambda.data();
  double s = .0;
  for (uint32_t t = 0; t < _t; ++t)
    s += ld[k][t];
  assert(s);
  return ld[k][0] / s;
}

inline void
SBM::estimate_beta(Array &beta) const
{
  const double ** const ld = _lambda.const_data();
  for (uint32_t k = 0; k < _K; ++k) {
    double s = .0;
    for (uint32_t t = 0; t < _t; ++t)
      s += ld[k][t];
    beta[k] = ld[k][0] / s;
  }
}

inline double
SBM::edge_likelihood(uint32_t p, uint32_t q, yval_t y)
{
  Array ones(_K);
  Array zeros(_K);

  for (uint32_t i = 0; i < _k+1; ++i) {
    double u = estimate_bernoulli_rate(i);
    ones[i] = gsl_ran_bernoulli_pdf(1, u);
    zeros[i] = gsl_ran_bernoulli_pdf(0, u);
  }

  double s = .0, sum = .0;
  for (uint32_t k = 0; k < _k; ++k) {
    s += _Epi[k] * _Epi[k] * (y == 0 ? zeros[k]: ones[k]);
    sum += _Epi[k] * _Epi[k];
  };
  s += (1 - sum) * (y == 0 ? zeros[_k]: ones[_k]);

  //assert(s > .0);
  if (s < 1e-30)
    s = 1e-30;
  return log(s);
}

inline double
SBM::edge_likelihood2(uint32_t p, uint32_t q, yval_t y)
{
  const double ** const phid = _phi.const_data();
  Array ones(_K);
  Array zeros(_K);

  for (uint32_t i = 0; i < _k+1; ++i) {
    double u = estimate_bernoulli_rate(i);
    ones[i] = gsl_ran_bernoulli_pdf(1, u);
    zeros[i] = gsl_ran_bernoulli_pdf(0, u);
  }

  double s = .0, sum = .0;
  for (uint32_t k = 0; k < _k; ++k) {
    s += phid[p][k] * phid[q][k] * (y == 0 ? zeros[k]: ones[k]);
    sum += phid[p][k] * phid[q][k];
  };
  s += (1 - sum) * (y == 0 ? zeros[_k]: ones[_k]);

  //assert(s > .0);
  if (s < 1e-30)
    s = 1e-30;
  return log(s);
}

inline bool
SBM::edge_ok(const Edge &e) const
{
  if (e.first == e.second)
    return false;
  
  const SampleMap::const_iterator u = _heldout_map.find(e);
  if (u != _heldout_map.end())
    return false;

  const SampleMap::const_iterator w = _validation_map.find(e);
  if (w != _validation_map.end())
    return false;
  
  const SampleMap::const_iterator v = _precision_map.find(e);
  if (v != _precision_map.end())
    return false;
  
  return true;
}
 
inline void
SBM::get_random_edge(bool link, Edge &e) const
{
  if (!link) {
    do {
      e.first = gsl_rng_uniform_int(_r, _n);
      e.second = gsl_rng_uniform_int(_r, _n);
      Network::order_edge(_env, e);
      assert(e.first == e.second || Network::check_edge_order(e));
    } while (!edge_ok(e));
  } else {
    do {
      const EdgeList &edges = _network.edges();
      int m  = gsl_rng_uniform_int(_r, _network.ones());
      e = edges[m];
      // already ordered 
      assert(Network::check_edge_order(e));
    } while (!edge_ok(e));
  }
}

inline void
SBM::mark_seen(uint32_t a, uint32_t b)
{
#ifndef SPARSE_NETWORK
  yval_t **yd = _network.y().data();
  yd[a][b] = yd[a][b] | 0x80;
  yd[b][a] = yd[b][a] | 0x80;
#endif
}

inline yval_t
SBM::get_y(uint32_t p, uint32_t q)
{
  return _network.y(p,q);
}

// todo: set for SBM
inline uint32_t
SBM::precision_ones() const
{
  return _env.n > 100000 ? 1000 : 10; // (uint64_t)(_network.ones() * _env.precision_ratio);
}

inline uint32_t
SBM::precision_zeros() const
{
  return _env.n > 100000 ? 1000000 : 100; // (uint64_t)((_total_pairs - _network.ones()) * _env.precision_ratio);
}

#endif

