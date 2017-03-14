#ifndef FASTAMM2_HH
#define FASTAMM2_HH

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

//#define TRAINING_SAMPLE 1
#define COMPUTE_GROUPS 1
//#define EGO_NETWORK 1

class PhiCompute {
public:
  PhiCompute(const Env &env, gsl_rng **r, 
	   const uint32_t &iter,
	  uint32_t n, uint32_t k, uint32_t t,
	  uint32_t p, uint32_t q, yval_t y,
	  const Matrix &Elogpi, 
	  const Matrix &Elogbeta,
	  Array &Elogf, bool phifix = false)
    : _env(env), _r(r), _iter(iter),
      _n(n), _k(k), _t(t), 
      _p(p), _q(q), _y(y),
      _Elogpi(Elogpi), _Elogbeta(Elogbeta),
      _Elogf(Elogf),
      _v1(_k), _v2(_k),
      _phi1(_k), _phi2(_k),
      _phinext1(_k), _phinext2(_k),
      _phi1old(_k), _phi2old(_k),
      _phifix(phifix) { }
  ~PhiCompute() { }

  static void static_initialize(uint32_t n, uint32_t k);
  static void static_uninitialize();

  void reset(uint32_t p, uint32_t q, yval_t y, bool phifix=false);

  const Array& phi1() const { return _phi1; }
  const Array& phi2() const { return _phi2; }

  Array& mutable_phi1()  { return _phi1; }
  Array& mutable_phi2()  { return _phi2; }

  uint32_t iter() const { return _iter; }

  void update_phis_until_conv();
  void update_phis(bool is_phi1);
  static void compute_Elogf(uint32_t p, uint32_t q, yval_t y,
			    uint32_t K, uint32_t T,
			    const Matrix &Elogbeta, Array &Elogf);

private:
  const Env &_env;
  gsl_rng **_r;
  const uint32_t &_iter;
  
  uint32_t _n;
  uint32_t _k;
  uint32_t _t;

  uint32_t _p;
  uint32_t _q;
  yval_t _y;
  
  const Matrix &_Elogpi;
  const Matrix &_Elogbeta;
  Array &_Elogf;

  Array _v1;
  Array _v2;
  
  Array _phi1;
  Array _phi2;
  Array _phinext1;
  Array _phinext2;
  Array _phi1old;
  Array _phi2old;
  bool _phifix;
};

inline void
PhiCompute::reset(uint32_t p, uint32_t q, yval_t y, bool phifix)
{
  _p = p;
  _q = q;
  _y = y;
  _v1.zero();
  _v2.zero();
  _phifix = phifix;
}

inline void
PhiCompute::update_phis(bool is_phi1)
{
  Array &b = is_phi1 ? _phi2 : _phi1;
  Array &anext = is_phi1 ? _phinext1 : _phinext2;
  uint32_t c = is_phi1 ? _p : _q;
  
  for (uint32_t k = 0; k < _k; ++k) {
    double u = .0;
    if (_y == 1)
      u = (1 - b[k]) * _env.logepsilon;
    const double ** const elogpid = _Elogpi.const_data();
    anext[k] = elogpid[c][k] + (_Elogf[k] * b[k]) + u;
  }
  anext.lognormalize();

  //double s = .0;
  //for (uint32_t i = 0; i < _k; ++i) {
  //anext[i] = exp(anext[i]);
  //s += anext[i];
  //}
  //assert(s > .0);
  //for (uint32_t i = 0; i < _k; ++i)
  //anext[i] = anext[i] / s;
  
  if (_phifix) {
    if (is_phi1) {
      _phi1.copy_from(_phinext1);
    } else
      _phi2.copy_from(_phinext2);
  }
}

inline void
PhiCompute::compute_Elogf(uint32_t p, uint32_t q, yval_t y,
		       uint32_t K, uint32_t T,
		       const Matrix &Elogbeta, Array &Elogf)
{
  Elogf.zero();
  const double ** const elogbetad = Elogbeta.data();
  for (uint32_t k = 0; k < K; ++k)
    for (uint32_t t = 0; t < T; ++t)
      Elogf[k] += elogbetad[k][t] * (t == 0 ? y : (1-y));
}

