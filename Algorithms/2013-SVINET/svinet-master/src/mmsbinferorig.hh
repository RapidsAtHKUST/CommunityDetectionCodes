#ifndef MMSBINFERORIG_HH
#define MMSBINFERORIG_HH

#include <list>
#include <utility>
#include <unistd.h>

#include "env.hh"
#include "matrix.hh"
#include "network.hh"
#include "thread.hh"
#include "tsqueue.hh"

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_psi.h>
#include <gsl/gsl_sf.h>

#define GLOBALPHIS 1

// bool operator==(const Edge &a, const Edge &b);
class PhiComp2 {
public:
  PhiComp2(const Env &env, const uint32_t &iter,
	  uint32_t n, uint32_t k, uint32_t t,
	  uint32_t p, uint32_t q, yval_t y,
	  const Matrix &Elogpi, 
	  const Matrix &beta,
	  Matrix &f, bool phifix = false)
    : _env(env), _iter(iter),
      _n(n), _k(k), _t(t), 
      _p(p), _q(q), _y(y),
      _Elogpi(Elogpi), _beta(beta),
      _f(f),
      _v1(_k), _v2(_k),
      _phi1(_k), _phi2(_k),
      _phinext1(_k), _phinext2(_k),
      _phi1old(_k), _phi2old(_k),
      _phifix(phifix) { }
  ~PhiComp2() { }

  static void static_initialize(uint32_t n, uint32_t k);
  static void static_uninitialize();

  void reset(uint32_t p, uint32_t q, yval_t y);

  const Array& phi1() const { return _phi1; }
  const Array& phi2() const { return _phi2; }

  uint32_t iter() const { return _iter; }

  void update_phis_until_conv();
  void update_phis(bool is_phi1);

  static const D3 &gphi1() { assert(_gphi1); return *_gphi1; }
  static const D3 &gphi2() { assert(_gphi2); return *_gphi2; }
  static void compute_f(uint32_t p, uint32_t q, yval_t y,
			uint32_t K, uint32_t T,
			const Matrix &beta, Matrix &f);

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
  const Matrix &_beta;
  Matrix &_f;

  Array _v1;
  Array _v2;
  
  Array _phi1;
  Array _phi2;
  Array _phinext1;
  Array _phinext2;
  Array _phi1old;
  Array _phi2old;
  bool _phifix;
  
  static D3 *_gphi1;
  static D3 *_gphi2;
};

inline void
PhiComp2::static_initialize(uint32_t n, uint32_t k)
{
#ifdef GLOBALPHIS
  _gphi1 = new D3(n,n,k);
  _gphi2 = new D3(n,n,k);
  _gphi1->set_elements(1./k);
  _gphi2->set_elements(1./k);
#endif
}

inline void
PhiComp2::static_uninitialize()
{
  delete _gphi1;
  delete _gphi2;
}

inline void
PhiComp2::reset(uint32_t p, uint32_t q, yval_t y)
{
  _p = p;
  _q = q;
  _y = y;
  _v1.zero();
  _v2.zero();
}

