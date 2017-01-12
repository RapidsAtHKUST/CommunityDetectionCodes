#ifndef MMSBINFER_HH
#define MMSBINFER_HH

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

//#define ALT_PAIRS 1
//#define MRSTATS 1

// bool operator==(const Edge &a, const Edge &b);
class PhiComp {
public:
  PhiComp(const Env &env, const uint32_t &iter,
	  uint32_t n, uint32_t k, uint32_t t,
	  uint32_t p, uint32_t q, yval_t y,
	  const Matrix &Elogpi, 
	  const Matrix &Elogbeta,
	  Array &Elogf, bool phifix = false)
    : _env(env), _iter(iter),
      _n(n), _k(k), _t(t), 
      _p(p), _q(q), _y(y),
      _Elogpi(Elogpi), _Elogbeta(Elogbeta),
      _Elogf(Elogf),
      _v1(_k), _v2(_k),
      _phi1(_k), _phi2(_k),
      _phinext1(_k), _phinext2(_k),
      _phi1old(_k), _phi2old(_k),
      _phifix(phifix) { }
  ~PhiComp() { }

  static void static_initialize(uint32_t n, uint32_t k) { }
  static void static_uninitialize() { }

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
PhiComp::reset(uint32_t p, uint32_t q, yval_t y, bool phifix)
{
  _p = p;
  _q = q;
  _y = y;
  _v1.zero();
  _v2.zero();
  _phifix = phifix;
}

inline void
PhiComp::update_phis(bool is_phi1)
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
  //anext.lognormalize();
  double s = .0;
  for (uint32_t i = 0; i < _k; ++i) {
    anext[i] = exp(anext[i]);
    s += anext[i];
  }
  if (!(s)) {
    printf("%s\n", anext.s().c_str());
    fflush(stdout);
  }
  assert(s > .0);
  for (uint32_t i = 0; i < _k; ++i)
    anext[i] = anext[i] / s;
  
  if (_phifix) {
    if (is_phi1) {
      _phi1.copy_from(_phinext1);
    } else
      _phi2.copy_from(_phinext2);
  }
}

inline void
PhiComp::compute_Elogf(uint32_t p, uint32_t q, yval_t y,
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
PhiComp::update_phis_until_conv()
{
  double u = 1./_k;
  _phi1.set_elements(u);
  _phi2.set_elements(u);

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
      debug("breaking at i = %d\n", i);
      break;
    }
  }
}

struct MRStats  {
  MRStats();
  void zero();
  struct timeval init_tv_before;
  struct timeval init_tv_after;
  struct timeval init_tv_sum;
  uint32_t init_n;

  struct timeval map_tv_before;
  struct timeval map_tv_after;
  struct timeval map_tv_sum;
  uint32_t map_n;

  struct timeval reduce_tv_before;
  struct timeval reduce_tv_after;
  struct timeval reduce_tv_sum;
  uint32_t reduce_n;
};

inline
MRStats::MRStats()
 :init_n(0), map_n(0), reduce_n(0) 
{ 
  zero();
}

inline void
MRStats::zero()
{
  memset(&init_tv_before, 0, sizeof(struct timeval));
  memset(&init_tv_after, 0, sizeof(struct timeval));
  memset(&init_tv_sum, 0, sizeof(struct timeval));

  memset(&map_tv_before, 0, sizeof(struct timeval));
  memset(&map_tv_after, 0, sizeof(struct timeval));
  memset(&map_tv_sum, 0, sizeof(struct timeval));

  memset(&reduce_tv_before, 0, sizeof(struct timeval));
  memset(&reduce_tv_after, 0, sizeof(struct timeval));
  memset(&reduce_tv_sum, 0, sizeof(struct timeval));
}

typedef std::map<uint32_t, EdgeList *> ChunkMap;

