#include "fastamm.hh"
#include "log.hh"
#include <sys/time.h>

int FastAMM::ea = 0;
int FastAMM::eb = 0;

uint32_t FastQueue::_K = 0;
double FastQueue::_alpha = .0;

FastAMM::FastAMM(Env &env, Network &network)
  :_env(env), _network(network),
   _n(env.n), _k(env.k),
   _t(env.t), _s(env.s),
   _alpha(_k),
   _family(0), _prev_mbsize0(_s), _prev_mbsize1(_s),
   _eta(_k,_t),
   _hitcurve_id(0), _inf_epsilon(0.0001), _noninf_setsize(200),
   _link_thresh(0.9),
   _gamma(_n,_k), _lambda(_k,_t),
   _tau0(env.tau0 + 1), _kappa(env.kappa),
   _nodetau0(env.nodetau0 + 1), _nodekappa(env.nodekappa),
   _rhot(.0), _noderhot(_n), _nodec(_n),
   _nodeupdatec(_n),
   _nodeupdate_rn(_n),
   _start_time(time(0)),
   _Elogpi(_n,_k), _Elogbeta(_k,_t),

#ifdef COMPUTE_GROUPS
   _Epi(_n,_k),
#endif

#ifdef EGO_NETWORK
   _gammat(1,1), // lower memory for ego analysis
#else
   _gammat(_n,_k),
#endif
   _lambdat(_k,_t), _count(_n),
   _Elogf(_k),
   _pcomp(env, &_r, _iter, _n, _k, _t, 0, 0, 0,
	  _Elogpi, _Elogbeta, _Elogf),
   _shuffled_nodes(_n),
   _delaylearn_reported(false),
   _max_t(-2147483647),
   _max_h(-2147483647),
   _prev_h(-2147483647),
   _prev_w(-2147483647),
   _prev_t(-2147483647),
   _nh(0), _nt(0),
   _training_done(false),
   _start_node(0),
   _neighbors(_env.reportfreq)
{
  //FastQueue::static_initialize(_k, _env.alpha);

  printf("+ fastamm initialization begin\n");
  fflush(stdout);
  ea = 0;
  eb = 0;

  info("+ running inference on %lu nodes\n", _n);
  Env::plog("inference n", _n);
  Env::plog("total pairs", _total_pairs);
  Env::plog("inf_epsilon", _inf_epsilon);
  Env::plog("noninf_setsize", _noninf_setsize);
  Env::plog("link_thresh", _link_thresh);

  _alpha.set_elements(env.alpha);
  info("alpha set to %s\n", _alpha.s().c_str());
  fflush(stdout);

  double **d = _eta.data();
  for (uint32_t i = 0; i < _eta.m(); ++i) {
    d[i][0] = env.eta0;
    d[i][1] = env.eta1;
  }

  // random number generation
  gsl_rng_env_setup();
  const gsl_rng_type *T = gsl_rng_default;
  _r = gsl_rng_alloc(T);
  if (_env.seed)
    gsl_rng_set(_r, _env.seed);

  shuffle_nodes();

  _hef = fopen(Env::file_str("/heldout-pairs.txt").c_str(), "w");
  if (!_hef)  {
    lerr("cannot open heldout pairs file:%s\n",  strerror(errno));
    exit(-1);
  }

  _vef = fopen(Env::file_str("/validation-pairs.txt").c_str(), "w");
  if (!_vef)  {
    lerr("cannot open validation edges file:%s\n",  strerror(errno));
    exit(-1);
  }

  _pef = fopen(Env::file_str("/precision-pairs.txt").c_str(), "w");
  if (!_pef)  {
    lerr("cannot open precision pairs file:%s\n",  strerror(errno));
    exit(-1);
  }

#ifdef TRAINING_SAMPLE
  _tef = fopen(Env::file_str("/training-pairs.txt").c_str(), "w");
  if (!_tef)  {
    lerr("cannot open training edges file:%s\n",  strerror(errno));
    exit(-1);
  }
#endif

  init_heldout();
  info("+ done heldout\n");

#ifdef PRECISION_SAMPLE
  if (_env.adamic_adar) {
    compute_adamic_adar_score();
    exit(0);
  }
#endif

  info("+ initializing gamma, lambda\n");
  if (_env.model_load) {
    assert(load_model() >= 0);

#ifdef EGO_NETWORK
  uint32_t M = 10;
  //uint32_t M = 20;
  
  uArray nodes(M);
  
  // ARXIV
  // nodes[0] = 82233;
  // nodes[1] = 112330;
  // nodes[2] = 55847;

  // PATENTS
  nodes[0] = 3953566;
  //nodes[1] = 3845770; <- low bridgeness
  //nodes[2] = 3976982; <- low bridgeness
  nodes[1] = 4367924;
  nodes[2] = 5103459;
  nodes[3] = 4901307;
  nodes[4] = 5109390;
  nodes[5] = 5101501;
  nodes[6] = 5167024;
  nodes[7] = 4234435;
  nodes[8] = 4230463;
  nodes[9] = 4190757;

  // nodes[0] = 55847;
  // nodes[1] = 29745;
  // nodes[2] = 37929;
  // nodes[3] = 179228;
  // nodes[4] = 18553;
  // nodes[5] = 98256;
  // nodes[6] = 219529;
  // nodes[7] = 47743;
  // nodes[8] = 11836;
  // nodes[9] = 31861;
  // nodes[10] = 314660;
  
  // US PATENTS

  // nodes[5] = 3953566;
  // nodes[6] = 4367924;
  // nodes[7] = 4234435;
  // nodes[8] = 4230463;
  // nodes[9] = 4190757;
  // nodes[10] = 5056109;
  // nodes[11] = 4228496;
  // nodes[12] = 4916441;
  // nodes[13] = 5008853;
  // nodes[14] = 4313124;
  // nodes[15] = 4740796;
  // nodes[16] = 4655771;
  // nodes[17] = 4310440;
  // nodes[18] = 4776337;
  // nodes[19] = 4647447;

  group_stats(nodes, M);
  ego(nodes, M);
  exit(0);
#endif

  } else {
    init_gamma();
    assert (init_lambda() >= 0);
    Env::plog("lambda", _lambda);
  }

  info("+ done initializing gamma, lambda\n");

  // initialize expectations
  info("+ Elogpi and Elogbeta\n");
  set_dir_exp(_gamma, _Elogpi);
  set_dir_exp(_lambda, _Elogbeta);

  info("+ done Elogpi and Elogbeta\n");

  debug("Elogpi = %s", _Elogpi.s().c_str());
  debug("Elogbeta = %s", _Elogbeta.s().c_str());

  _statsf = fopen(Env::file_str("/stats.txt").c_str(), "w");
  if (!_statsf)  {
    lerr("cannot open stats file:%s\n",  strerror(errno));
    exit(-1);
  }

  _tf = fopen(Env::file_str("/time.txt").c_str(), "w");
  if (!_tf)  {
    lerr("cannot open time file:%s\n",  strerror(errno));
    exit(-1);
  }

  _cf = fopen(Env::file_str("/convergence.txt").c_str(), "w");
  if (!_cf)  {
    lerr("cannot open convergence file:%s\n",  strerror(errno));
    exit(-1);
  }

  _cmapf = fopen(Env::file_str("/cmap.txt").c_str(), "w");
  if (!_cmapf)  {
    lerr("cannot open cmap file:%s\n",  strerror(errno));
    exit(-1);
  }

  _hf = fopen(Env::file_str("/heldout.txt").c_str(), "w");
  if (!_hf)  {
    lerr("cannot open heldout file:%s\n",  strerror(errno));
    exit(-1);
  }

  _vf = fopen(Env::file_str("/validation.txt").c_str(), "w");
  if (!_vf)  {
    lerr("cannot open validation file:%s\n",  strerror(errno));
    exit(-1);
  }

  _pf = fopen(Env::file_str("/precision.txt").c_str(), "w");
  if (!_pf)  {
    lerr("cannot open precision file:%s\n",  strerror(errno));
    exit(-1);
  }

#ifdef TRAINING_SAMPLE
  _trf = fopen(Env::file_str("/training.txt").c_str(), "w");
  if (!_trf)  {
    lerr("cannot open training file:%s\n",  strerror(errno));
    exit(-1);
  }
  debug("y = %s", _network.y().s().c_str());
#endif

  Env::plog("network ones", _network.ones());
  Env::plog("network singles", _network.singles());

  _lf = fopen(Env::file_str("/logl.txt").c_str(), "w");
  if (!_lf)  {
    lerr("cannot open logl file:%s\n",  strerror(errno));
    exit(-1);
  }
  _mf = fopen(Env::file_str("/modularity.txt").c_str(), "w");
  if (!_mf)  {
    lerr("cannot open modularity file:%s\n",  strerror(errno));
    exit(-1);
  }

  if (_env.model_load) {
    gettimeofday(&_last_iter, NULL);
    compute_stats();
    exit(0);
  }

  info("+ computing initial heldout likelihood\n");
  heldout_likelihood();
  validation_likelihood();

#ifdef PRECISION_SAMPLE
  uint32_t c10, c100, c1000;
  compute_precision(c10, c100, c1000);
#endif

#ifdef TRAINING_SAMPLE
  training_likelihood();
#endif

  if (_env.logl)
    approx_log_likelihood();

  gettimeofday(&_last_iter, NULL);
  info("+ fastamm initialization end\n");
  fflush(stdout);
}

