#ifndef FASTINIT_HH
#define FASTINIT_HH

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

//#define GLOBALPHIS 1
//#define ALT_PAIRS 1
//#define MRSTATS 1

//#define TRAINING_SAMPLE 1

class FastInit {
public:
  FastInit(Env &env, Network &network, uint32_t max_deg);
  ~FastInit();

  void infer();
  void batch_infer();
  void save_model();

  static void set_dir_exp(const Matrix &u, Matrix &exp);
  static void set_dir_exp(const MatrixKV &u, Matrix &exp, double alpha);
  static void set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp);

private:
  void init_heldout();
  void set_heldout_sample(int sz);
  void set_validation_sample(int sz);
  void update_all_nodes();

#ifdef TRAINING_SAMPLE
  void set_training_sample(int sz);
  void training_likelihood(double &a, double &a0, double &a1);
#endif

  void heldout_likelihood();
  int load_model();
  void get_random_edge(bool link, Edge &e) const;
  void do_on_stop();

  void init_gamma();

  yval_t get_y(uint32_t p, uint32_t q);
  void mark_seen(uint32_t a, uint32_t b);
  string edgelist_s(EdgeList &elist);
  uint32_t duration() const;

  double approx_log_likelihood();
  double edge_likelihood(uint32_t p, uint32_t q, yval_t y) const;
  double training_likelihood() const;
  double estimate_bernoulli_rate(uint32_t k) const;
  void estimate_beta(Array &beta) const;
  void estimate_pi(uint32_t p, KVArray &pi_p) const;
  void estimate_all_pi();
  void set_gamma();
  void compute_and_log_groups();

  bool edge_ok(const Edge &e) const;
  void get_Epi(uint32_t n, Array &Epi);
  uint32_t most_likely_group(uint32_t p);

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
  uint32_t _max_deg;

  SampleMap _sample_map;
  SampleMap _heldout_map;
  SampleMap _validation_map;
  SampleMap _training_map;
  SampleMap _node_stale;
  CountMap _node_ts;
  CountMap _cmap;
  NodeValMap _iter_map;
  MapVec _community_map;

  EdgeList _heldout_pairs;  

  MapVec _communities;
  MapVec _memberships;
  NodeValMap _mcount;

  uint32_t _n;
  uint32_t _k;
  uint32_t _t;
  uint32_t _s;
  uint32_t _iter;
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

  MatrixKV _gamma;
  D1Array<FreqMap> _gammanext;

  uArray _maxgamma;

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

  MatrixKV _Epi;

  Array _count;

  FreqMap _links;
  FreqMap _nodes;
  MapVec _nodesv;
  
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
  FILE *_uf;

  double _total_pairs;

  // threads
  bool _delaylearn_reported;
  double _max_t, _max_h, _max_v, _prev_h, _prev_w, _prev_t;
  mutable uint32_t _nh, _nt;
  bool _training_done;

  // bfs
  std::queue<uint32_t> _q;
  std::map<uint32_t, bool> _visited;

  // random node
  uint32_t _start_node;
  uint32_t _n1, _n2, _n3;

  std::map<uint32_t, uint32_t> _conv_time;
  uint32_t _cn;
  uint32_t _skipped;
  uArray _neighbors;
  uint32_t _unlikely;
  uint32_t _prev_unlikely;
};

inline uint32_t
FastInit::duration() const
{
  time_t t = time(0);
  return t - _start_time;
}

inline void
FastInit::set_dir_exp(const Matrix &u, Matrix &exp)
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
      double x = d[i][j];
      if (d[i][j] < 1e-30)
	x = 1e-30;
      e[i][j] = gsl_sf_psi(x) - psi_sum;
    }
  }
}



inline void
FastInit::set_dir_exp(const MatrixKV &u, Matrix &exp, double alpha)
{
  const KV ** const d = u.data();
  double **e = exp.data();
  for (uint32_t i = 0; i < u.m(); ++i) {
    // psi(e[i][j]) - psi(sum(e[i]))
    double s = .0;
    for (uint32_t j = 0; j < u.n(); ++j) 
      s += d[i][j].second;
    //debug("set_dir_exp: s = %f\n",s);
    //
    // NOTE: here we assume that only the top entries are in MatrixKV u
    // the rest are _env.alpha
    //
    s += (u.n() - u.m()) * alpha;
    double psi_sum = gsl_sf_psi(s);
    for (uint32_t j = 0; j < u.n(); ++j) {
      double x = d[i][j].second;
      if (x < 1e-30)
	x = 1e-30;
      //
      // XXX: assume that exp is of the same dimension as u
      //
      e[i][j] = gsl_sf_psi(x) - psi_sum;
    }
  }
}