class PhiRunner : public Thread {
public:
  PhiRunner(const Env &env, Network &network, 
	    const uint32_t &iter,
	    uint32_t n, uint32_t k, uint32_t t,
	    yval_t family,
	    const Matrix &Elogpi, 
	    const Matrix &Elogbeta,
	    TSQueue<EdgeList> &out_q,
	    TSQueue<pthread_t> &in_q,
	    CondMutex &cm, CondMutex &rest)
    :_env(env),
     _network(network),
     _n(n), _k(k), _t(t),
     _family(family),
     _Elogf(k),
     _pcomp(env, iter, n, k, t, 
	    0, 0, 0,
	    Elogpi, Elogbeta, _Elogf),
     _gammat(n,k), 
     _szgammat(n,k),
     _lambdat(k,t), 
     _count(n),
     _out_q(out_q),
     _in_q(in_q),
     _cm(cm), _rest(rest),
     _done(false),
     _sample(NULL) { }    
  
  ~PhiRunner() { }
  
  int do_work();
  void set_done()   { _done = true; }
  bool done() const { return _done; }
  void reset();

  const EdgeList &sample() const { assert(_sample); return *_sample; }

  const Matrix &gammat() const  { return _gammat; }
  const Matrix &lambdat() const { return _lambdat; }
  const Array &count() const    { return _count; }

  const MRStats *rt() const { return &_rt; }
  
private:
  void process(const EdgeList &sample);

  const Env &_env;
  Network &_network;
  uint32_t _n;
  uint32_t _k;
  uint32_t _t;
  yval_t _family;

  Array _Elogf;
  PhiComp _pcomp;

  Matrix _gammat;
  Matrix _szgammat;
  Matrix _lambdat;
  Array _count;
  
  TSQueue<EdgeList> &_out_q;
  TSQueue<pthread_t> &_in_q;
  CondMutex &_cm;
  CondMutex &_rest;

  bool _done;
  EdgeList *_sample;
  NodeMap _nodemap;

  MRStats _rt;
};
typedef std::map<pthread_t, PhiRunner *> ThreadMap;


inline void
PhiRunner::reset()
{
  _done = false;
  _gammat.zero();
  _lambdat.zero();
  _szgammat.zero();
  _rt.zero();
  if (_sample) {
    delete _sample;
    _sample = NULL;
  }
  if (_env.stratified)
    _family = _family ? 0 : 1;
}

class MMSBInfer {
public:
  MMSBInfer(Env &env, Network &network);
  ~MMSBInfer();

  void infer();
  void batch_infer();
  void save_model();

  static void set_dir_exp(const Matrix &u, Matrix &exp);
  static void set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp);

private:
  void init_heldout();
  void set_heldout_sample(int sz);
  void set_validation_sample(int sz);

#ifdef TRAINING_SAMPLE
  void set_training_sample(int sz);
  void training_likelihood(double &a, double &a0, double &a1);
#endif

  void heldout_likelihood(double &a, double &a0, double &a1);
  void validation_likelihood(double &a, double &a0, double &a1) const;

  void obs_likelihood(double &a, double &a0, double &a1) const;
  int load_model();
  void get_random_edge(Edge &e, bool heldout) const;
  void get_edge(Edge &e, bool heldout) const;
  void get_randomnode_edges(uint32_t node, EdgeList &edges) const;
  void get_randomnode_edges2(uint32_t start_node, EdgeList &edges) const;
  void do_on_stop();

  void hosts_conv();
  void log_mrstats();

  void init_gamma();
  int init_lambda();

#ifdef SPARSE_NETWORK
  void get_bfs_edges(uint32_t start_node, uint32_t limit, EdgeList &edges);