FastAMM::~FastAMM()
{
  fclose(_statsf);
  fclose(_hf);
  fclose(_pf);
  fclose(_tf);
  fclose(_lf);
  fclose(_mf);
  fclose(_trf);
  fclose(_cf);
  fclose(_cmapf);

#ifdef MRSTATS
  fclose(_mrstatsf);
#endif
}

void
FastAMM::init_heldout()
{
  int s = _env.heldout_ratio * _network.ones();
  info("set validation samples\n");
  set_heldout_sample(s);
  set_validation_sample(s);

#ifdef PRECISION_SAMPLE
  info("set precision sample\n");
  set_precision_sample();
#endif

#ifdef TRAINING_SAMPLE
  set_training_sample(2*(_network.ones() - s));
#endif

  Env::plog("heldout ratio", _env.heldout_ratio);
  Env::plog("heldout pairs (1s and 0s)", _heldout_map.size());

#ifdef PRECISION_SAMPLE
  Env::plog("precision ratio", _env.precision_ratio);
  Env::plog("precision links", precision_ones());
  Env::plog("precision nonlinks", precision_zeros());
  Env::plog("precision pairs (1s and 0s)", _precision_map.size());
#endif

  fprintf(_hef, "%s\n", edgelist_s(_heldout_pairs).c_str());
  fclose(_hef);

  fprintf(_vef, "%s\n", edgelist_s(_validation_pairs).c_str());
  fclose(_vef);

#ifdef PRECISION_SAMPLE
  fprintf(_pef, "%s\n", edgelist_s(_precision_pairs).c_str());
  fclose(_pef);
#endif

#ifdef TRAINING_SAMPLE
  fprintf(_tef, "%s\n", edgelist_s(_training_pairs).c_str());
  fclose(_tef);
#endif
}

string
FastAMM::edgelist_s(EdgeList &elist)
{
  ostringstream sa;
  for (EdgeList::const_iterator i = elist.begin(); i != elist.end(); ++i) {
    const Edge &p = *i;
    const IDMap &m = _network.seq2id();
    IDMap::const_iterator a = m.find(p.first);
    IDMap::const_iterator b = m.find(p.second);
    if (a != m.end() && b!= m.end())
      sa << a->second << "\t" << b->second << "\n";      
  }
  return sa.str();
}

#ifdef PRECISION_SAMPLE
void
FastAMM::set_precision_sample()
{
  if (_env.accuracy)
    return;

  int c0 = 0;
  int c1 = 0;
  int p0 = precision_zeros();
  int p1 = precision_ones();

  while (c0 < p0 || c1 < p1) {
    Edge e;
    if (c0 == p0)
      get_random_edge(true, e); // link
    else
      get_random_edge(false, e); // nonlink

    uint32_t a = e.first;
    uint32_t b = e.second;
    yval_t y = get_y(a,b);

    if (y == 0 and c0 < p0) {
      c0++;
      _precision_pairs.push_back(e);
      _precision_map[e] = true;
    }
    if (y == 1 and c1 < p1) {
      c1++;
      _precision_pairs.push_back(e);
      _precision_map[e] = true;
    }
  }
}
#endif

void
FastAMM::set_heldout_sample(int s)
{
  int c0 = 0;
  int c1 = 0;
  int p = s / 2;
  while (c0 < p || c1 < p) {
    Edge e;
    if (c0 == p)
      get_random_edge(true, e); // link
    else
      get_random_edge(false, e); // nonlink

    uint32_t a = e.first;
    uint32_t b = e.second;
    yval_t y = get_y(a,b);

    if (y == 0 and c0 < p) {
      c0++;
      _heldout_pairs.push_back(e);
      _heldout_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _heldout_pairs.push_back(e);
      _heldout_map[e] = true;
    }
  }
}

void
FastAMM::set_validation_sample(int s)
{
  if (_env.accuracy)
    return;
  int c0 = 0;
  int c1 = 0;
  int p = s / 2;
  while (c0 < p || c1 < p) {
    Edge e;
    if (c0 == p)
      get_random_edge(true, e); // link
    else
      get_random_edge(false, e); // nonlink

    uint32_t a = e.first;
    uint32_t b = e.second;
    yval_t y = get_y(a,b);

    if (y == 0 and c0 < p) {
      c0++;
      _validation_pairs.push_back(e);
      _validation_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _validation_pairs.push_back(e);
      _validation_map[e] = true;
    }
  }
}


#ifdef TRAINING_SAMPLE
void
FastAMM::set_training_sample(int s)
{
  int c0 = 0;
  int c1 = 0;
  int p = s / 2;
  while (c0 < p || c1 < p) {
    Edge e;
    if (c0 == p)
      get_random_edge(true, e); // link
    else
      get_random_edge(false, e); // nonlink

    uint32_t a = e.first;
    uint32_t b = e.second;
    yval_t y = get_y(a,b);

    if (y == 0 and c0 < p) {
      c0++;
      _training_pairs.push_back(e);
      _training_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _training_pairs.push_back(e);
      _training_map[e] = true;
    }
  }
}
#endif

void
FastAMM::shuffle_nodes()
{
  for (uint32_t i = 0; i < _n; ++i)
    _shuffled_nodes[i] = i;
  gsl_ran_shuffle(_r, (void *)_shuffled_nodes.data(), _n, sizeof(uint32_t));
}

void
FastAMM::init_gamma()
{
  double **d = _gamma.data();
  for (uint32_t i = 0; i < _n; ++i) {
    for (uint32_t j = 0; j < _k; ++j)  {
      if (_env.deterministic) {
	d[i][j] = 0.09 + (0.01 * ((i+1) / ( i + j + 1)));
	if (d[i][j] > 1.) {
	  d[i][j] = 0.9;
	}
      } else {
	double v = (_k < 100) ? 1.0 : (double)100.0 / _k;
	d[i][j] = gsl_ran_gamma(_r, 100 * v, 0.01);
      }
    }
  }
}