inline void
PhiCompute::update_phis_until_conv()
{
  double u = 1./_k;
  _phi1.set_elements(u);
  _phi2.set_elements(u);

  //for (uint32_t i = 0; i < _k; ++i) {
  //_phi1[i] = gsl_ran_gamma(*_r, 100, 1./100);
  //_phi2[i] = gsl_ran_gamma(*_r, 100, 1./100);
  //}
  //_phi1.scale((double)1.0 / _phi1.sum());
  //_phi2.scale((double)1.0 / _phi2.sum());

  compute_Elogf(_p,_q,_y,_k,_t,_Elogbeta,_Elogf);

  _phi1old.zero();
  _phi2old.zero();

  for (uint32_t i = 0; i < _env.online_iterations; ++i) {
    if (_phifix || i % 2 == 0) {
      _phi1old.copy_from(_phi1);
      _phi2old.copy_from(_phi2);
      debug("_phi1old = %s", _phi1old.s().c_str());
      debug("_phi2old = %s", _phi2old.s().c_str());
    }
    update_phis(true);
    update_phis(false);

    debug("_phinext1 = %s", _phinext1.s().c_str());
    debug("_phinext2 = %s", _phinext2.s().c_str());
    
    //_v1.zero();
    //_v2.zero();
    sub(_phinext1,_phi1old,_v1);
    sub(_phinext2,_phi2old,_v2);

    debug("_v1 = %s", _v1.s().c_str());
    debug("_v2 = %s", _v2.s().c_str());

    _phi1.copy_from(_phinext1);
    _phi2.copy_from(_phinext2);

    if (!_phifix && i % 2 == 0)
      continue;
    
    if (_v1.abs().mean() < _env.meanchangethresh and
	_v2.abs().mean() < _env.meanchangethresh) {
      debug("v1 mean = %f", _v1.abs().mean());
      debug("v2 mean = %f", _v2.abs().mean());
      //printf("breaking at i = %d\n", i);
      break;
    }
  }

  //if (_y == 1) {
  //  printf("%s\n", _phi1.s().c_str());
  //  printf("%s\n", _phi2.s().c_str());
  //}
}

class FastAMM2 {
public:
  FastAMM2(Env &env, Network &network);
  ~FastAMM2();

  void infer();
  void save_model();

  void set_dir_exp(const Matrix &u, Matrix &exp);
  void set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp);

private:
  void init_heldout();
  void load_heldout();
  void set_heldout_sample(int sz);
  void set_validation_sample(int sz);
  void set_precision_sample();
  void set_training_sample(int sz);

  double heldout_likelihood();
  void compute_precision(uint32_t &c10, uint32_t &c100, uint32_t &c1000);
  void compute_adamic_adar_score();
  double training_likelihood();
  double validation_likelihood();

  int load_model();
  void get_random_edge(bool link, Edge &e) const;
  void init_gamma();
  int init_lambda();
  void opt_process(NodeMap &v, FreqMap &counts);
  void opt_process_noninf(NodeMap &nodes, FreqMap &counts);

  void update_gmap(const double **gdt, uint32_t a, double rho, double scale);
  
  yval_t get_y(uint32_t p, uint32_t q);
  uint32_t precision_ones() const;
  uint32_t precision_zeros() const;
  void mark_seen(uint32_t a, uint32_t b);

  string edgelist_s(EdgeList &elist);
  uint32_t duration() const;
  
  double approx_log_likelihood();
  void moving_heldout_likelihood(EdgeList &sample) const;

  double edge_likelihood(uint32_t p, uint32_t q, yval_t y) const;
  double estimate_bernoulli_rate(uint32_t k) const;

  void estimate_beta(Array &beta) const;
  void estimate_pi(uint32_t p, Array &pi_p) const;

#ifdef COMPUTE_GROUPS
  void estimate_all_pi();
  void estimate_pi(const vector<uint32_t> &nodes);
  void compute_and_log_groups();
#endif

  void compute_stats();
  bool edge_ok(const Edge &e) const;

  void set_iter(uint32_t a, uint32_t iter);
  uint32_t get_iter(uint32_t a);
  void refresh_all();
  void refresh(uint32_t a);

  void add_to_community_map(uint32_t k, uint32_t p);
  void shuffle_nodes();