#endif

  void process(const EdgeList &sample);
  void multithreaded_process(const EdgeList &sample);
  void opt_nonuniform_process(const EdgeList &sample);
  void opt_process(const EdgeList &sample);
  void opt_process2(vector<uint32_t> &v);

  yval_t get_y(uint32_t p, uint32_t q);
  void mark_seen(uint32_t a, uint32_t b);
  void get_subsample(EdgeList &edges, uint32_t n);
  string edgelist_s(EdgeList &elist);
  uint32_t duration() const;

  double approx_log_likelihood();
  void moving_heldout_likelihood(EdgeList &sample) const;

  double edge_likelihood(uint32_t p, uint32_t q, yval_t y) const;
  double estimate_bernoulli_rate(uint32_t k) const;
  void estimate_beta(Array &beta) const;
  void estimate_pi(uint32_t p, Array &pi_p) const;
  void estimate_all_pi();
  void compute_and_log_groups();
  void compute_modularity();
  bool edge_ok(const Edge &e, bool heldout) const;
  int start_threads();
  int join_all_threads();
  bool all_threads_done();

  double compute_mean_change_gamma(uint32_t i);
  double compute_mean_change_lambda(uint32_t i);

  void set_iter(uint32_t a, uint32_t iter);
  uint32_t get_iter(uint32_t a);
  void refresh_all();
  void refresh(uint32_t a);
  void add_to_community_map(uint32_t k, uint32_t p);
  void get_similar_nodes(uint32_t a, 
			 vector<uint32_t> &neighbors);
  void get_top_p_communities(uint32_t a, uint32_t p,
			     vector<uint32_t> &u);
  void mark_seen(yval_t **yd, uint32_t a, uint32_t b);

  Env &_env;
  Network &_network;
  SampleMap _sample_map;
  SampleMap _heldout_map;
  SampleMap _validation_map;
  SampleMap _training_map;
  SampleMap _node_stale;
  CountMap _node_ts;
  CountMap _cmap;
  NodeValMap _iter_map;
  MapVec _community_map;

  uint32_t _n;
  uint32_t _k;
  uint32_t _t;
  uint32_t _s;
  uint32_t _iter;
  uint32_t _lambda_start_iter;
  Array _alpha;

  yval_t _family;
  uint32_t _prev_mbsize0;
  uint32_t _prev_mbsize1;
  
  Matrix _eta;

  double _ones_prob;
  double _zeros_prob;
  EdgeList _heldout_edges;
  EdgeList _validation_edges;
  EdgeList _training_edges;
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
  Array _nodec;
  Array _nodeupdatec;
  Array _nodeupdate_rn;

  time_t _start_time;
  struct timeval _last_iter;
  FILE *_lf;

  Matrix _Elogpi;
  Matrix _Elogbeta;
  Matrix _Epi;

  Matrix _gammat;
  Matrix _szgammat;

  Matrix _lambdat;
  Array _count;

  Array _Elogf;
  FILE *_statsf;
  FILE *_cf;
  FILE *_hf;
  FILE *_vf;
  FILE *_tf;
  FILE *_trf;
  FILE *_mf;
  FILE *_cmapf;
  FILE *_hef;
  FILE *_vef;
  FILE *_tef;

#ifdef MRSTATS
  FILE *_mrstatsf;
#endif

  double _total_pairs;
  PhiComp _pcomp;

  // threads
  ThreadMap _thread_map;
  ChunkMap _chunk_map;
  TSQueue<EdgeList> _out_q;
  TSQueue<pthread_t> _in_q;
  uint16_t _nthreads;
  bool _delaylearn_reported;
  double _max_t, _max_h, _max_v, _prev_h, _prev_w, _prev_t;
  mutable uint32_t _nh, _nt;
  bool _training_done;

  CondMutex _cm;
  CondMutex _rest;

  // bfs
  std::queue<uint32_t> _q;
  std::map<uint32_t, bool> _visited;

#ifdef ALT_PAIRS
  std::map<uint32_t, uint32_t> _nodemap;
#endif

  // random node
  uint32_t _start_node;
  uint32_t _n1, _n2, _n3;

  std::map<uint32_t, uint32_t> _conv_time;
  uint32_t _cn;
  uint32_t _skipped;
  uArray _neighbors;

#ifdef MRSTATS
  MRStats _rt;
#endif

  static int ea;
  static int eb;
};

inline uint32_t
MMSBInfer::duration() const
{
  time_t t = time(0);
  return t - _start_time;
}

inline void
MMSBInfer::set_dir_exp(const Matrix &u, Matrix &exp)
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
MMSBInfer::set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp)
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
MMSBInfer::estimate_pi(uint32_t p, Array &pi_p) const
{
  const double ** const gd = _gamma.data();
  double s = .0;
  for (uint32_t k = 0; k < _k; ++k)
    s += gd[p][k];
  assert(s);
  for (uint32_t k = 0; k < _k; ++k)
    pi_p[k] = gd[p][k] / s;
}

inline void
MMSBInfer::estimate_all_pi()
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

inline double
MMSBInfer::estimate_bernoulli_rate(uint32_t k) const
{
  const double ** const ld = _lambda.data();
  double s = .0;
  for (uint32_t t = 0; t < _t; ++t)
    s += ld[k][t];
  assert(s);
  return ld[k][0] / s;
}