int
FastAMM::init_lambda()
{
  if (_lambda.copy_from(_eta) < 0) {
    lerr("init lambda failed");
    return -1;
  }
  double **ld = _lambda.data();
  for (uint32_t k = 0; k < _k; ++k)
    for (uint32_t t = 0; t < _t; ++t) {
      double v = (_k <= 100) ? 1.0 : (double)100.0 / _k;
      ld[k][t] += gsl_ran_gamma(_r, 100 * v, 0.01);
    }
  return 0;
}

void
FastAMM::infer()
{
#ifdef PERF
  struct timeval s;
  s.tv_sec = 0;
  s.tv_usec = 0;
#endif

  while (1) {

    if (_env.max_iterations && _iter > _env.max_iterations) {
      printf("+ Quitting: reached max iterations.\n");
      Env::plog("maxiterations reached", true);
      exit(0);
    }

    _lambdat.zero();
    set_dir_exp(_lambda, _Elogbeta);

#ifdef PERF
    struct timeval tv3,tv4,r;
    gettimeofday(&tv3, NULL);
#endif

    vector<uint32_t> nodes;
    uint32_t type = gsl_ran_bernoulli(_r, _inf_epsilon);

    if (type == 0)
      opt_process(nodes);
    else // non-informative sets
      opt_process_noninf(nodes);

#ifdef PERF
    gettimeofday(&tv4, NULL);
    timeval_subtract(&r, &tv4, &tv3);
    timeval_add(&s, &r);
#endif

    double scale = type == 0 ? 
      (double)_n / 2 : ((double)_n * (double)_n) / (2 * _inf_epsilon * _noninf_setsize);
    double **gd = _gamma.data();
    double **gdt = _gammat.data();

    for (uint32_t i = 0; i < nodes.size(); ++i) {
      uint32_t a = nodes[i];
      _noderhot[a] = pow(_nodetau0 + _nodec[a], -1 * _nodekappa);

      for (uint32_t k = 0; k < _k; ++k)
	gd[a][k] = (1 - _noderhot[a]) * gd[a][k] + \
	  _noderhot[a] * (_alpha[k] + scale * gdt[a][k]);

      _nodec[a]++;
      _iter_map[a]++;
    }

    if (!_env.nolambda)  {
      _rhot = pow(_tau0 + (_iter - _lambda_start_iter + 1), -1 * _kappa);

      double **ldt = _lambdat.data();
      double **ed = _eta.data();
      double **ld = _lambda.data();

      for (uint32_t k = 0; k < _k; ++k)
	for (uint32_t t = 0; t < _t; ++t) {
	  ldt[k][t] = ed[k][t] + scale  * ldt[k][t];
	  ld[k][t] = (1 - _rhot) * ld[k][t] + _rhot * ldt[k][t];
	}
    }

    _iter++;

    fflush(stdout);

    if (_iter % 100 == 0 || type == 1) {
#ifdef PERF
      info("(%d) opt_process took %ld:%ld secs\n",
	     type, s.tv_sec, s.tv_usec);
#endif
      printf("\riteration = %d took %d secs", _iter, duration());
    }

    if (_iter % _env.reportfreq == 0 || _env.terminate) {
#ifdef PERF
      s.tv_sec = 0;
      s.tv_usec = 0;
#endif

      double n_mean = _neighbors.mean();
      double n_stdev = _neighbors.stdev(n_mean);
      info("neighbor size = (mean:%.5f, stdev:%.5f)", n_mean, n_stdev);
      fprintf(_cmapf,"%d\t%d\t%.5f\t%.5f\n", _iter, duration(), n_mean, n_stdev);
      fflush(_cmapf);

      fflush(stdout);

      _neighbors.zero();

      info("validation begin\n");
      heldout_likelihood();
      validation_likelihood();
      info("validation end\n");
      fflush(stdout);

#ifdef PRECISION_SAMPLE
      uint32_t c10, c100, c1000;
      info("precision begin\n");
      compute_precision(c10, c100, c1000);
      info("precision end\n");
#endif

      if (_env.terminate) {
	do_on_stop();
	_env.terminate = false;
      }

#ifdef TRAINING_SAMPLE
      training_likelihood();
#endif
      
      if (_env.logl)
	approx_log_likelihood();
    }
  }
}

void
FastAMM::do_on_stop()
{
  info("saving model...\n");
  save_model();
  info("done saving model!\n");
  
#ifdef COMPUTE_GROUPS
  info("computing groups...\n");
  estimate_all_pi();
  compute_and_log_groups();
  info("done computing groups!\n");
#endif
}

void
FastAMM::save_model()
{
  FILE *gammaf = fopen(Env::file_str("/gamma.txt").c_str(), "w");
  const double ** const gd = _gamma.const_data();
  for (uint32_t i = 0; i < _n; ++i) {
    const IDMap &m = _network.seq2id();
    IDMap::const_iterator idt = m.find(i);
    if (idt != m.end()) {
      fprintf(gammaf,"%d\t", i);
      debug("looking up i %d\n", i);
      fprintf(gammaf,"%d\t", (*idt).second);
      for (uint32_t k = 0; k < _k; ++k) {
	if (k == _k - 1)
	  fprintf(gammaf,"%.5f\n", gd[i][k]);
	else
	  fprintf(gammaf,"%.5f\t", gd[i][k]);
      }
    }
  }
  fclose(gammaf);

  FILE *lambdaf = fopen(Env::file_str("/lambda.txt").c_str(), "w");
  const double ** const ld = _lambda.const_data();
  for (uint32_t k = 0; k < _k; ++k) {
    fprintf(lambdaf,"%d\t", k);
    for (uint32_t t = 0; t < _t; ++t) {
      if (t == _t - 1)
	fprintf(lambdaf,"%.5f\n", ld[k][t]);
      else
	fprintf(lambdaf,"%.5f\t", ld[k][t]);
    }
  }
  fclose(lambdaf);
}

#ifdef COMPUTE_GROUPS