#if 0
  void get_similar_nodes(uint32_t a, 
			 vector<uint32_t> &neighbors);
  void get_similar_nodes2(uint32_t a, 
			 vector<uint32_t> &neighbors);
#endif
  void get_top_p_communities(uint32_t a, uint32_t p,
			     vector<uint32_t> &u);
#ifdef EGO_NETWORK
  void get_Epi(uint32_t n, Array &Epi);
  void write_node_gml(FILE *f, uint32_t a, uint32_t max_k);
  void write_edge_gml(FILE *f, uint32_t a, 
		      uint32_t b, uint32_t color, double weight);
  void ego(const uArray &nodes, uint32_t M);
  void group_stats(const uArray &nodes, uint32_t M);

  double bridgeness(uint32_t a);
  void explore(FILE *f, bool root, uint32_t parent, 
	       uint32_t a, NodeMap &uniq, BoolMap &tc, 
	       uint32_t top_k, uint32_t depth);
  uint32_t most_likely_group(uint32_t p);
#endif

  Env &_env;
  Network &_network;

  SampleMap _heldout_map;
  SampleMap _validation_map;
  SampleMap _precision_map;
  SampleMap _training_map;

  SampleMap _ego_smap;
  MapVec _comm_subgraph;
  SampleMap _node_stale;
  CountMap _node_ts;
  CountMap _cmap;
  NodeValMap _iter_map;
  MapVec _community_map;
  MapVec _communities;
  MapVec _memberships;
  NodeValMap _mcount;

  uint64_t _n;
  uint32_t _k;
  uint32_t _t;
  uint32_t _s;
  uint64_t _m;
  uint32_t _iter;
  uint32_t _lambda_start_iter;
  Array _alpha;

  yval_t _family;
  uint32_t _prev_mbsize0;
  uint32_t _prev_mbsize1;
  
  Matrix _eta;

  double _ones_prob;
  double _zeros_prob;
  uint32_t _hitcurve_id;
  double _inf_epsilon;
  double _link_thresh;
  uint64_t _total_pairs_sampled;

  EdgeList _heldout_pairs;
  EdgeList _validation_pairs;
  EdgeList _precision_pairs;
  EdgeList _training_pairs;
  gsl_rng *_r;

  Matrix _gamma;
  Matrix _lambda;

  double _tau0;
  double _kappa;
  double _nodetau0;
  double _nodekappa;
  
  double _rhot;
  Array _noderhot;
  Array _nodec;
  Array _nodeupdatec;
  Array _nodeupdate_rn;

  time_t _start_time;
  struct timeval _last_iter;
  FILE *_lf;

  Matrix _Elogpi;
  Matrix _Elogbeta;

#ifdef COMPUTE_GROUPS
  Matrix _Epi;
#endif

  Matrix _gammat;
  Matrix _lambdat;
  Array _count;

  Array _Elogf;
  FILE *_statsf;
  FILE *_cf;
  FILE *_hf;
  FILE *_vf;
  FILE *_pf;
  FILE *_hcf;
  FILE *_tf;
  FILE *_trf;
  FILE *_mf;
  FILE *_cmapf;
  FILE *_hef;
  FILE *_vef;
  FILE *_pef;
  FILE *_tef;

  uint64_t _total_pairs;
  PhiCompute _pcomp;
  uArray _shuffled_nodes;

  bool _delaylearn_reported;
  double _max_t, _max_h, _max_v, _prev_h, _prev_w, _prev_t;
  mutable uint32_t _nh, _nt;
  bool _training_done;

  // random node
  uint32_t _start_node;
  uArray _neighbors;

  static int ea;
  static int eb;
};
inline uint32_t
FastAMM2::duration() const
{
  time_t t = time(0);
  return t - _start_time;
}