inline void
FastInit::set_dir_exp(uint32_t a, const Matrix &u, Matrix &exp)
{
  const double ** const d = u.data();
  double **e = exp.data();

  double s = .0;
  for (uint32_t j = 0; j < u.n(); ++j) 
    s += d[a][j];
  double psi_sum = gsl_sf_psi(s);
  for (uint32_t j = 0; j < u.n(); ++j)  {
    double x = d[a][j];
    if (d[a][j] < 1e-30)
      x = 1e-30;
    e[a][j] = gsl_sf_psi(x) - psi_sum;
  }
}

inline void
FastInit::estimate_pi(uint32_t p, KVArray &pi_p) const
{
  const KV ** const gd = _gamma.data();
  double s = .0;
  for (uint32_t k = 0; k < _k; ++k)
    s += gd[p][k].second;
  s += (_gamma.m() - _gamma.n()) * _env.alpha;
  assert(s);
  for (uint32_t k = 0; k < _k; ++k) {
    pi_p[k].first = gd[p][k].first;
    pi_p[k].second = gd[p][k].second / s;
  }
}

inline void
FastInit::estimate_all_pi()
{
  const KV ** const gd = _gamma.const_data();
  KV **epid = _Epi.data();
  for (uint32_t n = 0; n < _n; ++n) {
    double s = .0;
    for (uint32_t k = 0; k < _k; ++k)
      s += gd[n][k].second;
    s += (_gamma.m() - _gamma.n()) * _env.alpha;
    assert(s);
    for (uint32_t k = 0; k < _k; ++k) {
      epid[n][k].first = gd[n][k].first;
      epid[n][k].second = gd[n][k].second / s;
    }
  }
}

inline double
FastInit::estimate_bernoulli_rate(uint32_t k) const
{
  return 1.0;
}

inline bool
FastInit::edge_ok(const Edge &e) const
{
  if (e.first == e.second)
    return false;
  
  const SampleMap::const_iterator u = _heldout_map.find(e);
  if (u != _heldout_map.end())
    return false;
  
  return true;
}
 
 
inline void
FastInit::get_random_edge(bool link, Edge &e) const
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
FastInit::set_iter(uint32_t a, uint32_t iter)
{
  _iter_map[a] = iter;
}

inline uint32_t
FastInit::get_iter(uint32_t a)
{
  NodeValMap::const_iterator i = _iter_map.find(a);
  return (i != _iter_map.end()) ? i->second : 0;
}

inline void
FastInit::refresh_all()
{
  for (uint32_t i = 0; i < _n; ++i) {
    uint32_t j = get_iter(i);
    if (j != _iter)
      refresh(i);
  }
}

inline void
FastInit::add_to_community_map(uint32_t k, uint32_t p)
{
  vector<uint32_t> &v = _community_map[k];
  for (uint32_t i = 0; i < v.size(); ++i)
    if (v[i] == p)
      return;
  v.push_back(p);
}

inline void
FastInit::get_similar_nodes(uint32_t a, 
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
FastInit::get_top_p_communities(uint32_t a, uint32_t p,
				vector<uint32_t> &u)
{
  KVArray v(_k);
  _gamma.slice(0, a, v);

  v.sort_by_value();

  printf("node:%d, sorted top communities:%s\n", a, v.s().c_str());
  double s = .0;
  for (uint32_t k = 0; k < p; k++) {
    u.push_back(v[k].first);
    s += v[k].second;
  }
}

inline void
FastInit::mark_seen(uint32_t a, uint32_t b)
{
#ifndef SPARSE_NETWORK
  yval_t **yd = _network.y().data();
  yd[a][b] = yd[a][b] | 0x80;
  yd[b][a] = yd[b][a] | 0x80;
#endif
}

inline yval_t
FastInit::get_y(uint32_t p, uint32_t q)
{
  return _network.y(p,q);
}

#endif