void
FastAMM::compute_and_log_groups()
{
  FILE *groupsf = fopen(Env::file_str("/groups.txt").c_str(), "w");
  FILE *summaryf = fopen(Env::file_str("/summary.txt").c_str(), "a");
  FILE *commf = fopen(Env::file_str("/communities.txt").c_str(), "w");
  FILE *sizef = fopen(Env::file_str("/communities_size.txt").c_str(), "w");
  FILE *mf = fopen(Env::file_str("/mcount.txt").c_str(), "w");
  FILE *aggf = fopen(Env::file_str("/aggregate.txt").c_str(), "w");
  //FILE *mmf = fopen(Env::file_str("/memberships.txt").c_str(), "w");


  _communities.clear();

  uint32_t unlikely = 0;
  char buf[32];
  const IDMap &seq2id = _network.seq2id();
  const IDMap &id2seq = _network.id2seq();
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
    _Epi.slice(0, i, pi_i);
    //estimate_pi(i, pi_i);
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
    Array pi_m(_k);
    Array beta(_k);
    estimate_beta(beta);
    const vector<uint32_t> *edges = _network.get_edges(i);
    for (uint32_t e = 0; e < edges->size(); ++e) {
      uint32_t m = (*edges)[e];
      if (i < m) {
	yval_t y = get_y(i,m);
	assert  (y == 1);

	_Epi.slice(0, m, pi_m);
	uint32_t max_k = 65535;
	double max = inner_prod_max(pi_i, pi_m, beta, max_k);
	//printf("max = %f\n", max);
	if (max < _link_thresh) {
	  unlikely++;
	  continue;
	}
	assert (max_k < _k);
	_communities[max_k].push_back(i);
	_communities[max_k].push_back(m);
      }
    }
    sa << groups[i] << "\n";
  }
  printf("unlikely = %d\n", unlikely);
  fflush(stdout);
  fprintf(groupsf, "%s", sa.str().c_str());

  uArray s(_k);
  std::map<uint32_t, vector<uint32_t> > comm;
  for (uint32_t i = 0; i < _n; ++i) {
    s[groups[i]]++;
    comm[groups[i]].push_back(i);
  }
  for (uint32_t i = 0; i < _k; ++i)
    fprintf(summaryf, "%d\t", s[i]);
  fprintf(summaryf, ":%d\n", unlikely);

  map<uint32_t, uint32_t> count;
  for (std::map<uint32_t, vector<uint32_t> >::const_iterator i = _communities.begin();
       i != _communities.end(); ++i) {
    //fprintf(commf, "%d\t", i->first);
    uint32_t commid = i->first;
    const vector<uint32_t> &u = i->second;

    map<uint32_t, bool> uniq;
    for (uint32_t p = 0; p < u.size(); ++p) {
      map<uint32_t, bool>::const_iterator ut = uniq.find(u[p]);
      if (ut == uniq.end()) {
	IDMap::const_iterator it = seq2id.find(u[p]);
	uint32_t id = 0;
	assert (it != seq2id.end());
	id = (*it).second;
	fprintf(commf, "%d ", id);

	_mcount[u[p]]++;
	_memberships[u[p]].push_back(commid);

	uniq[u[p]] = true;
      }
    }
    fprintf(commf, "\n");
    fprintf(sizef, "%d\t%ld\n", i->first, uniq.size());

    // arXiv 82233's communities exploration?
    if (commid == 38 || commid == 149) {
      FreqStrMap fmap;
      char buf1[512];
      map<uint32_t, bool> uniq;
      sprintf(buf1, "/cmap-%d.gml", commid);
      for (uint32_t zz = 0; zz < u.size(); ++zz) {
	map<uint32_t, bool>::const_iterator ut = uniq.find(u[zz]);
	if (ut == uniq.end()) {
	  uniq[u[zz]] = true;
	  string group = _network.gt_group(u[zz]);
	  fmap[group] += 1;
	}
      }
      FILE *g = fopen(Env::file_str(buf1).c_str(), "w");
      for (FreqStrMap::const_iterator fm = fmap.begin(); fm != fmap.end(); ++fm)
	fprintf(g, "%s\t%d\n", fm->first.c_str(), fm->second);
      fclose(g);
    }
    
  }
  
  map<uint32_t, uint32_t> agg;
  for (std::map<uint32_t, uint32_t>::const_iterator i = _mcount.begin();
       i != _mcount.end(); ++i) {
    IDMap::const_iterator it = id2seq.find(i->first);
    fprintf(mf, "%d\t%d\t%d\n", it->second, i->first, i->second);
    agg[i->second]++;
  }

  // for (MapVec::const_iterator i = _memberships.begin();
  //      i != _memberships.end(); ++i) {
  //   IDMap::const_iterator it = id2seq.find(i->first);
  //   fprintf(mmf, "%d\t", it->second);
  //   const vector<uint32_t> &mm = i->second;
  //   for (uint32_t j = 0; j < mm.size(); ++j) {
  //     fprintf(mmf, "%d", it->second);
  //     if (j == mm.size() - 1)
  // 	fprintf(mmf, "\n");
  //     else
  // 	fprintf(mmf, "\t");
  //   }
  // }

  for (std::map<uint32_t, uint32_t>::const_iterator i = agg.begin();
       i != agg.end(); ++i)
    fprintf(aggf, "%d\t%d\n", i->first, i->second);

  fprintf(summaryf,"\n");
  fclose(groupsf);
  fclose(summaryf);
  fclose(commf);
  fclose(sizef);
  fclose(mf);
  fclose(aggf);

  if (_env.nmi) {
    if (!_env.benchmark) {
      char cmd[1024];
      sprintf(cmd, "/u/pgopalan/mutual %s %s >> %s",
	      _env.ground_truth_fname.c_str(),
	      Env::file_str("/communities.txt").c_str(),
	      Env::file_str("/mutual.txt").c_str());
      if (system(cmd) < 0)
	lerr("error spawning cmd %s:%s", cmd, strerror(errno));
    } else {
      char cmd[1024];
      sprintf(cmd, "/u/pgopalan/mutual %s %s >> %s",
	      Env::file_str("/ground_truth.txt").c_str(),
	      Env::file_str("/communities.txt").c_str(),
	      Env::file_str("/mutual.txt").c_str());
      if (system(cmd) < 0)
	lerr("error spawning cmd %s:%s", cmd, strerror(errno));
    }
  }
}

#endif

void
FastAMM::opt_process(vector<uint32_t> &nodes)
{
  _start_node = gsl_rng_uniform_int(_r, _n);
  double **ldt = _lambdat.data();

  // full inference only around the neighborhood _start_node
  set_dir_exp(_start_node, _gamma, _Elogpi);
  _gammat.zero(_start_node);
  nodes.push_back(_start_node);

  const vector<uint32_t> *edges = _network.get_edges(_start_node);
  bool singleton = false;
  if (!edges)  // singleton node
    singleton = true;

  uint32_t l_size = 0;
  if (!singleton) {
    for (uint32_t i = 0; i < edges->size(); ++i) {
      uint32_t a = (*edges)[i];

      Edge e(_start_node,a);
      Network::order_edge(_env, e);
      if (!edge_ok(e))
	continue;

      l_size++;

      nodes.push_back(a);
      set_dir_exp(a, _gamma, _Elogpi);
      _gammat.zero(a);

      uint32_t p = e.first;
      uint32_t q = e.second;

      assert (p != q);
      yval_t y = get_y(p, q);

      mark_seen(p,q);
      assert (y == 1);

#ifdef PERF
      struct timeval tv3,tv4,r;
      gettimeofday(&tv3, NULL);
#endif

      _pcomp.reset(p,q,y);
      _pcomp.update_phis_until_conv();

#ifdef PERF
      gettimeofday(&tv4, NULL);
      timeval_subtract(&r, &tv4, &tv3);
      timeval_add(&s, &r);
#endif

      const Array &phi1 = _pcomp.phi1();
      const Array &phi2 = _pcomp.phi2();

      _gammat.add_slice(p, phi1);
      _gammat.add_slice(q, phi2);

      for (uint32_t k = 0; k < _k; ++k)
	for (uint32_t t = 0; t < _t; ++t)
	  ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
    }
  }

#ifdef PERF
  debug("time for %ld edge updates = %ld:%ld\n", edges->size(),
	s.tv_sec, s.tv_usec);
  s.tv_sec = 0;
  s.tv_usec = 0;
#endif

  vector<uint32_t> *zeros = _network.sparse_zeros(_start_node);

  if (zeros)
    debug("%d: zeros size:%ld\n", _start_node, zeros->size());
  else
    debug("no zeros for %d\n", _start_node);

  uint32_t nl_inf_size = 0;
  for (uint32_t i = 0; zeros && i < zeros->size(); ++i) {
    uint32_t a = (*zeros)[i];
    assert (a != _start_node);

    yval_t y = get_y(_start_node, a);
    assert ((y & 0x01) == 0);

    Edge e(_start_node,a);
    Network::order_edge(_env, e);
    if (!edge_ok(e))
      continue;

    set_dir_exp(a, _gamma, _Elogpi);
    nodes.push_back(a);
    _gammat.zero(a);

    uint32_t p = e.first;
    uint32_t q = e.second;

    mark_seen(p,q);
    nl_inf_size++;

#ifdef PERF
    struct timeval tv3,tv4,r;
    gettimeofday(&tv3, NULL);
#endif

    _pcomp.reset(p,q,y);
    _pcomp.update_phis_until_conv();

#ifdef PERF
    gettimeofday(&tv4, NULL);
    timeval_subtract(&r, &tv4, &tv3);
    timeval_add(&s, &r);
#endif

    const Array &phi1 = _pcomp.phi1();
    const Array &phi2 = _pcomp.phi2();

    _gammat.add_slice(p, phi1);
    _gammat.add_slice(q, phi2);

    for (uint32_t k = 0; k < _k; ++k)
      for (uint32_t t = 0; t < _t; ++t)
	ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
  }
  _neighbors[_iter % _env.reportfreq] = nl_inf_size;

#ifdef PERF
  debug("time for %ld non-edge updates = %ld:%ld\n",
	zeros->size(),
	s.tv_sec, s.tv_usec);
#endif
}