inline void
FastAMM2::set_dir_exp(const Matrix &u, Matrix &exp)
{
  const double ** const d = u.data();
  double **e = exp.data();
  for (uint32_t i = 0; i < u.m(); ++i) {
    // psi(e[i][j]) - psi(sum(e[i]))
    double s = .0;
    for (uint32_t j = 0; j < u.n(); ++j) 
      s += d[i][j];
    fflush(stdout);
    assert (s > .0);
    double psi_sum = gsl_sf_psi(s);
    for (uint32_t j = 0; j < u.n(); ++j) {
      double tt = .0;
      if (d[i][j] <= .0)
	tt = _env.alpha;
      else
	tt = d[i][j];
      e[i][j] = gsl_sf_psi(tt) - psi_sum;
    }
  }
}

inline void
FastAMM2::set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp)
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
FastAMM2::estimate_pi(uint32_t p, Array &pi_p) const
{
  const double ** const gd = _gamma.data();
  double s = .0;
  for (uint32_t k = 0; k < _k; ++k)
    s += gd[p][k];
  assert(s);
  for (uint32_t k = 0; k < _k; ++k)
    pi_p[k] = gd[p][k] / s;
}

#ifdef COMPUTE_GROUPS

inline void
FastAMM2::estimate_all_pi()
{
  const double ** const gd = _gamma.const_data();
  double **epid = _Epi.data();
  for (uint32_t n = 0; n < _n; ++n) {
    double s = .0;
    for (uint32_t k = 0; k < _k; ++k)
      s += gd[n][k];
    assert(s);
    for (uint32_t k = 0; k < _k; ++k)
      epid[n][k] = gd[n][k] / s;
  }
}

inline void
FastAMM2::estimate_pi(const vector<uint32_t> &nodes)
{
  const double ** const gd = _gamma.const_data();
  double **epid = _Epi.data();
  for (uint32_t i = 0; i < nodes.size(); ++i) {
    uint32_t n = nodes[i];
    double s = .0;
    for (uint32_t k = 0; k < _k; ++k)
      s += gd[n][k];
    assert(s);
    for (uint32_t k = 0; k < _k; ++k)
      epid[n][k] = gd[n][k] / s;
  }
}

#endif

#ifdef EGO_NETWORK

inline void
FastAMM2::get_Epi(uint32_t n, Array &Epi)
{
  const double ** const gd = _gamma.const_data();
  double *epid = Epi.data();
  double s = .0;
  for (uint32_t k = 0; k < _k; ++k)
    s += gd[n][k];
  assert(s);
  for (uint32_t k = 0; k < _k; ++k)
    epid[k] = gd[n][k] / s;
}

#endif

inline double
FastAMM2::estimate_bernoulli_rate(uint32_t k) const
{
  const double ** const ld = _lambda.data();
  double s = .0;
  for (uint32_t t = 0; t < _t; ++t)
    s += ld[k][t];
  assert(s);
  return ld[k][0] / s;
}

inline void
FastAMM2::estimate_beta(Array &beta) const
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
FastAMM2::edge_likelihood(uint32_t p, uint32_t q, yval_t y) const
{
  Array pi_p(_k);
  Array pi_q(_k);
  estimate_pi(p, pi_p);
  estimate_pi(q, pi_q);

  double v = gsl_ran_bernoulli_pdf(0, _env.epsilon);

  Array ones(_k);
  Array zeros(_k);

  for (uint32_t i = 0; i < _k; ++i) {
    double u = estimate_bernoulli_rate(i);
    ones[i] = gsl_ran_bernoulli_pdf(1, u);
    zeros[i] = gsl_ran_bernoulli_pdf(0, u);
  }

  debug("estimated pi (%d) = %s\n", p, pi_p.s().c_str());
  debug("estimated pi (%d) = %s\n", q, pi_q.s().c_str());
  
  double s = .0;
  if (y == 1) {
    for (uint32_t z = 0; z < _k; ++z)
      s += pi_p[z] * pi_q[z] * ones[z];
  } else { // y == 0
    double sum = .0;
    for (uint32_t z = 0; z < _k; ++z) {
      s += pi_p[z] * pi_q[z] * zeros[z];
      sum += pi_p[z] * pi_q[z];
    }
    s += (1 - sum) * v;
  }
  //assert(s > .0);
  if (s < 1e-30)
    s = 1e-30;
  return log(s);
}

