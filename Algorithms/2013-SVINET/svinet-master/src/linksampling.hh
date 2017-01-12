#ifndef LINKSAMPLING_HH
#define LINKSAMPLING_HH

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

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_psi.h>
#include <gsl/gsl_sf.h>

class LinkSampling {
public:
  LinkSampling(Env &env, Network &network);
  ~LinkSampling();

  void infer();
  void save_model();

  static void set_dir_exp(const Matrix &u, Matrix &exp);
  static void set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp);

private:
  void init_validation();
  void load_validation();
  void load_test();
  void write_groups();

  void set_test_sample(int s1);
  void set_validation_sample(int sz);

  void set_precision_biased_sample(int s1);
  void set_precision_uniform_sample(int s1);

  void write_communities(MapVec &communities, string name);
  void auc();
  void biased_auc();
  void uniform_auc();

  void check_and_set_converged(uint32_t p);
  void prune();
  void assign_training_links();
  void compute_mean_indicators();
  void clear();

  void validation_likelihood(double &a, double &a0, double &a1);
  void precision_likelihood();

  int load_model();
  void load_test_sets();
  void get_random_edge(bool link, Edge &e) const;
  void do_on_stop();
  void compute_test_likelihood();
  void test_likelihood(const SampleMap &m, FILE *outf);

  void init_gamma();
  void init_gamma2();
  void init_gamma_external();
  int init_lambda();

  yval_t get_y(uint32_t p, uint32_t q);
  string edgelist_s(EdgeList &elist);
  uint32_t duration() const;

  double edge_likelihood(uint32_t p, uint32_t q, yval_t y) const;
  double link_prob(uint32_t p, uint32_t q) const;
  double estimate_bernoulli_rate(uint32_t k) const;
  void estimate_beta(Array &beta) const;
  void estimate_pi(uint32_t p, Array &pi_p) const;
  void log_communities();
  void gml(uint32_t cid, const vector<uint32_t> &ids);
  bool edge_ok(const Edge &e) const;

  void get_Epi(uint32_t n, Array &Epi);
  uint32_t most_likely_group(uint32_t p);

  Env &_env;
  Network &_network;

  SampleMap _sample_map;
  SampleMap _precision_map;

  SampleMap _validation_map;
  SampleMap _test_map;

  SampleMap _uniform_map;
  SampleMap _biased_map;

  MapVec _communities;
  uint32_t _n;
  uint32_t _k;
  uint32_t _t;
  uint32_t _s;
  uint32_t _iter;
  Array _alpha;
  Array _beta;
  Matrix _eta;

  double _ones_prob;
  double _zeros_prob;
  EdgeList _test_pairs;
  EdgeList _precision_pairs;
  EdgeList _validation_pairs;
  EdgeList _uniform_pairs;
  EdgeList _biased_pairs;
  gsl_rng *_r;

  Matrix _gamma;
  Matrix _lambda;
  Matrix _gammanext;
  Matrix _lambdanext;

  double _tau0;
  double _kappa;
  double _nodetau0;
  double _nodekappa;
  
  double _rhot;
  Array _noderhot;

  time_t _start_time;
  struct timeval _last_iter;
  FILE *_lf;

  Matrix _Elogpi;
  Matrix _Elogbeta;
  Array _Elogf;

  FILE *_hf;
  FILE *_vf;
  FILE *_tf;
  FILE *_hef;
  FILE *_pef;
  FILE *_vef;
  FILE *_tef;

  double _total_pairs;
  double _max_t, _max_h, _max_v, _prev_h, _prev_w, _prev_t;
  mutable uint32_t _nh, _nt;
  bool _training_done;

  Matrix _links;
  uArray _converged;
  uArray _active_comms;
  StaticSparseMatrix16 _active_k;
  Matrix _mphi;
  Matrix _fmap;
  Array _s1, _s2, _s3, _sum;
  uint32_t _nlinks;
  Array _training_links;
  bool _annealing_phase;
};

inline uint32_t
LinkSampling::duration() const
{
  time_t t = time(0);
  return t - _start_time;
}

inline void
LinkSampling::set_dir_exp(const Matrix &u, Matrix &exp)
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
LinkSampling::set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp)
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
LinkSampling::estimate_pi(uint32_t p, Array &pi_p) const
{
  const double ** const gd = _gamma.data();
  double s = .0;
  for (uint32_t k = 0; k < _k; ++k)
    s += gd[p][k];
  assert(s);
  for (uint32_t k = 0; k < _k; ++k)
    pi_p[k] = gd[p][k] / s;
}