void
FastAMM::opt_process_noninf(vector<uint32_t> &nodes)
{
  _start_node = gsl_rng_uniform_int(_r, _n);
  double **ldt = _lambdat.data();

  // full inference only around the non-inf set around _start_node
  set_dir_exp(_start_node, _gamma, _Elogpi);
  _gammat.zero(_start_node);
  nodes.push_back(_start_node);

  const vector<uint32_t> *zeros = _network.sparse_zeros(_start_node);
  NodeMap inf_map;
  for (uint32_t i = 0; i < zeros->size(); ++i)
    inf_map[(*zeros)[i]] = true;

  vector<Edge> sample;
  double v = (double)(gsl_rng_uniform_int(_r, _n)) / _noninf_setsize;
  uint32_t q = ((int)v) * _noninf_setsize;
  tst("\nq = %d, set size = %d\n", q, _noninf_setsize);

  while (sample.size() < _noninf_setsize) {
    uint32_t node = _shuffled_nodes[q];
    if (node == _start_node) {
      q = (q + 1) % _n;
      continue;
    }
    
    yval_t y = get_y(_start_node, node);
    Edge e(_start_node, node);
    Network::order_edge(_env, e);
    if (y == 0 && edge_ok(e)) {
      NodeMap::iterator itr = inf_map.find(node);
      if (!itr->second) {
	sample.push_back(e);
	tst("(%d,%d)\n", e.first, e.second);
      }
    }
    q = (q + 1) % _n;
  };

  info("noninf sample size = %ld\n", sample.size());
  for (uint32_t i = 0; i < sample.size(); ++i) {
    Edge e = sample[i];
    assert (edge_ok(e));

    uint32_t p = e.first;
    uint32_t q = e.second;
    uint32_t a;
    if (p != _start_node)
      a = p;
    else
      a = q;

    nodes.push_back(a);
    set_dir_exp(a, _gamma, _Elogpi);
    _gammat.zero(a);

    yval_t y = get_y(p, q);
    assert (y == 0);
    mark_seen(p,q);

    _pcomp.reset(p,q,y);
    _pcomp.update_phis_until_conv();

    const Array &phi1 = _pcomp.phi1();
    const Array &phi2 = _pcomp.phi2();

    _gammat.add_slice(p, phi1);
    _gammat.add_slice(q, phi2);

    for (uint32_t k = 0; k < _k; ++k)
      for (uint32_t t = 0; t < _t; ++t)
	ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
  }
}