inline void
PhiComp2::update_phis(bool is_phi1)
{
  Array &b = is_phi1 ? _phi2 : _phi1;
  Array &anext = is_phi1 ? _phinext1 : _phinext2;
  uint32_t c = is_phi1 ? _p : _q;

  const double ** const elogpid = _Elogpi.const_data();
  const double ** const fd = _f.const_data();
  for (uint32_t g = 0; g < _k; ++g) {
    double u = elogpid[c][g];
    for (uint32_t h = 0; h < _k; ++h) {
      assert(fd[g][h] > 0);
      u += log(fd[g][h]) * b[h];
      //printf("%.2f %.2f\n", fd[g][h], u);
      //fflush(stdout);
    }
    anext[g] = u;
  }
  
  //anext.lognormalize();
  double s = .0;
  for (uint32_t i = 0; i < _k; ++i) {
    anext[i] = exp(anext[i]);
    s += anext[i];
  }
  assert(s);
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
PhiComp2::compute_f(uint32_t p, uint32_t q, yval_t y,
		    uint32_t K, uint32_t T,
		    const Matrix &beta, Matrix &f)
{
  f.zero();
  const double ** const betad = beta.data();
  double **fd = f.data();
  for (uint32_t g = 0; g < K; ++g)
    for (uint32_t h = 0; h < K; ++h) {
      fd[g][h] = pow(betad[g][h], y) * pow(1 - betad[g][h], 1-y);
      //printf("beta -> %.2f\n", betad[g][h]);
      //printf("fd -> %.2f\n", fd[g][h]);
    }
}

inline void
PhiComp2::update_phis_until_conv()
{
  double u = 1./_k;
  _phi1.set_elements(u);
  _phi2.set_elements(u);

  compute_f(_p,_q,_y,_k,_t,_beta,_f);

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
  
#ifdef GLOBALPHIS
    _gphi1->copy_slice(_p,_q,_phi1);
    _gphi2->copy_slice(_p,_q,_phi2);
#endif
}


class MMSBInferOrig {
public:
  MMSBInferOrig(Env &env, Network &network, int itype);
  ~MMSBInferOrig();

  void batch_infer();
  void save_model();
  
  static void set_dir_exp(const Matrix &u, Matrix &exp);
  
private:
  void init_gamma();
  void init_beta1();
  void init_beta2();
  void compute_and_log_groups() const;
  void init_heldout();
  void set_heldout_sample(int s);
  void set_validation_sample(int s);
  void get_random_edge(Edge &e, bool heldout) const;
  bool edge_ok(const Edge &e, bool heldout) const;

  yval_t get_y(uint32_t p, uint32_t q) const;

  double approx_log_likelihood() const;

  void heldout_likelihood(double &a, double &a0, double &a1) const;
  void validation_likelihood(double &a, double &a0, double &a1) const;
  double edge_likelihood(uint32_t p, uint32_t q, yval_t y) const;

  uint32_t duration() const;
  void estimate_pi(uint32_t p, Array &pi_p) const;
  string edgelist_s(EdgeList &elist);
  
  const Env &_env;
  Network &_network;

  SampleMap _heldout_map;
  SampleMap _validation_map;

  uint32_t _n;
  uint32_t _k;
  uint32_t _t;
  uint32_t _s;
  uint32_t _itype;
  uint32_t _iter;
  uint32_t _lambda_start_iter;
  Array _alpha;

  double _ones_prob;
  double _zeros_prob;
  EdgeList _heldout_edges;
  EdgeList _validation_edges;
  gsl_rng *_r;

  Matrix _gamma;
  Matrix _beta;

  Matrix _gammanext;
  Matrix _betanext;
  Matrix _betanexttotals;

  time_t _start_time;
  struct timeval _last_iter;

  Matrix _Elogpi;
  Matrix _Epi;
  Matrix _f;

  FILE *_statsf;
  FILE *_tf;
  FILE *_cf;
  FILE *_hf;
  FILE *_vf;
  FILE *_mf;
  FILE *_hef;
  FILE *_vef;
  FILE *_lf;

  double _total_pairs;
  PhiComp2 _pcomp;

  mutable double _max_h, _max_h0, _max_h1;
  mutable double _max_vh, _max_vh0, _max_vh1;
  mutable double _max_a;
  double _h;
  mutable uint32_t _nh;
};

inline uint32_t
MMSBInferOrig::duration() const
{
  time_t t = time(0);
  return t - _start_time;
}

inline void
MMSBInferOrig::estimate_pi(uint32_t p, Array &pi_p) const
{
  const double ** const gd = _gamma.data();
  double s = .0;
  for (uint32_t k = 0; k < _k; ++k)
    s += gd[p][k];
  assert(s);
  for (uint32_t k = 0; k < _k; ++k)
    pi_p[k] = gd[p][k] / s;
}

inline yval_t
MMSBInferOrig::get_y(uint32_t p, uint32_t q) const
{
  return _network.y(p,q);
}

#endif
