#include "mmsbinferorig.hh"
#include "log.hh"
#include <sys/time.h>

D3 *PhiComp2::_gphi1 = NULL;
D3 *PhiComp2::_gphi2 = NULL;

MMSBInferOrig::MMSBInferOrig(Env &env, Network &network,
			     int itype)
  :_env(env), _network(network),
   _n(env.n), _k(env.k),
   _t(env.t), _s(env.s), _itype(itype),
   _alpha(_k), 
   _gamma(_n,_k), _beta(_k,_k),
   _gammanext(_n,_k), _betanext(_k,_k), 
   _betanexttotals(_k,_k),
   _start_time(time(0)), 
   _Elogpi(_n,_k), _Epi(_n,_k),
   _f(_k,_k),
   _pcomp(env, _iter, _n, _k, _t, 0, 0, 0, 
	  _Elogpi, _beta, _f),
   _max_h(-2147483647),
   _max_h0(.0), _max_h1(.0),
   _max_vh(-2147483647),
   _max_vh0(.0), _max_vh1(.0),
   _max_a(-2147483647),
   _h(.0),_nh(0)
{
  PhiComp2::static_initialize(_n, _k);

  fprintf(stdout, "+ mmsb orig initialization begin\n");
  fflush(stdout);

  if (env.undirected)
    _total_pairs = _n * (_n - 1) / 2; 
  else
    _total_pairs = _n * (_n - 1);

  fprintf(stdout, "+ running inference on %d nodes\n", _n);
  Env::plog("inference n", _n);
  Env::plog("total pairs", _total_pairs);

  _alpha.set_elements(env.alpha);
  _ones_prob = double(_network.ones()) / _total_pairs;
  _zeros_prob = 1 - _ones_prob;

  printf("ones_prob = %.2f", _ones_prob);
  Env::plog("ones_prob", _ones_prob);
  Env::plog("zeros_prob", _zeros_prob);
  
  // random number generation
  gsl_rng_env_setup();
  const gsl_rng_type *T = gsl_rng_default;
  _r = gsl_rng_alloc(T);

  _hef = fopen(Env::file_str("/heldout-edges.txt").c_str(), "w");
  if (!_hef)  {
    printf("cannot open heldout edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  _vef = fopen(Env::file_str("/validation-edges.txt").c_str(), "w");
  if (!_vef)  {
    printf("cannot open validation edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  init_heldout();
  printf("+ done heldout\n");

  assert(!_env.model_load); // not supported
  
  init_gamma();
  if (itype == 0)
    init_beta1();
  else
    init_beta2();    
  debug("gamma = %s", _gamma.s().c_str());

  printf("+ done initializing gamma, beta\n");

  // initialize expectations
  set_dir_exp(_gamma, _Elogpi);
  debug("Elogpi = %s", _Elogpi.s().c_str());

  _statsf = fopen(Env::file_str("/stats.txt").c_str(), "w");
  if (!_statsf)  {
    printf("cannot open stats file:%s\n",  strerror(errno));
    exit(-1);
  }

  _tf = fopen(Env::file_str("/time.txt").c_str(), "w");
  if (!_tf)  {
    printf("cannot open time file:%s\n",  strerror(errno));
    exit(-1);
  }

  _cf = fopen(Env::file_str("/convergence.txt").c_str(), "w");
  if (!_cf)  {
    printf("cannot open convergence file:%s\n",  strerror(errno));
    exit(-1);
  }

  _hf = fopen(Env::file_str("/heldout.txt").c_str(), "w");
  if (!_hf)  {
    printf("cannot open heldout file:%s\n",  strerror(errno));
    exit(-1);
  }
  _vf = fopen(Env::file_str("/validation.txt").c_str(), "w");
  if (!_vf)  {
    printf("cannot open validation file:%s\n",  strerror(errno));
    exit(-1);
  }
  //debug("sparse y = %s\n", _network.sparse_y_s().c_str());

  Env::plog("network ones", _network.ones());
  Env::plog("network singles", _network.singles());

  _lf = fopen(Env::file_str("/logl.txt").c_str(), "w");
  if (!_lf)  {
    printf("cannot open logl file:%s\n",  strerror(errno));
    exit(-1);
  }
  _mf = fopen(Env::file_str("/modularity.txt").c_str(), "w");
  if (!_mf)  {
    printf("cannot open modularity file:%s\n",  strerror(errno));
    exit(-1);
  }
  double a, a0, a1;
  heldout_likelihood(a, a0, a1);

  double av, av0, av1;
  validation_likelihood(av, av0, av1);
  //compute_modularity();
#ifdef GLOBALPHIS
  approx_log_likelihood();
#endif

  fprintf(stdout, "+ mmsb initialization end\n");
  fflush(stdout);
  gettimeofday(&_last_iter, NULL);
  _start_time = time(0);
}

MMSBInferOrig::~MMSBInferOrig()
{
  fclose(_statsf);
  fclose(_hf);
  fclose(_vf); 
  fclose(_lf);
  fclose(_mf);
  fclose(_tf);
  fclose(_cf);
}

void
MMSBInferOrig::init_gamma()
{
  double **d = _gamma.data();
  for (uint32_t i = 0; i < _n; ++i)
    for (uint32_t j = 0; j < _k; ++j)
      d[i][j] = gsl_ran_gamma(_r, 100, 1./100);
  _gammanext.set_elements(_env.alpha);
}

void
MMSBInferOrig::init_beta1()
{
  _beta.zero();
  double **betad = _beta.data();
  for (uint32_t i = 0; i < _k; ++i)
    for (uint32_t j = 0; j < _k; ++j) {
      betad[i][j] = (double)gsl_rng_uniform_int(_r, 100) / 100; 
      if (betad[i][j] > 0.99)
	betad[i][j] = 0.99;
      if (betad[i][j] <= 0.01)
	betad[i][j] = 0.01;
      //gamma(_r, 100, 1./100);
      //if (betad[i][j] > 0.99)
      //	betad[i][j] = 0.99;
      printf("beta->%.2f\n", betad[i][j]);
      assert(betad[i][j] >= .0);
    }
  _betanext.zero();
  _betanexttotals.zero();
}

void
MMSBInferOrig::init_beta2()
{
  double eta0 = _total_pairs * _ones_prob / _k;
  double eta1 = (_total_pairs / (_k * _k)) - eta0;
  if (eta1 < 0)
    eta1 = 1.0;

  _beta.zero();

  double **betad = _beta.data();
  for (uint32_t i = 0; i < _k; ++i)
    for (uint32_t j = 0; j < _k; ++j) {
      if (i == j)
	betad[i][j] = eta0 / (eta0 + eta1);
      else
	betad[i][j] = _env.epsilon;
    }
  printf("%s\n", _beta.s().c_str());
  _betanext.zero();
  _betanexttotals.zero();
}

void
MMSBInferOrig::batch_infer()
{
  printf("+ Running MMSB Orig batch inference\n");

  while (1) {
    set_dir_exp(_gamma, _Elogpi);
    double **betanextd = _betanext.data();
    double **betanexttotalsd = _betanexttotals.data();

    for (uint32_t p = 0; p < _n; ++p) {
      //if (p % 10 == 0) {
      //printf("*");
      //fflush(stdout);
      //}
      
      for (uint32_t q = 0; q < _n; ++q) {
	if (p == q)
	  continue;

	Edge e(p,q);

	const SampleMap::const_iterator u = _heldout_map.find(e);
	if (u != _heldout_map.end())
	  continue;

	yval_t y = get_y(p,q);

	_pcomp.reset(p,q,y);
	_pcomp.update_phis_until_conv();

	const Array &phi1 = _pcomp.phi1();
	const Array &phi2 = _pcomp.phi2();
	
	_gammanext.add_slice(p, phi1);
	_gammanext.add_slice(q, phi2);

	for (uint32_t g = 0; g < _k; ++g)
	  for (uint32_t h = 0; h < _k; ++h) {
	    betanextd[g][h] += y * phi1[g] * phi2[h];
	    betanexttotalsd[g][h] += phi1[g] * phi2[h];
	  }
      }
    }
    printf("done\n");
    fflush(stdout);
    _gamma.reset(_gammanext);
    _gammanext.set_elements(_env.alpha);

    for (uint32_t g = 0; g < _k; ++g)
      for (uint32_t h = 0; h < _k; ++h)  {
	assert (betanexttotalsd[g][h]);
	betanextd[g][h] = betanextd[g][h] / betanexttotalsd[g][h];
      }
  
    _beta.reset(_betanext);
    printf("%s\n", _beta.s().c_str());
    _betanext.zero();
    _betanexttotals.zero();

    _iter++;

    printf("iteration = %d took %d secs\n", _iter, duration());
    fflush(stdout);

    if (_iter % _env.reportfreq == 0) {
      set_dir_exp(_gamma, _Elogpi);
      
      compute_and_log_groups();
      
      double a, a0, a1;
      heldout_likelihood(a, a0, a1);

      double av, av0, av1;
      validation_likelihood(av, av0, av1);

#ifdef GLOBALPHIS
      approx_log_likelihood();
#endif
    }
    debug("beta = %s", _beta.s().c_str());
    debug("gamma = %s", _gamma.s().c_str());
  }
}

void
MMSBInferOrig::compute_and_log_groups() const
{
  FILE *groupsf = fopen(Env::file_str("/groups.txt").c_str(), "w");
  FILE *summaryf = fopen(Env::file_str("/summary.txt").c_str(), "a");
  FILE *commf = fopen(Env::file_str("/communities.txt").c_str(), "w");
  
  char buf[32];
  const IDMap &seq2id = _network.seq2id();
  ostringstream sa;
  Array groups(_n);
  Array pi_i(_k);
  for (uint32_t i = 0; i < _n; ++i) {
    sa << i << "\t";
    IDMap::const_iterator it = seq2id.find(i);
    uint32_t id = 0;
    if (it == seq2id.end()) { // single node
      id = i;
    } else
      id = (*it).second;

    sa << id << "\t";
    estimate_pi(i, pi_i);
    double max = .0;
    for (uint32_t j = 0; j < _k; ++j) {
      memset(buf, 0, 32);
      sprintf(buf,"%.3f", pi_i[j]);
      sa << buf << "\t";
      if (pi_i[j] > max) {
	max = pi_i[j];
	groups[i] = j;
      }
    }
    sa << groups[i] << "\n";
  }
  fprintf(groupsf, "%s", sa.str().c_str());

  D1Array<int> s(_k);
  std::map<uint32_t, vector<uint32_t> > comm;
  for (uint32_t i = 0; i < _n; ++i) {
    s[groups[i]]++;
    comm[groups[i]].push_back(i);
  }
  for (uint32_t i = 0; i < _k; ++i)
    fprintf(summaryf, "%d\t", s[i]);

  for (std::map<uint32_t, vector<uint32_t> >::const_iterator i = comm.begin();
       i != comm.end(); ++i) {
    fprintf(commf, "%d\t", i->first);
    const vector<uint32_t> &u = i->second;
    for (uint32_t p = 0; p < u.size(); ++p)
      fprintf(commf, "%d ", u[p]);
    fprintf(commf, "\n");
  }
  
  fprintf(summaryf,"\n");
  fflush(groupsf);
  fflush(summaryf);
  fflush(commf);
  fclose(groupsf);
  fclose(summaryf);
  fclose(commf);
}

void
MMSBInferOrig::init_heldout()
{
  int s = _env.heldout_ratio * _network.ones();
  set_heldout_sample(s);
  set_validation_sample(s);
  Env::plog("heldout ratio", _env.heldout_ratio);
  Env::plog("heldout edges (1s and 0s)", _heldout_map.size());
  fprintf(_hef, "%s\n", edgelist_s(_heldout_edges).c_str());
  fprintf(_vef, "%s\n", edgelist_s(_validation_edges).c_str());
  fclose(_hef); 
  fclose(_vef); 
}


void
MMSBInferOrig::set_heldout_sample(int s)
{
  int c0 = 0;
  int c1 = 0;
  int p = s / 2;
  while (c0 < p || c1 < p) {
    Edge e;
    get_random_edge(e, true);

    uint32_t a = e.first;
    uint32_t b = e.second;
    assert (a != b);
    
    yval_t y = get_y(a,b); 
    if (y == 0 and c0 < p) {
      c0++;
      _heldout_edges.push_back(e);
      _heldout_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _heldout_edges.push_back(e);
      _heldout_map[e] = true;
    }
    //printf("c0 = %d, c1 = %d\n", c0, c1);
    //fflush(stdout);
  }
}

void
MMSBInferOrig::set_validation_sample(int s)
{
  int c0 = 0;
  int c1 = 0;
  int p = s / 2;
  while (c0 < p || c1 < p) {
    Edge e;
    get_random_edge(e, false);

    uint32_t a = e.first;
    uint32_t b = e.second;
    assert (a != b);

    yval_t y = get_y(a,b);

    if (y == 0 and c0 < p) {
      c0++;
      _validation_edges.push_back(e);
      _validation_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _validation_edges.push_back(e);
      _validation_map[e] = true;
    }
  }
}
 
void
MMSBInferOrig::get_random_edge(Edge &e, bool heldout = false) const
{
  do {
    e.first = gsl_rng_uniform_int(_r, _n);
    e.second = gsl_rng_uniform_int(_r, _n);
  } while (!edge_ok(e, heldout));
}

bool
MMSBInferOrig::edge_ok(const Edge &e, bool heldout) const
{
  bool ok = true;
  if (e.first != e.second) {
    if (!heldout) { // if not heldout check that edge is not in both maps
      const SampleMap::const_iterator u = _heldout_map.find(e);
      if (u != _heldout_map.end())
	ok = false;

      const SampleMap::const_iterator v = _validation_map.find(e);
      if (v != _validation_map.end())
	ok = false;
    }
  } else
    ok = false;
  return ok;
}

void
MMSBInferOrig::heldout_likelihood(double &a, double &a0, double &a1) const
{
  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  for (SampleMap::const_iterator i = _heldout_map.begin(); 
       i != _heldout_map.end(); ++i) {
    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

    // bool seen = yd[p][q] & 0x80; XXX
    // assert (not seen);

    yval_t y = get_y(p,q);
    double u = edge_likelihood(p,q,y);
    s += u;
    k += 1;
    if (y) {
      sones += u;
      kones++;
    } else {
      szeros += u;
      kzeros++;
    }
    debug("edge likelihood for (%d,%d) is %f\n", p,q,u);
  }
  fprintf(_hf, "%d\t%d\t%.5f\t%d\t%.5f\t%d\t%.5f\t%d\n", 
	  _iter, duration(), s / k, k, 
	  szeros / kzeros, kzeros, sones / kones, kones);
  fflush(_hf);

  a = s / k;
  a0 = szeros / kzeros;
  a1 = sones / kones;

#if 0
  if (a > _max_h) {
    double av, av0, av1;
    validation_likelihood(av, av0, av1);

    _max_h = a;
    _max_h0 = a0;
    _max_h1 = a1;

    _max_vh = av;
    _max_vh0 = av0;
    _max_vh1 = av1;
    _nh = 0;
  } else if (a < _max_h && _iter > 2000)
    _nh++;

  if (_nh > 10) {
    FILE *f = fopen(Env::file_str("/max.txt").c_str(), "w");
    fprintf(f, "%d\t%d\t%f\t%f\n", _iter, duration(), 
	    _max_h, _max_vh);
    fclose(f);
    exit(0);
  }
#endif
}

void
MMSBInferOrig::validation_likelihood(double &a, double &a0, double &a1) const
{
  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  for (SampleMap::const_iterator i = _validation_map.begin(); 
       i != _validation_map.end(); ++i) {
    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

    yval_t y = get_y(p,q);
    // bool seen = yd[p][q] & 0x80;
    // assert (not seen);
    double u = edge_likelihood(p,q,y);
    s += u;
    k += 1;
    if (y) {
      sones += u;
      kones++;
    } else {
      szeros += u;
      kzeros++;
    }
    debug("edge likelihood for (%d,%d) is %f\n", p,q,u);
  }
  fprintf(_vf, "%d\t%d\t%.5f\t%d\t%.5f\t%d\t%.5f\t%d\n", 
	  _iter, duration(), s / k, k, 
	  szeros / kzeros, kzeros, sones / kones, kones);
  fflush(_vf);

  a = s / k;
  a0 = szeros / kzeros;
  a1 = sones / kones;
}

inline double
MMSBInferOrig::edge_likelihood(uint32_t p, uint32_t q, yval_t y) const
{
  Array pi_p(_k);
  Array pi_q(_k);
  estimate_pi(p, pi_p);
  estimate_pi(q, pi_q);

  debug("estimated pi (%d) = %s\n", p, pi_p.s().c_str());
  debug("estimated pi (%d) = %s\n", q, pi_q.s().c_str());

  const double ** const betad = _beta.data();

  double s = .0;
  for (uint32_t g = 0; g < _k; ++g) 
    for (uint32_t h = 0; h < _k; ++h) {
      double brate = betad[g][h];
      debug("(%d:%d):1 estimated rate = %f, %f\n", p, q, brate,
	    gsl_ran_bernoulli_pdf(y, brate));
      s += pi_p[g] * pi_q[h] * gsl_ran_bernoulli_pdf(y, brate);
    }
  //assert(s > .0);
  //if (s < 1e-30)
  //s = 1e-30;
  return log(s);
}

string
MMSBInferOrig::edgelist_s(EdgeList &elist)
{
  ostringstream sa;
  for (EdgeList::const_iterator i = elist.begin(); i != elist.end(); ++i) {
    const Edge &p = *i;
    sa << p.first << "\t" << p.second << "\n";
  }
  return sa.str();
}

void
MMSBInferOrig::set_dir_exp(const Matrix &u, Matrix &exp)
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
      double q = .0;
      if (d[i][j] < 1e-30)
	q = 1e-30;
      else
	q = d[i][j];
      e[i][j] = gsl_sf_psi(q) - psi_sum;
    }
  }
}

#ifdef GLOBALPHIS
double
MMSBInferOrig::approx_log_likelihood() const
{
  const double * const alphad = _alpha.data();
  const double ** const elogpid = _Elogpi.data();
  const double ** const gd = _gamma.data();
  const double *** const gphi1d = PhiComp2::gphi1().data();
  const double *** const gphi2d = PhiComp2::gphi2().data();

  double s = .0, v = .0;
  for (uint32_t p = 0; p < _n; ++p) {
    for (uint32_t q = 0; q < _n; ++q) {
      if (p == q)
	continue;

      yval_t y = get_y(p,q);

      const double * const phi1 = gphi1d[p][q];
      const double * const phi2 = gphi2d[p][q];

      Matrix f(_k,_k);
      PhiComp2::compute_f(p,q,y,_k,_t,_beta,f);
      double **fd = f.data();

      //printf("%s\n", f.s().c_str());
      //fflush(stdout);
      
      for (uint32_t g = 0; g < _k; ++g)
	for (uint32_t h = 0; h < _k; ++h)
	  s += phi1[g] * phi2[h] * log(fd[g][h] + 1e-10);
      
      for (uint32_t k = 0; k < _k; ++k) {
	s += phi1[k] * elogpid[p][k];
	s += phi2[k] * elogpid[q][k];
      } 

      for (uint32_t k = 0; k < _k; ++k) {
	s -= log(phi1[k]) * phi1[k];
	s -= log(phi2[k]) * phi2[k];
      }
    }
  }

  for (uint32_t p = 0; p < _n; ++p) {
    v = .0;
    for (uint32_t k = 0; k < _k; ++k)
      v += gsl_sf_lngamma(alphad[k]);
    s += gsl_sf_lngamma(_alpha.sum()) - v;

    v = .0;
    for (uint32_t k = 0; k < _k; ++k)
      v += (alphad[k] - 1) * elogpid[p][k];
    s += v;

    v = .0;
    for (uint32_t k = 0; k < _k; ++k)
      v += gsl_sf_lngamma(gd[p][k]);
    s -= gsl_sf_lngamma(_gamma.sum(p)) - v;

    v = .0;
    for (uint32_t k = 0; k < _k; ++k)
      v += (gd[p][k] - 1) * elogpid[p][k];
    s -= v;
  }

  printf("approx. log likelihood = %f\n", s);
  fprintf(_lf, "%d\t%d\t%.5f\n", _iter, duration(), s); 
  fflush(_lf);

  if (s > _max_a) {
    _max_a = s;

    // calculate held-out and validation likelihood at max. approx log
    // likelihood

    double a, a0, a1;
    heldout_likelihood(a, a0, a1);

    double av, av0, av1;
    validation_likelihood(av, av0, av1);

    _max_h = a;
    _max_h0 = a0;
    _max_h1 = a1;

    _max_vh = av;
    _max_vh0 = av0;
    _max_vh1 = av1;
    _nh = 0;
  } else if (s < _max_a && _iter > 10)
    _nh++;

  if (_nh > 3) {
    FILE *f = fopen(Env::file_str("/max.txt").c_str(), "w");
    fprintf(f, "%d\t%d\t%f\t%f\t%f\t%f\t%f\t%f\t%f\n", _iter, duration(), 
	    _max_a, _max_h, _max_h0, _max_h1, _max_vh, _max_vh0, _max_vh1);
    fclose(f);
    exit(0);
  }
  return s;
}
#endif