double
FastAMM::approx_log_likelihood()
{
  const double ** const etad = _eta.const_data();
  const double * const alphad = _alpha.const_data();
  const double ** const elogbetad = _Elogbeta.const_data();
  const double ** const elogpid = _Elogpi.const_data();
  const double ** const ld = _lambda.const_data();
  const double ** const gd = _gamma.const_data();

#ifndef SPARSE_NETWORK
  const yval_t ** const yd = _network.y().const_data();
#endif

  double s = .0, v = .0;
  for (uint32_t k = 0; k < _k; ++k) {
    v = .0;
    for (uint32_t t = 0; t < _t; ++t)
      v += gsl_sf_lngamma(etad[k][t]);
    s += gsl_sf_lngamma(_eta.sum(k)) - v;

    v = .0;
    for (uint32_t t = 0; t < _t; ++t)
      v += (etad[k][t] - 1) * elogbetad[k][t];
    s += v;

    v = .0;
    for (uint32_t t = 0; t < _t; ++t)
      v += gsl_sf_lngamma(ld[k][t]);
    s -= gsl_sf_lngamma(_lambda.sum(k)) - v;

    v = .0;
    for (uint32_t t = 0; t < _t; ++t)
      v += (ld[k][t] - 1) * elogbetad[k][t];
    s -= v;
  }

  for (uint32_t p = 0; p < _n; ++p) {
    for (uint32_t q = 0; q < _n; ++q) {
      if (p >= q)
	continue;

      Edge e(p,q);
      Network::order_edge(_env, e);
      if (!edge_ok(e))
	continue;

      yval_t y = get_y(p,q);

      _pcomp.reset(p,q,y);
      _pcomp.update_phis_until_conv(); 
      const Array &phi1 = _pcomp.phi1();
      const Array &phi2 = _pcomp.phi2();

      Array Elogf(_k);
      FPhiComp::compute_Elogf(p,q,y,_k,_t,_Elogbeta,Elogf);
      double *elogfd = Elogf.data();

      for (uint32_t k = 0; k < _k; ++k)
	s += phi1[k] * phi2[k] * elogfd[k];

      if (y == 1)
	for (uint32_t g = 0; g < _k; ++g)
	  for (uint32_t h = 0; h < _k; ++h)
	    if (g != h)
	      s += phi1[g] * phi2[h] * _env.logepsilon;

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
    for (uint32_t k = 0; k < _k; ++k) {
      v += (alphad[k] - 1) * elogpid[p][k];
    }
    s += v;

    v = .0;
    for (uint32_t k = 0; k < _k; ++k) {
      double qq = gd[p][k];
      if (gd[p][k] < 1e-30)
	qq = 1e-30;
      v += gsl_sf_lngamma(qq);
    }
    s -= gsl_sf_lngamma(_gamma.sum(p)) - v;

    v = .0;
    for (uint32_t k = 0; k < _k; ++k)
      v += (gd[p][k] - 1) * elogpid[p][k];
    s -= v;
  }

  info("approx. log likelihood = %f\n", s);
  fprintf(_lf, "%d\t%d\t%.5f\n", _iter, duration(), s);
  fflush(_lf);

  double thresh = 0.00001;
  double w = s;
  if (_env.accuracy && (_iter > _n || _iter > 5000)) {
    if (w > _prev_w && _prev_w != 0 && fabs((w - _prev_w) / _prev_w) < thresh) {
      FILE *f = fopen(Env::file_str("/done.txt").c_str(), "w");
      fprintf(f, "%d\t%d\t%.5f\n", _iter, duration(), w);
      fclose(f);
      if (_env.nmi) {
	char cmd[1024];
	sprintf(cmd, "/usr/local/bin/mutual %s %s >> %s",
		Env::file_str("/ground_truth.txt").c_str(),
		Env::file_str("/communities.txt").c_str(),
		Env::file_str("/done.txt").c_str());
	if (system(cmd) < 0)
	  lerr("error spawning cmd %s:%s", cmd, strerror(errno));
      }
      //exit(0);
    }
  }
  _prev_w = w;
  return s;
}

double
FastAMM::heldout_likelihood()
{
  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  for (SampleMap::const_iterator i = _heldout_map.begin();
       i != _heldout_map.end(); ++i) {

    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

#ifndef SPARSE_NETWORK
    const yval_t ** const yd = _network.y().const_data();
    yval_t y = yd[p][q] & 0x01;
    bool seen = yd[p][q] & 0x80;
#else
    yval_t y = _network.y(p,q);
    bool seen = false; // TODO: fix heldout for sparse network
#endif

    assert (!seen);
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
    info("edge likelihood for (%d,%d) is %f\n", p,q,u);
  }
  double nshol = (_zeros_prob * (szeros / kzeros)) + (_ones_prob * (sones / kones));
  fprintf(_hf, "%d\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%.9f\t%.9f\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones,
	  _zeros_prob * (szeros / kzeros),
	  _ones_prob * (sones / kones),
	  nshol);
  fflush(_hf);

  // Use hol @ network sparsity as stopping criteria
  double a = nshol;

  bool stop = false;
  int why = -1;
  if (_iter > _n || _iter > 5000) {
    if (a > _prev_h && _prev_h != 0 && fabs((a - _prev_h) / _prev_h) < 0.00001) {
      stop = true;
      why = 0;
    } else if (a < _prev_h)
      _nh++;
    else if (a > _prev_h)
      _nh = 0;

    if (a > _max_h)
      _max_h = a;

    if (_nh > 2) {
      why = 1;
      stop = true;
    }
  }
  _prev_h = a;

  if (stop) {
    uint32_t v10 = 0, v100 = 0, v1000 = 0;

#ifdef PRECISION_SAMPLE    
    compute_precision(v10, v100, v1000);
#endif

    double v = validation_likelihood();
    double t = 0;
#ifdef TRAINING_SAMPLE
    t = training_likelihood();
#endif

    FILE *f = fopen(Env::file_str("/max.txt").c_str(), "w");
    fprintf(f, "%d\t%d\t%.5f\t%.5f\t%.5f\t%.5f\t%d\t%d\t%d\t%d\n",
	    _iter, duration(),
	    a, t, v, _max_h,
	    v10, v100, v1000,
	    why);
    fclose(f);

    if (_env.use_validation_stop) {
      do_on_stop();
      exit(0);
    }
  }
  return s / k;
}

#ifdef PRECISION_SAMPLE
void
FastAMM::compute_precision(uint32_t &c10, uint32_t &c100,
			   uint32_t &c1000)
{
  if (_env.accuracy)
    return;
  char buf[128];
  sprintf(buf, "/hitcurve_%d.txt", _hitcurve_id);
  _hcf = fopen(Env::file_str(buf).c_str(), "w");
  if (!_hcf)  {
    lerr("cannot open hitcurve file:%s\n",  strerror(errno));
    exit(-1);
  }
  _hitcurve_id++;

  uint32_t k = 0;
  D1Array<EdgeV> v(_precision_map.size());

  // compute likelihood for all pairs in precision set
  for (SampleMap::const_iterator i = _precision_map.begin();
       i != _precision_map.end(); ++i) {

    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

    double u1 = edge_likelihood(p,q,1);
    double p1 = exp(u1);
    debug("prob. that pair is 1:%.5f\n", p1);

    v[k++] = EdgeV(e,p1);
  }

  // rank the pairs in desc order of likelihood
  v.sort_by_value();

  c10 = 0, c100 = 0, c1000 = 0;
  uint32_t step_size = 1000;
  uint32_t hits = 0;
  for (uint32_t i = 0; i < v.size(); ++i) {
    const Edge &e = v[i].first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

#ifndef SPARSE_NETWORK
    const yval_t ** const yd = _network.y().const_data();
    yval_t true_y = yd[p][q] & 0x01;
#else
    yval_t true_y = _network.y(p,q);
#endif

    if (true_y == 1) { // hit
      hits++;
      if (i < 10)
	c10++;
      else if (i < 100)
	c100++;
      else if (i < 1000)
	c1000++;
    }
    if ( i == 0 || (i+1) % step_size == 0) {
      fprintf(_hcf, "%d\t%d\n", i+1, hits);
      fflush(_hcf);
    }
  }
  c100 += c10;
  c1000 += c100;
  fprintf(_pf, "%d\t%d\t%d\t%d\t%d\n", _iter, duration(), c10, c100, c1000);
  fflush(_pf);
  fclose(_hcf);
}
#endif

double
FastAMM::validation_likelihood()
{
  if (_env.accuracy)
    return .0;

  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  for (SampleMap::const_iterator i = _validation_map.begin();
       i != _validation_map.end(); ++i) {

    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

#ifndef SPARSE_NETWORK
    const yval_t ** const yd = _network.y().const_data();
    yval_t y = yd[p][q] & 0x01;
    bool seen = yd[p][q] & 0x80;
#else
    yval_t y = _network.y(p,q);
    bool seen = false; // TODO: fix heldout for sparse network
#endif

    assert (!seen);
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
  double nshol = (_zeros_prob * (szeros / kzeros)) + (_ones_prob * (sones / kones));
  fprintf(_vf, "%d\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%.9f\t%.9f\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones,
	  _zeros_prob * (szeros / kzeros),
	  _ones_prob * (sones / kones),
	  nshol);
  fflush(_vf);

  return s / k;
}

#ifdef PRECISION_SAMPLE
void
FastAMM::compute_adamic_adar_score()
{
  FILE *ahcf = fopen(Env::file_str("/ad_hitcurve.txt").c_str(), "w");
  if (!ahcf)  {
    lerr("cannot open hitcurve file:%s\n",  strerror(errno));
    exit(-1);
  }

  uint32_t k = 0;
  D1Array<EdgeV> v(_precision_map.size());

  // compute scores for all pairs in precision set
  for (SampleMap::const_iterator i = _precision_map.begin();
       i != _precision_map.end(); ++i) {

    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

    // get neighbors of p and q
    const vector<uint32_t> *pn = _network.get_edges(p);
    const vector<uint32_t> *qn = _network.get_edges(q);

    double score = .0;
    for (uint32_t j = 0; j < pn->size(); ++j) {
      uint32_t pnz = (*pn)[j];
      if (pnz == q)
	continue;
      assert (pnz != p);
      for (uint32_t k = 0; k < qn->size(); ++k) {
	uint32_t qnz = (*qn)[k];
	if (qnz == p)
	  continue;

	if (pnz == qnz) {
	  const vector<uint32_t> *zn = _network.get_edges(pnz);
	  // z has at least two edges (to p and q)
	  assert (zn->size() >= 2);
	  score += 1 / log(zn->size());
	}
      }
    }
    v[k++] = EdgeV(e,score);
  }

  // rank the pairs in desc order of likelihood
  v.sort_by_value();

  uint32_t c10 = 0, c100 = 0, c1000 = 0;
  uint32_t step_size = 1000;
  uint32_t hits = 0;
  for (uint32_t i = 0; i < v.size(); ++i) {
    const Edge &e = v[i].first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

#ifndef SPARSE_NETWORK
    const yval_t ** const yd = _network.y().const_data();
    yval_t true_y = yd[p][q] & 0x01;
#else
    yval_t true_y = _network.y(p,q);
#endif

    if (true_y == 1) { // hit
      hits++;
      if (i < 10)
	c10++;
      else if (i < 100)
	c100++;
      else if (i < 1000)
	c1000++;
    }
    if ( i == 0 || (i+1) % step_size == 0) {
      fprintf(ahcf, "%d\t%d\n", i+1, hits);
      fflush(ahcf);
    }
  }
  fclose(ahcf);

  c100 += c10;
  c1000 += c100;
  FILE *adf = fopen(Env::file_str("/adamic_adar.txt").c_str(), "w");
  if (!adf)  {
    lerr("cannot open adamic adar file:%s\n",  strerror(errno));
    exit(-1);
  }
  fprintf(adf, "%d\t%d\t%d\n", c10, c100, c1000);
  fclose(adf);
}
#endif

#ifdef TRAINING_SAMPLE
double
FastAMM::training_likelihood()
{
  uint32_t k = 0, kzeros = 0, kones = 0;
  double s = .0, szeros = 0, sones = 0;
  uint32_t c = 0;

  for (SampleMap::const_iterator i = _training_map.begin();
       i != _training_map.end(); ++i) {
    const Edge &e = i->first;
    uint32_t p = e.first;
    uint32_t q = e.second;
    assert (p != q);

#ifndef SPARSE_NETWORK
    const yval_t ** const yd = _network.y().const_data();
    yval_t y = yd[p][q] & 0x01;
    bool seen = yd[p][q] & 0x80;
#else
    yval_t y = _network.y(p,q);
    bool seen = false;
#endif

    if (!seen)
      c++;
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
  fprintf(_trf, "%d\t%d\t%.5f\t%d\t%.5f\t%d\t%.5f\t%d\t%d\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones,c);
  fflush(_trf);

  double w = s / k;

  double thresh = _env.stopthresh;
  bool stop = false;
  if (_env.accuracy && (_iter > _n || _iter > 5000) && !_training_done) {
    if (w > _prev_t && _prev_t != 0 && fabs((w - _prev_t) / _prev_t) < thresh) {
      stop = true;
    } else if (w > _max_t) {
      _max_t = w;
      _nt = 0;
    } else if (w < _max_t) {
      _nt++;
    }
    if (_nt > 3)
      stop = true;
  }

  if (stop) {
    FILE *f = fopen(Env::file_str("/donet.txt").c_str(), "w");
    fprintf(f, "%d\t%d\t%.5f\n", _iter, duration(), w);
    fclose(f);

    if (_env.nmi) {
      char cmd[1024];
	sprintf(cmd, "/usr/local/bin/mutual %s %s >> %s",
		Env::file_str("/ground_truth.txt").c_str(),
		Env::file_str("/communities.txt").c_str(),
		Env::file_str("/donet.txt").c_str());
	if (system(cmd) < 0)
	  lerr("error spawning cmd %s:%s", cmd, strerror(errno));
    }
    if (_env.use_training_stop)
      exit(0);
    _training_done = true;
  }
  _prev_t = w;
  return w;
}
#endif


void
FastAMM::moving_heldout_likelihood(EdgeList &sample) const
{
  if (_env.accuracy)
    return;

  uint32_t p, q;
  double lones = .0, lzeros = .0;
  uint32_t kones = 0, kzeros = 0;
  double s = .0;
  uint32_t k = 0;
  for (EdgeList::const_iterator i = sample.begin(); i != sample.end(); ++i) {
    p = i->first;
    q = i->second;
    assert (p != q);

#ifndef SPARSE_NETWORK
    const yval_t ** const yd = _network.y().const_data();
    yval_t y = yd[p][q] & 0x01;
    bool seen = yd[p][q] & 0x80;
#else
    yval_t y = _network.y(p,q);
    bool seen = false; // TODO sparse network heldout
#endif

    if (not seen) {
      double l = .0;
      l = edge_likelihood(p,q,y);
      s += l;
      if (y == 1) {
	lones += l;
	kones++;
      } else {
	lzeros += l;
	kzeros++;
      }
      k += 1;
    }
  }

  double h = .0;
  if (k > 0)
    h = s / k;
  else
    h = _env.illegal_likelihood;
  double hones = .0;
  if (kones > 0)
    hones = lones / kones;
  else
    hones = _env.illegal_likelihood;
  double hzeros = .0;
  if (kzeros > 0)
    hzeros = lzeros / kzeros;
  else
    hzeros = _env.illegal_likelihood;

  if (h != _env.illegal_likelihood) {
    fprintf(_statsf, "%d\t%d\t%.5f\t%.5f\t%.5f\t%d\t%d\n",
	    _iter, duration(), h, hones, hzeros, kones, kzeros);
    fflush(_statsf);
  }
}

int
FastAMM::load_model()
{
  fprintf(stderr, "+ loading gamma\n");
  double **gd = _gamma.data();
  FILE *gammaf = fopen("gamma.txt", "r");
  if (!gammaf) {
    fprintf(stderr, "no gamma.txt found\n");
    return -1;
  }
  uint32_t n = 0;
  int sz = 32*_k;
  char *line = (char *)malloc(sz);
  while (!feof(gammaf)) {
    if (fgets(line, sz, gammaf) == NULL)
      break;
    //assert (fscanf(gammaf, "%[^\n]", line) > 0);
    debug("line = %s\n", line);
    uint32_t k = 0;
    char *p = line;
    do {
      char *q = NULL;
      double d = strtod(p, &q);
      if (q == p) {
	if (k < _k - 1) {
	  fprintf(stderr, "error parsing gamma file\n");
	  assert(0);
	}
	break;
      }
      p = q;
      if (k >= 2) // skip node id and seq
	gd[n][k-2] = d;
      k++;
    } while (p != NULL);
    n++;
    debug("read %d lines\n", n);
    memset(line, 0, sz);
  }
  assert (n == _n);
  fclose(gammaf);
  memset(line, 0, sz);

  fprintf(stderr, "+ loading lambda\n");
  double **ld = _lambda.data();
  FILE *lambdaf = fopen("lambda.txt", "r");
  if (!lambdaf) {
    lerr("no lambda file found\n");
    return -1;
  }
  uint32_t k = 0;
  while (!feof(lambdaf)) {
    if (fgets(line, sz, lambdaf) == NULL)
      break;
    debug("line = %s\n", line);
    uint32_t t = 0;
    char *p = line;
    do {
      char *q = NULL;
      double d = strtod(p, &q);
      if (q == p) {
	if (t < _t - 1) {
	  fprintf(stderr, "error parsing gamma file\n");
	  assert(0);
	}
	break;
      }
      p = q;
      if (t >= 1) // skip seq
	ld[k][t-1] = d;
      t++;
    } while (p != NULL);
    k++;
    debug("read %d lines\n", n);
    memset(line, 0, sz);
  }
  assert (k == _k);
  fclose(lambdaf);
  free(line);

  return 0;
}

void
FastAMM::compute_stats()
{
#ifdef PRECISION_SAMPLE
  info("computing precision\n");
  uint32_t c10, c100, c1000;
  compute_precision(c10, c100, c1000);
#endif

  info("computing heldout likelihood\n");
  heldout_likelihood();

#ifdef COMPUTE_GROUPS
  info("computing groups\n");
  estimate_all_pi();
  compute_and_log_groups();
#endif
}

#ifdef EGO_NETWORK

void
FastAMM::group_stats(const uArray &nodes, uint32_t M)
{
  for (uint32_t i = 0; i < M; ++i) {
    char buf1[512];
    sprintf(buf1, "/groupstats-%d.gml", nodes[i]);
    
    const IDMap &m = _network.id2seq();
    const IDMap::const_iterator idt = m.find(nodes[i]);
    uint32_t a = idt->second;
    
    FreqStrMap fmap;
    const vector<uint32_t> *v = _network.get_edges(a);
    if (v)  {
      for (uint32_t j = 0; j < v->size(); ++j) {
	string group = _network.gt_group((*v)[j]);
	fmap[group] += 1;
      }
    }
    FILE *g = fopen(Env::file_str(buf1).c_str(), "w");
    for (FreqStrMap::const_iterator fm = fmap.begin(); fm != fmap.end(); ++fm)
      fprintf(g, "%s\t%d\n", fm->first.c_str(), fm->second);
    fclose(g);
  }
}

void
FastAMM::ego(const uArray &nodes, 
	     uint32_t M)
{
  uint32_t ROOT_TOP_K = 10; // 4 for arXiv, 10 for patents?
  for (uint32_t i = 0; i < M; ++i) {
    char buf[512];
    sprintf(buf, "/ego-%d.gml", nodes[i]);
    NodeMap uniq;
    BoolMap tc;

    const IDMap &m = _network.id2seq();
    const IDMap::const_iterator idt = m.find(nodes[i]);
    assert (idt != m.end());
    
    FILE *f = fopen(Env::file_str(buf).c_str(), "w");
    fprintf(f, "graph\n[\n\tdirected 0\n");
    _ego_smap.clear();
    _comm_subgraph.clear();

    explore(f, true, 0, idt->second, uniq, tc, ROOT_TOP_K, 0);
    
    MapVec _comm_groups;
    // we have a subgraph consisting of ROOT_TOP_K communities
    for (MapVec::const_iterator itr = _comm_subgraph.begin(); 
	 itr != _comm_subgraph.end(); ++itr) {
      Array pi_a(_k);
      Array pi_b(_k);
      uint32_t comm = itr->first;
      const vector<uint32_t> &l = itr->second;
      // consider edges between all nodes
      // XXX add the ROOT itself
      printf("considering edges between %ld nodes\n", l.size());
      for (uint32_t ni = 0; ni < l.size(); ++ni) 
	for (uint32_t nj = 0; nj < l.size(); ++nj) {
	  
	  if (get_y(l[ni],l[nj]) == 0)
	    continue;
	  
	  Edge e(l[ni],l[nj]);
	  Network::order_edge(_env, e);
	  SampleMap::const_iterator ei = _ego_smap.find(e);
	  if (ei != _ego_smap.end() && ei->second)
	    continue; // already seen this edge
	  
	  Array beta(_k);
	  estimate_beta(beta);
	  get_Epi(l[ni], pi_a);
	  get_Epi(l[nj], pi_b);

	  //_Epi.slice(0, l[ni], pi_a);
	  //_Epi.slice(0, l[nj], pi_b);
    
	  uint32_t max_k = 65535;
	  double max = inner_prod_max(pi_a, pi_b, beta, max_k);
	  
	  if (max_k == comm) {
	    write_edge_gml(f, l[ni], l[nj], max_k, max); // XXX
	    _ego_smap[e] = true;
	  }
	}

      char buf1[512];
      sprintf(buf1, "/cmap-%d-%d.gml", nodes[i], comm);
      FreqStrMap fmap;
      for (uint32_t ni = 0; ni < l.size(); ++ni) {
	string group = _network.gt_group(l[ni]);
	fmap[group] += 1;
      }
      FILE *g = fopen(Env::file_str(buf1).c_str(), "w");
      for (FreqStrMap::const_iterator fm = fmap.begin(); fm != fmap.end(); ++fm)
	fprintf(g, "%s\t%d\n", fm->first.c_str(), fm->second);
      fclose(g);
    }
    fprintf(f, "\t]\n");
    fclose(f);
  }
}

void
FastAMM::explore(FILE *f, bool root, uint32_t parent, 
		 uint32_t a, NodeMap &uniq, BoolMap &tc, 
		 uint32_t top_k, uint32_t depth)
{
  if (uniq[a])
    return;
  
  uint32_t max_k = 65535;
  if (!root) {
    Array pi_a(_k);
    Array pi_b(_k);

    Edge e(parent,a);
    Network::order_edge(_env, e);
    SampleMap::const_iterator i = _ego_smap.find(e);
    if (i != _ego_smap.end() && i->second)
      return; // already seen this edge
    
    Array beta(_k);
    estimate_beta(beta);
    
    get_Epi(parent, pi_a);
    get_Epi(a, pi_b);

    //_Epi.slice(0, parent, pi_a);
    //_Epi.slice(0, a, pi_b);
    
    double max = inner_prod_max(pi_a, pi_b, beta, max_k);

    if (tc[max_k]) { // in top_k communities
      //printf("node %d is in comm %d\n", a, max_k);
      write_node_gml(f, a, max_k);
      write_edge_gml(f, parent, a, max_k, max);
      uniq[a] = true;
      _ego_smap[e] = true;
    } else
      return;
  } else {
    write_node_gml(f, a, most_likely_group(a));
    uniq[a] = true;
  }

  // get the top communities of a
  vector<uint32_t> u;
  get_top_p_communities(a, top_k, u);
  
  BoolMap tc_child; // top communities
  if (root) {
    for (uint32_t j = 0; j < u.size(); ++j) {
      printf("setting comm %d for node %d\n", u[j], a);
      tc_child[u[j]] = true;
    }
  } else 
    tc_child[max_k] = true;

  static uint32_t MAX_DEPTH = 2;
  if (depth >= MAX_DEPTH)
    return;

  const vector<uint32_t> *v = _network.get_edges(a);
  if (v)  {
    //double prob = 0.1; // patents
    // double prob = 0.01; // arxiv
    double prob = 1;
    for (uint32_t j = 0; j < v->size(); ++j) {
      uint32_t b = (*v)[j];
      uint32_t type = gsl_ran_bernoulli(_r, prob);
      if (type)
	explore(f, false, a, b, uniq, tc_child, 1, depth+1);
    }
  }
}

void
FastAMM::write_edge_gml(FILE *f, uint32_t a, 
			uint32_t b, uint32_t color, double weight)
{
  fprintf(f, "\tedge\n\t[\n");
  fprintf(f, "\t\tsource %d\n", a); 
  fprintf(f, "\t\ttarget %d\n", b);
  fprintf(f, "\t\tcolor %d\n", color);
  fprintf(f, "\t\tweight %.5f\n", weight);
  fprintf(f, "\t]\n");
}

void
FastAMM::write_node_gml(FILE *f, uint32_t a, uint32_t max_k)
{
  if (max_k < 65536)
    _comm_subgraph[max_k].push_back(a);

  //const vector<uint32_t> &v = _memberships[a];
  const IDMap &m = _network.seq2id();
  const IDMap::const_iterator idt = m.find(a);
  assert (idt != m.end());

  fprintf(f, "\tnode\n\t[\n");
  fprintf(f, "\t\tid %d\n", a);
  if (_env.strid) {
    const StrMapInv &s = _network.id2str();
    const StrMapInv::const_iterator &strt = s.find(a);
    assert (strt != s.end());
    fprintf(f, "\t\ttextid \"%s\"\n", strt->second.c_str());
  }
  fprintf(f, "\t\textid %d\n", idt->second);

  //fprintf(f, "\t\tmemberships\t[ ");
  //for (uint32_t j = 0; j < v.size(); ++j)
  //fprintf(f, "%d %d ", j, v[j]);
  //fprintf(f, "]\n");

  fprintf(f, "\t\tgroup %d\n", max_k);
  fprintf(f, "\t\tbridgeness %.5f\n", bridgeness(a));
  //fprintf(f, "\t\tinfluence %d\n", _communities[g].deg(a));
  fprintf(f, "\t\tdegree %d\n", _network.deg(a));
  fprintf(f, "\t]\n");
}

inline uint32_t
FastAMM::most_likely_group(uint32_t p)
{
  Array Epi(_k);
  get_Epi(p, Epi);
  double max_k = .0, max_p = .0;
  
  for (uint32_t k = 0; k < _k; ++k)
    if (Epi[k] > max_p) {
      max_p = Epi[k];
      max_k = k;
    }
  return max_k;
}

double
FastAMM::bridgeness(uint32_t a)
{
  Array Epi(_k);
  get_Epi(a, Epi);
  const double * const pid = Epi.const_data();

  double v = .0;
  for (uint32_t k = 0; k < _k; ++k)
    v += (pid[k] - ((double)1.0 / _k))*(pid[k] - ((double)1.0 / _k));
  v = (1 - sqrt(v * ((double)_k) / (_k - 1))) * _network.deg(a);
  return v;
}
#endif


