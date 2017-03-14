#ifndef MMSBGEN_HH
#define MMSBGEN_HH

#include <list>
#include <utility>
#include <unistd.h>
#include "mmsbinfer.hh"
#include "community.hh"

class MMSBGen {
public:
  MMSBGen(Env &env, Network &network, bool ppc);
  void gen(double alpha);
  void ppc();
  void get_lc_stats();
  void gml();
  void obs_data();

private:
  int load_model();
  void save_model();
  void compute_and_log_groups() const;
  void bridgeness();
  void compute_modularity();
  void community_degrees();
  
  void process_link_communities();
  void process_link_communities2();
  double edge_likelihood(uint32_t p, uint32_t q, uint32_t color) const;

  uint32_t most_likely_group(uint32_t p);

  void estimate_pi(uint32_t p, Array &pi_p) const;
  void estimate_all();
  void estimate_beta(Array &beta) const;
  void draw_all();
  void draw_and_save(uint32_t id);
  double estimate_bernoulli_rate(uint32_t k) const;
  void get_lc_zscores();
  void reset();

  void draw_membership_indicators(uint32_t p, uint32_t q);
  void ppc_avg_deg() const;
  void ppc_ones() const;
  void ppc_local_clustering_coeff() const;
  void ppc_per_community();
  void lc_current_draw();
  void lc_current_draw2();
  void lc_current_draw_helper(bool obs);
  
  void obs_avg_deg() const;
  void obs_ones() const;
  int setup_dir(string dirname);
  void process(string outfile, string infile, string pattern) const;

  void create_id_dir(uint32_t id);
  string id_dir() const;

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
  Array _beta;

  Matrix _gamma;
  Matrix _lambda;
  FILE *_onesf;
  uint32_t _curr_id;

  uint32_t _total_pairs;

  Matrix _Elogpi;
  Matrix _Elogbeta;
  Array _Elogf;
  PhiComp _pcomp;

  mutable uint32_t _legal;
  mutable uint32_t _illegal;

  std::map<uint32_t, EdgeList> _ppc_link_communities;
  std::map<uint32_t, EdgeList> _obs_link_communities;

  Matrix _lc_ppc_logpe;
  Matrix _lc_obs_logpe;
  Array _zscores_pe;

  Matrix _lc_ppc_size;
  Matrix _lc_obs_size;
  Array _zscores_size;

  D1Array<Community> _communities;

  Matrix _lc_obs_avgdeg;
  Matrix _lc_obs_maxdeg;
  Array _bridgeness;
};


inline uint32_t
MMSBGen::most_likely_group(uint32_t p)
{
  const double **pid = _pi.const_data();
  double max_k = .0, max_p = .0;
  
  for (uint32_t k = 0; k < _k; ++k)
    if (pid[p][k] > max_p) {
      max_p = pid[p][k];
      max_k = k;
    }
  return max_k;
}


inline void
MMSBGen::create_id_dir(uint32_t id)
{
  char buf[128];
  sprintf(buf, "ppc/%d", id);
  setup_dir(string(buf));
  _curr_id = id;
}

inline string
MMSBGen::id_dir() const
{
  char buf[128];
  sprintf(buf, "%d", _curr_id);
  return string("ppc/") + string(buf) + string("/");
}

inline void
MMSBGen::draw_membership_indicators(uint32_t p, uint32_t q)
{
  uArray zp(_k);
  uArray zq(_k);

  const double **pid = _pi.const_data();
  unsigned int *zpd = zp.data();
  unsigned int *zqd = zq.data();
  Array pp(_k);
  Array qp(_k);

  if (_env.disjoint) {
    uint32_t zp_k = most_likely_group(p);
    uint32_t zq_k = most_likely_group(q);
    for (uint32_t k = 0; k < _k; ++k) {
      if (k == zp_k)
	zpd[k] = 1;
      if (k == zq_k)
	zqd[k] = 1;
    }
  } else {
    gsl_ran_multinomial(_r, _k, 1, pid[p], zpd);
    gsl_ran_multinomial(_r, _k, 1, pid[q], zqd);
#ifdef DEBUG_GEN    
    _pi.slice(0, p, pp);
    _pi.slice(0, q, qp);
#endif
  }

  debug("zp = %s\n", zp.s().c_str());
  debug("zq = %s\n", zq.s().c_str());
  uint8_t rate = dot(zp, zq);
  debug("rate = %d\n", rate);

  uint32_t beta_k = 65536;
  yval_t **gen_yd = _gen_y.data();
  if (rate > 0) {
    for (uint32_t k = 0; k < _k; ++k)
      if (zp[k])
	beta_k = k;

    assert (pid[p][beta_k] > .0 && pid[q][beta_k] > .0);

#ifdef DEBUG_GEN
      if (pid[p][beta_k] < 1e-03 || pid[q][beta_k] < 1e-03) {
	double v = edge_likelihood(p,q,beta_k);
	
	printf("zp = %s\n", zp.s().c_str());
	printf("zq = %s\n", zq.s().c_str());
	
	printf("%d -> %d : %d\n", p, q, beta_k);
	printf("pid[%d]:%s\n", p, pp.s().c_str());
	printf("pid[%d]:%s\n", q, qp.s().c_str());
	printf("edge_likelihood:%f\n", v);
	printf("\n\n\n");
      }
#endif

    double brate = _beta[beta_k];
    gen_yd[p][q] = gsl_ran_bernoulli(_r, brate);
    if (_env.undirected)
      gen_yd[q][p] = gen_yd[p][q];
    if (gen_yd[p][q])
      _gen_ones++;
  }
  debug("y = %d\n\n", gen_yd[p][q]);
}

inline void
MMSBGen::estimate_beta(Array &beta) const
{
  const double ** const ld = _lambda.const_data();
  for (uint32_t k = 0; k < _k; ++k) {
    double s = .0;
    for (uint32_t t = 0; t < _t; ++t)
      s += ld[k][t];
    beta[k] = ld[k][0] / s;
  }
}

#endif