// assumes edge is an ordered pair: (a,b) s.t. a < b
// call Network::order_edge() before invoking edge_ok
inline bool
FastAMM2::edge_ok(const Edge &e) const
{
  if (e.first == e.second)
    return false;
  
  const SampleMap::const_iterator u = _heldout_map.find(e);
  if (u != _heldout_map.end())
    return false;

  if (!_env.single_heldout_set) {
    const SampleMap::const_iterator w = _validation_map.find(e);
    if (w != _validation_map.end())
      return false;
  }

  return true;
}
 
inline void
FastAMM2::get_random_edge(bool link, Edge &e) const
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

inline void
FastAMM2::set_iter(uint32_t a, uint32_t iter)
{
  _iter_map[a] = iter;
}

inline uint32_t
FastAMM2::get_iter(uint32_t a)
{
  NodeValMap::const_iterator i = _iter_map.find(a);
  return (i != _iter_map.end()) ? i->second : 0;
}

inline void
FastAMM2::refresh_all()
{
  for (uint32_t i = 0; i < _n; ++i) {
    uint32_t j = get_iter(i);
    if (j != _iter)
      refresh(i);
  }
}

inline void
FastAMM2::add_to_community_map(uint32_t k, uint32_t p)
{
  vector<uint32_t> &v = _community_map[k];
  for (uint32_t i = 0; i < v.size(); ++i)
    if (v[i] == p)
      return;
  v.push_back(p);
}

#if 0

inline void
FastAMM2::get_similar_nodes2(uint32_t n, 
			    vector<uint32_t> &neighbors)
{
  Array a(_k);
  _Epi.slice(0, n, a);

  for (uint32_t i = 0; i < _n; ++i) {
    if (i == n)
      continue;

    yval_t y = get_y(n,i);
    if (y == 1)
      continue;

    Array b(_k);
    _Epi.slice(0, i, b);
      
    if (dot(a,b) >= _env.infthresh)
      neighbors.push_back(i);
  }
  fflush(stdout);
}

inline void
FastAMM2::get_similar_nodes(uint32_t a, 
			   vector<uint32_t> &neighbors)
{
  // choose nodes in the top p communities of node a where p << k
  uint32_t p = 2;
  vector<uint32_t> u;
  get_top_p_communities(a, p, u);

#if 0
  printf("[%d] top communities:", a);
  for (uint32_t i = 0; i < u.size(); ++i)
    printf("%d ", u[i]);
#endif

  for (uint32_t i = 0; i < u.size(); ++i) {
    //printf("%d ", u[i]);
    vector<uint32_t> &v = _community_map[u[i]];
    for (uint32_t j = 0; j < v.size(); ++j) {
      yval_t y = get_y(a, v[j]);
      if (a != v[j] && y == 0)
	neighbors.push_back(v[j]);
    }
  }
  //printf("\n");
}

#endif


inline void
FastAMM2::get_top_p_communities(uint32_t a, uint32_t p,
				 vector<uint32_t> &u)
{
  Array g(_k);
  _gamma.slice(0, a, g);
  
  D1Array<KV> v(_k);
  for (uint32_t k = 0; k < _k; ++k) {
    if (g[k] > _alpha[k]) // alpha symmetric
      v[k] = KV(k, g[k]);
  }
  v.sort_by_value();

  //printf("node:%d, sorted top communities:%s\n", a, v.s().c_str());
  double s = .0;
  for (uint32_t k = 0; k < p; k++) {
    u.push_back(v[k].first);
    s += v[k].second;
  }
  //double t = g.sum();
}


inline yval_t
FastAMM2::get_y(uint32_t p, uint32_t q)
{
  return _network.y(p,q);
}

inline uint32_t
FastAMM2::precision_ones() const
{
  return 1000; // (uint64_t)(_network.ones() * _env.precision_ratio);
}

inline uint32_t
FastAMM2::precision_zeros() const
{
  return 1000000; // (uint64_t)((_total_pairs - _network.ones()) * _env.precision_ratio);
}

//bool operator==(const Edge &a, const Edge &b);
// inline bool
// operator==(const Edge &a, const Edge &b)
// {
//   if ((a.first == b.first && a.second == b.second) ||
//       //      (a.second == b.first && a.first == b.second))
//     return true;
//   return false;
// }

#endif