inline double
LinkSampling::estimate_bernoulli_rate(uint32_t k) const
{
  const double ** const ld = _lambda.data();
  double s = .0;
  for (uint32_t t = 0; t < _t; ++t)
    s += ld[k][t];
  assert(s);
  return ld[k][0] / s;
}

inline void
LinkSampling::estimate_beta(Array &beta) const
{
  const double ** const ld = _lambda.const_data();
  for (uint32_t k = 0; k < _k; ++k) {
    double s = .0;
    for (uint32_t t = 0; t < _t; ++t)
      s += ld[k][t];
    beta[k] = ld[k][0] / s;
  }
}

inline double
LinkSampling::link_prob(uint32_t p, uint32_t q) const
{
  Array pi_p(_k);
  Array pi_q(_k);
  estimate_pi(p, pi_p);
  estimate_pi(q, pi_q);

  Array ones(_k);
  for (uint32_t i = 0; i < _k; ++i) {
    double u = estimate_bernoulli_rate(i);
    ones[i] = gsl_ran_bernoulli_pdf(1, u);
  }
  double s = .0;
  for (uint32_t z = 0; z < _k; ++z)
    s += pi_p[z] * pi_q[z] * ones[z];
  return s;
}

inline double
LinkSampling::edge_likelihood(uint32_t p, uint32_t q, yval_t y) const
{
  Array pi_p(_k);
  Array pi_q(_k);
  estimate_pi(p, pi_p);
  estimate_pi(q, pi_q);

  debug("estimated pi (%d) = %s\n", p, pi_p.s().c_str());
  debug("estimated pi (%d) = %s\n", q, pi_q.s().c_str());
  
  double s = .0;
  if (y == 1) {
    for (uint32_t z = 0; z < _k; ++z) {
      double brate = estimate_bernoulli_rate(z);
      debug("(%d:%d):1 estimated rate = %f, %f\n", p, q, brate,
	    gsl_ran_bernoulli_pdf(y, brate));
      s += pi_p[z] * pi_q[z] * gsl_ran_bernoulli_pdf(y, brate);
    }
  } else { // y == 0
    for (uint32_t z_p = 0; z_p < _k; ++z_p)
      for (uint32_t z_q = 0; z_q < _k; ++z_q) {
	double brate = (z_p == z_q) ? 
	  estimate_bernoulli_rate(z_p) : _env.epsilon;
	s += pi_p[z_p] * pi_q[z_q] * gsl_ran_bernoulli_pdf(y, brate);
	debug("(%d:%d):0 estimated rate = %f, %f\n", p, q, 
	      brate, gsl_ran_bernoulli_pdf(y, brate));
      }
  }
  //assert(s > .0);
  if (s < 1e-30)
    s = 1e-30;
  debug("s = %f\n", s);
  return log(s);
}

// assumes edge is an ordered pair: (a,b) s.t. a < b
// call Network::order_edge() before invoking edge_ok
inline bool
LinkSampling::edge_ok(const Edge &e) const
{
  if (e.first == e.second)
    return false;
  
  const SampleMap::const_iterator u = _test_map.find(e);
  if (u != _test_map.end())
    return false;
  
  const SampleMap::const_iterator w = _validation_map.find(e);
  if (w != _validation_map.end())
    return false;

  if (_env.create_test_precision_sets) {
    const SampleMap::const_iterator w = _precision_map.find(e);
    if (w != _precision_map.end()) 
      return false;
  }

  if (_env.load_test_sets) {
    const SampleMap::const_iterator u1 = _uniform_map.find(e);
    if (u1 != _uniform_map.end()) 
      return false;    
    const SampleMap::const_iterator b1 = _biased_map.find(e);
    if (b1 != _biased_map.end()) 
      return false;    
  }

  return true;
}

inline void
LinkSampling::get_random_edge(bool link, Edge &e) const
{
  if (!link) {
    e.first = 0;
    e.second = 0;

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
      assert(Network::check_edge_order(e));
    } while (!edge_ok(e));
  }
}

inline yval_t
LinkSampling::get_y(uint32_t p, uint32_t q)
{
  return _network.y(p,q);
}

#endif