inline void
MMSBInfer::estimate_beta(Array &beta) const
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
MMSBInfer::edge_likelihood(uint32_t p, uint32_t q, yval_t y) const
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
  return log(s);
}

inline void
MMSBInfer::get_edge(Edge &e, bool heldout = false) const
{
  do {
    e.first = ea;
    e.second = eb;

    eb++;
    if (eb >= (int)_n) {
      eb = 0;
      ea++;
      if (ea >= (int)_n) 
	ea = 0;
    }
    if (edge_ok(e, heldout))
      break;
  } while (1);
  Network::order_edge(_env, e);
}

inline bool
MMSBInfer::edge_ok(const Edge &e, bool heldout) const
{
  bool ok = true;
  if (e.first != e.second) {
    if (!heldout) {

      const SampleMap::const_iterator u = _heldout_map.find(e);
      if (u != _heldout_map.end()) {
	ok = false;
      }

      const SampleMap::const_iterator v = _validation_map.find(e);
      if (v != _validation_map.end()) {
	ok = false;
      }

#ifndef SPARSE_NETWORK
      const yval_t ** const yd = _network.y().const_data();
      yval_t y = yd[e.first][e.second] & 0x01;
#else
      yval_t y = _network.y(e.first, e.second);
#endif

      if (_env.stratified && y != _family)
	ok = false;
    }
  } else
    ok = false;
  return ok;
}
 
inline void
MMSBInfer::get_random_edge(Edge &e, bool heldout = false) const
{
  if (!_env.stratified || heldout || (_env.stratified && _family == 0)) {
    // stratified is run on very sparse matrices so this while loop is highly
    // unlikely to repeat more than once for the case when 
    // _env.stratified && _family == 0
    do {
      e.first = gsl_rng_uniform_int(_r, _n);
      e.second = gsl_rng_uniform_int(_r, _n);
      Network::order_edge(_env, e);
      assert(e.first == e.second || Network::check_edge_order(e));
    } while (!edge_ok(e, heldout));
  } else { // _env.stratified && _family == 1
    if (_family == 1) {
      do {
	const EdgeList &edges = _network.edges();
	int m  = gsl_rng_uniform_int(_r, _network.ones());
	e = edges[m];
	// already ordered 
	assert(Network::check_edge_order(e));
      } while (!edge_ok(e, heldout));
    } else  {
      fprintf(stderr, "unexpected case in get_random_edge()\n");
      assert(0);
    }
  }
}

inline void
MMSBInfer::set_iter(uint32_t a, uint32_t iter)
{
  _iter_map[a] = iter;
}

inline uint32_t
MMSBInfer::get_iter(uint32_t a)
{
  NodeValMap::const_iterator i = _iter_map.find(a);
  return (i != _iter_map.end()) ? i->second : 0;
}

inline void
MMSBInfer::refresh_all()
{
  for (uint32_t i = 0; i < _n; ++i) {
    uint32_t j = get_iter(i);
    if (j != _iter)
      refresh(i);
  }
}

inline void
MMSBInfer::add_to_community_map(uint32_t k, uint32_t p)
{
  vector<uint32_t> &v = _community_map[k];
  for (uint32_t i = 0; i < v.size(); ++i)
    if (v[i] == p)
      return;
  v.push_back(p);
}

inline void
MMSBInfer::get_similar_nodes(uint32_t a, 
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

inline void
MMSBInfer::get_top_p_communities(uint32_t a, uint32_t p,
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

  printf("node:%d, sorted top communities:%s\n", a, v.s().c_str());
  double s = .0;
  for (uint32_t k = 0; k < p; k++) {
    u.push_back(v[k].first);
    s += v[k].second;
  }
  double t = g.sum();
  printf("----> (%.5f,%.5f)\n", s, t);
}

inline void
MMSBInfer::mark_seen(uint32_t a, uint32_t b)
{
#ifndef SPARSE_NETWORK
  yval_t **yd = _network.y().data();
  yd[a][b] = yd[a][b] | 0x80;
  yd[b][a] = yd[b][a] | 0x80;
#endif
}

inline yval_t
MMSBInfer::get_y(uint32_t p, uint32_t q)
{
  return _network.y(p,q);
}
#endif

