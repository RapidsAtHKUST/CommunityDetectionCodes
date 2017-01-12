#include "mmsbinfer.hh"
#include "log.hh"
#include <sys/time.h>

int MMSBInfer::ea = 0;
int MMSBInfer::eb = 0;

MMSBInfer::MMSBInfer(Env &env, Network &network)
  :_env(env), _network(network),
   _n(env.n), _k(env.k),
   _t(env.t), _s(env.s),
   _alpha(_k),
   _family(0), _prev_mbsize0(_s), _prev_mbsize1(_s),
   _eta(_k,_t),
   _gamma(_n,_k), _lambda(_k,_t),
   _gammanext(_n,_k), _lambdanext(_k,_t),
   _tau0(env.tau0 + 1), _kappa(env.kappa),
   _nodetau0(env.nodetau0 + 1), _nodekappa(env.nodekappa),
   _rhot(.0), _noderhot(_n), _nodec(_n),
   _nodeupdatec(_n),
   _nodeupdate_rn(_n),
   _start_time(time(0)),
   _Elogpi(_n,_k), _Elogbeta(_k,_t), _Epi(_n,_k),
   _gammat(_n,_k), _szgammat(_n,_k),
   _lambdat(_k,_t), _count(_n),
   _Elogf(_k),
   _pcomp(env, _iter, _n, _k, _t, 0, 0, 0,
	  _Elogpi, _Elogbeta, _Elogf),
   _nthreads(env.nthreads),
   _delaylearn_reported(false),
   _max_t(-2147483647),
   _max_h(-2147483647),
   _max_v(-2147483647),
   _prev_h(-2147483647),
   _prev_w(-2147483647),
   _prev_t(-2147483647),
   _nh(0), _nt(0),
   _training_done(false),
   _start_node(0), 
   _n1(0), _n2(0), _n3(0),
   _cn(0), _skipped(0),
   _neighbors(_env.reportfreq)
{
  PhiComp::static_initialize(_n, _k);

  fprintf(stdout, "+ initialization begin\n");
  fflush(stdout);
  ea = 0;
  eb = 0;

  fprintf(stdout, "+ running inference on %d nodes\n", _n);
  Env::plog("inference n", _n);
  _alpha.set_elements(env.alpha);

  double **d = _eta.data();
  for (uint32_t i = 0; i < _eta.m(); ++i) {
    d[i][0] = env.eta0;
    d[i][1] = env.eta1;
  }
  Env::plog("eta", _eta);

  // random number generation
  gsl_rng_env_setup();
  const gsl_rng_type *T = gsl_rng_default;
  _r = gsl_rng_alloc(T);
  if (_env.seed)
    gsl_rng_set(_r, _env.seed);

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

  _tef = fopen(Env::file_str("/training-edges.txt").c_str(), "w");
  if (!_tef)  {
    printf("cannot open training edges file:%s\n",  strerror(errno));
    exit(-1);
  }


  init_heldout();
  printf("+ heldout sets created\n");

  if (_env.model_load)
    assert(load_model() >= 0);
  else {
    init_gamma();
    assert (init_lambda() >= 0);
  }

  debug("gamma = %s", _gamma.s().c_str());
  debug("lambda = %s", _lambda.s().c_str());

  // initialize expectations
  set_dir_exp(_gamma, _Elogpi);
  set_dir_exp(_lambda, _Elogbeta);

  debug("Elogpi = %s", _Elogpi.s().c_str());
  debug("Elogbeta = %s", _Elogbeta.s().c_str());

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

  _cmapf = fopen(Env::file_str("/cmap.txt").c_str(), "w");
  if (!_cmapf)  {
    printf("cannot open cmap file:%s\n",  strerror(errno));
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

  _trf = fopen(Env::file_str("/training.txt").c_str(), "w");
  if (!_trf)  {
    printf("cannot open training file:%s\n",  strerror(errno));
    exit(-1);
  }

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

  double a, b, c;
  heldout_likelihood(a, b, c);
  validation_likelihood(a, b, c);

#ifdef TRAINING_SAMPLE
  training_likelihood(a, b, c);
#endif

  //compute_modularity();
  
  if (_env.logl)
    approx_log_likelihood();

  if (_nthreads > 0) {
    Thread::static_initialize();
    PhiRunner::static_initialize();
    start_threads();
  }
  fprintf(stdout, "+ initialization end\n");
  fflush(stdout);
  gettimeofday(&_last_iter, NULL);
  _start_time = time(0);
}

MMSBInfer::~MMSBInfer()
{
  fclose(_statsf);
  fclose(_hf);
  fclose(_vf);
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
MMSBInfer::init_heldout()
{
  int s = _env.heldout_ratio * _network.ones();
  set_heldout_sample(s);
  set_validation_sample(s);

#ifdef TRAINING_SAMPLE
  set_training_sample(2*(_network.ones() - s));
#endif

  Env::plog("heldout ratio", _env.heldout_ratio);
  Env::plog("heldout edges (1s and 0s)", _heldout_map.size());
  fprintf(_hef, "%s\n", edgelist_s(_heldout_edges).c_str());
  fprintf(_vef, "%s\n", edgelist_s(_validation_edges).c_str());

#ifdef TRAINING_SAMPLE
  fprintf(_tef, "%s\n", edgelist_s(_training_edges).c_str());
#endif

  fclose(_hef);
  fclose(_vef);
  fclose(_tef);
}

string
MMSBInfer::edgelist_s(EdgeList &elist)
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

void
MMSBInfer::set_heldout_sample(int s)
{
  if (_env.accuracy)
    return;
  int c0 = 0;
  int c1 = 0;
  int p = s / 2;
  bool st = _env.stratified;
  while (c0 < p || c1 < p) {
    Edge e;
    if (_env.deterministic)
      get_edge(e, true);
    else  {
      if (c0 == p) {
	_family = 1;
	_env.stratified = true;
	get_random_edge(e, false);
      } else {
	get_random_edge(e, true);
      }
    }

    uint32_t a = e.first;
    uint32_t b = e.second;
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
  }
  _env.stratified = st;
  _family = 0;
}

void
MMSBInfer::set_validation_sample(int s)
{
  if (_env.accuracy)
    return;

  int c0 = 0;
  int c1 = 0;
  int p = s / 2;
  bool st = _env.stratified;
  while (c0 < p || c1 < p) {
    Edge e;
    if (_env.deterministic)
      get_edge(e, true);
    else  {
      if (c0 == p) {
	_family = 1;
	_env.stratified = true;
	get_random_edge(e, false);
      } else {
	get_random_edge(e, false);
      }
    }

    uint32_t a = e.first;
    uint32_t b = e.second;
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
  _env.stratified = st;
  _family = 0;
}

#ifdef TRAINING_SAMPLE
void
MMSBInfer::set_training_sample(int s)
{
  int c0 = 0;
  int c1 = 0;
  int p = s / 2;
  bool st = _env.stratified;
  while (c0 < p || c1 < p) {
    Edge e;
    if (_env.deterministic)
      get_edge(e, true);
    else  {
      if (c0 == p) {
	_family = 1;
	_env.stratified = true;
	get_random_edge(e, false);
      } else {
	get_random_edge(e, false);
      }
    }

    uint32_t a = e.first;
    uint32_t b = e.second;
    yval_t y = get_y(a,b);

    if (y == 0 and c0 < p) {
      c0++;
      _training_edges.push_back(e);
      _training_map[e] = true;
    }
    if (y == 1 and c1 < p) {
      c1++;
      _training_edges.push_back(e);
      _training_map[e] = true;
    }
  }
  _env.stratified = st;
  _family = 0;
}
#endif

void
MMSBInfer::init_gamma()
{
  double **d = _gamma.data();
  for (uint32_t i = 0; i < _n; ++i)
    for (uint32_t j = 0; j < _k; ++j)  {
      if (_env.deterministic) {
	d[i][j] = 0.09 + (0.01 * ((i+1) / ( i + j + 1)));
	if (d[i][j] > 1.) {
	  d[i][j] = 0.9;
	}
      } else
	d[i][j] = gsl_ran_gamma(_r, 100, 1./100);
    }
  _gammanext.set_elements(_env.alpha);
}

int
MMSBInfer::init_lambda()
{
  if (_lambda.copy_from(_eta) < 0 || _lambdanext.copy_from(_eta) < 0) {
    lerr("init lambda failed");
    return -1;
  }
  return 0;
}

int
MMSBInfer::start_threads()
{
  for (uint32_t i = 0; i < _nthreads; ++i) {
    PhiRunner *t = new PhiRunner(_env, _network,
				 _iter, _n, _k, _t,
				 _family,
				 _Elogpi, _Elogbeta,
				 _out_q, _in_q,
				 _cm, _rest);
    if (t->create() < 0)
      return -1;
    _thread_map[t->id()] = t;
  }
  return 0;
}

bool
MMSBInfer::all_threads_done()
{
  uint16_t c = 0;
  for(ThreadMap::const_iterator i = _thread_map.begin();
      i != _thread_map.end(); ++i) {
    PhiRunner *t = i->second;
    if (t->done())
      c++;
    else
      return false;
  }
  return c == _nthreads;
}

int
MMSBInfer::join_all_threads()
{
  vector<pthread_t> ids;
  int c = 0;
  for(ThreadMap::const_iterator i = _thread_map.begin();
      i != _thread_map.end(); ++i) {
    PhiRunner *t = i->second;
    if (t->join() >= 0) {
      c++;
      ids.push_back(t->id());
    }
  }
  if (c == _nthreads) {
    for (int i = 0; i < _nthreads; ++i) {
      ThreadMap::iterator k = _thread_map.find(ids[i]);
      PhiRunner *t = k->second;
      _thread_map.erase(ids[i]);
      delete t;
    }
  } else {
    fprintf(stderr, "Error: cannot shutdown all threads\n");
    return -1;
  }
  return 0;
}

void
MMSBInfer::infer()
{
  EdgeList sample;
  bool rep = false;

  while (1) {

    if (_env.max_iterations && _iter > _env.max_iterations) {
      printf("+ Quitting: reached max iterations.\n");
      Env::plog("maxiterations reached", true);
      exit(0);
    }

#ifdef MRSTATS
    gettimeofday(&_rt.init_tv_before, NULL);
#endif

    sample.clear();

#if PERF
    struct timeval tv,tv0,res;
    gettimeofday(&tv0, NULL);
#endif

    uint32_t mbsize = _s;
    if (_family == 0)
      mbsize = _s / _env.m;

    get_subsample(sample, mbsize);

    // printf("--> processing mini-batch with %ld edges", sample.size());
    // printf("--> start_node:%d with %d ones\n",
    //  _start_node, _network.deg(_start_node));

#if PERF
    gettimeofday(&tv, NULL);
    timeval_subtract(&res, &tv, &tv0);
    printf("get_subsample took %ld:%ld secs\n", res.tv_sec, res.tv_usec);
#endif

    debug("--> processing sample = %s", edgelist_s(sample).c_str());
    info("iter = %d, tau0 = %.2f, kapp = %.2f", _iter, _tau0, _kappa);
    info("rhot = %.2f", _rhot);

    if (_iter % _env.reportfreq == 0 || (_env.stratified && rep)) {
      if (_env.stratified) {
	if (!rep)
	  rep = true;
	else
	  rep = false;
      }
      //moving_heldout_likelihood(sample);
    }

    _gammat.zero();
    _lambdat.zero();
    _szgammat.zero();

    //_count.zero();
    //printf("COUNT = %s\n", _count.s().c_str());

    set_dir_exp(_gamma, _Elogpi);
    set_dir_exp(_lambda, _Elogbeta);

    if (_nthreads > 0) {
#ifdef PERF
      struct timeval tv1,tv2,result;
      gettimeofday(&tv1, NULL);
#endif

      multithreaded_process(sample);

#ifdef PERF
      gettimeofday(&tv2, NULL);
      timeval_subtract(&result, &tv2, &tv1);
      printf("multithreaded sample took %ld:%ld secs\n",
	     result.tv_sec, result.tv_usec);
#endif
    } else {
#ifdef PERF
      struct timeval tv1,tv2,result;
      gettimeofday(&tv1, NULL);
#endif

      if (!_env.infthresh)
	process(sample);
      else if (_env.nonuniform)
	opt_nonuniform_process(sample);
      else
	opt_process(sample);

#ifdef PERF
      gettimeofday(&tv2, NULL);
      timeval_subtract(&result, &tv2, &tv1);
      printf("single process sample took %ld:%ld secs\n",
	     result.tv_sec, result.tv_usec);
#endif
    }

#ifdef PERF
    struct timeval tv1,tv2,result;
    gettimeofday(&tv1, NULL);
#endif

    info("nodec = %s\n", _nodec.s().c_str());
    double scale = 0;
    if (!_env.randomnode) {
      scale = _total_pairs;
      if (_env.stratified)
	scale = _total_pairs * (_family ? _ones_prob : _zeros_prob);
    } else { // random node
      if (_env.infthresh) {
	if (_env.nonuniform) 
	  scale = (double)(3.0 * _n) / (2 * (_n1 + _n2 + _n3));
	else
	  scale = (double)_n / 2;
      } else
	scale = (double)_n / 2;
    }

    double **gd = _gamma.data();
    double **gdt = _gammat.data();

    for (uint32_t i = 0; i < _n; ++i) {
      uint32_t c = 1;
      _noderhot[i] = pow(_nodetau0 + ((double)_nodec[i] / 100), -1 * _nodekappa);
      if (!_env.randomnode) { // random pair
	for (uint32_t k = 0; k < _k; ++k) {
	  gdt[i][k] = _alpha[k] + (scale / mbsize) / c * gdt[i][k];
	  gd[i][k] = (1 - _noderhot[i]) * gd[i][k] + _noderhot[i] * gdt[i][k];
	}
      } else if (_env.randomnode) {
	for (uint32_t k = 0; k < _k; ++k) {
	  gdt[i][k] = _alpha[k] + scale * gdt[i][k];
	  gd[i][k] = (1 - _noderhot[i]) * gd[i][k] + _noderhot[i] * gdt[i][k];
	}
      }
      _nodec[i]++;
    }

    //printf("start node = %d, gamma = %s\n", _start_node, _gamma.s().c_str());

    // Array x(_n);
    // for (uint32_t i = 0; i < _n; ++i) {
    //   double v = _gamma.sum(i);
    //   x[i] = v;
    //   printf("%d,", (int)(v));
    // }
    // printf("\n");
    // double mean = x.mean();
    // printf("mean=%.5f, stdev=%.5f\n", mean, x.stdev(mean));

    if (!_env.nolambda)
      // if (!_env.delaylearn || (_iter * _s  > 2 * _network.ones())) {
      if (!_env.delaylearn || (_iter * _s  > _total_pairs)) {
	if (!_delaylearn_reported) {
	  _lambda_start_iter = _iter;
	  Env::plog("learning lambda since (time)", (int)duration());
	  Env::plog("learning lambda since (iter)", _iter);
	  Env::plog("lambda start iter", _lambda_start_iter);
	  _delaylearn_reported = true;
	}

	_rhot = pow(_tau0 + (_iter - _lambda_start_iter + 1), -1 * _kappa);
	//_rhot = pow(_tau0 + _iter, -1 * _kappa);
	double **ldt = _lambdat.data();
	double **ed = _eta.data();
	double **ld = _lambda.data();

	if (!_env.randomnode) { // random pair
	  for (uint32_t k = 0; k < _k; ++k)
	    for (uint32_t t = 0; t < _t; ++t) {
	      ldt[k][t] = ed[k][t] + (scale / mbsize) * ldt[k][t];
	      ld[k][t] = (1 - _rhot) * ld[k][t] + _rhot * ldt[k][t];
	    }
	} else { // random node
	  for (uint32_t k = 0; k < _k; ++k)
	    for (uint32_t t = 0; t < _t; ++t) {
	      ldt[k][t] = ed[k][t] + scale  * ldt[k][t];
	      ld[k][t] = (1 - _rhot) * ld[k][t] + _rhot * ldt[k][t];
	    }
	}
      }
    _iter++;
    fflush(stdout);
    //printf("ones:%d,zeros-inf:%d,zeros-noninf:%d\n", _n3, _n2, _n1);
    _n1 = 0;
    _n2 = 0;
    _n3 = 0;

#ifdef PERF
    gettimeofday(&tv2, NULL);
    timeval_subtract(&result, &tv2, &tv1);
    printf("process post-process took %ld:%ld secs\n",
	   result.tv_sec, result.tv_usec);
#endif

#ifdef PERF
    struct timeval tv3,tv4,r;
    gettimeofday(&tv3, NULL);
#endif

    // _rest.lock();
    // _rest.broadcast();
    // _rest.unlock();

#ifdef PERF
    gettimeofday(&tv4, NULL);
    timeval_subtract(&r, &tv4, &tv3);
    printf("process post-process took %ld:%ld secs\n",
	   r.tv_sec, r.tv_usec);
#endif

    struct timeval past = _last_iter;
    struct timeval tsec;
    gettimeofday(&_last_iter, NULL);
    timeval_subtract(&tsec, &_last_iter, &past);
    //fprintf(_tf, "%d\t%llu.%llu\n",
    //_family, tsec.tv_sec, tsec.tv_usec);
    //fflush(_tf);

    //_last_iter = time(0);

    if (_env.stratified)
      _family = _family ? 0 : 1;


#ifdef MRSTATS
    gettimeofday(&_rt.reduce_tv_after, NULL);
    struct timeval res;
    timeval_subtract(&res, &_rt.reduce_tv_after, &_rt.reduce_tv_before);
    timeval_add(&_rt.reduce_tv_sum, &res);
    _rt.reduce_n++;
#endif

#ifdef MRSTATS
    log_mrstats();
    if (_iter < 10)
      continue;
    else
      exit(0);
#endif

#ifdef HOSTS_CONV
    if (_iter == 1) {
      _lambdanext.copy_from(_lambda);
      _gammanext.copy_from(_gamma);
    }
#endif

    if (_iter % _env.reportfreq == 0) {
      printf("\riteration = %d took %d secs", _iter, duration());
      
      _skipped = 0;

      set_dir_exp(_gamma, _Elogpi);
      set_dir_exp(_lambda, _Elogbeta);

      double a, b, c;
      heldout_likelihood(a, b, c);
      validation_likelihood(a, b, c);

#ifdef TRAINING_SAMPLE
      training_likelihood(a, b, c);
#endif

      //compute_modularity();
      if (_env.logl)
	approx_log_likelihood();

#ifdef HOSTS_CONV
      hosts_conv();
#endif


      if (_env.terminate) {
	do_on_stop();
	_env.terminate = false;
      }
    }
  }
}

void
MMSBInfer::do_on_stop()
{
  info("saving model...\n");
  save_model();
  info("done saving model!\n");

  info("computing groups...\n");
  compute_and_log_groups();
  info("done computing groups!\n");
}

void
MMSBInfer::hosts_conv()
{
  if (_iter > 1) {
    FILE *chf = fopen(Env::file_str("/convergence_hosts.txt").c_str(), "w");

    for (uint32_t i = 0; i < _n; ++i) {
      if ((!_env.randomnode && (_nodeupdatec[i] > _env.conv_nupdates)) ||
	  (_env.randomnode && _nodeupdate_rn[i] > 0)) {
	double q = compute_mean_change_gamma(i);
	if (q < _env.conv_thresh1) {
	  if (_conv_time[i] == 0) {
	    //printf("node %d converging!\n", i);
	    _cn++;
	    _conv_time[i] = duration();
	  }
	} else if (_conv_time[i] > 0 && q > _env.conv_thresh2) {
	  //printf("node %d reconverging!\n", i);
	  _cn--;
	  _conv_time[i] = 0;
	}
	_nodeupdatec[i] = 0;
	_nodeupdate_rn[i] = 0;
	double **gnd = _gammanext.data();
	double **gd = _gamma.data();
	for (uint32_t k = 0; k < _k; k++)
	  gnd[i][k] = gd[i][k];
      }
      if (_conv_time[i] > 0) {
	const IDMap &m = _network.seq2id();
	IDMap::const_iterator idt = m.find(i);

	if (idt != m.end())
	  fprintf(chf, "%d\t%d\t%d\t%d\n",
		  i, idt->second, _network.deg(i), _conv_time[i]);
      }
    }
    fprintf(_cf, "%d\t%d\t%d\n",
	    _iter, duration(), _cn);
    fflush(_cf);
    fclose(chf);
  }
}

double
MMSBInfer::compute_mean_change_gamma(uint32_t i)
{
  Array x(_k);
  _gamma.slice(0, i, x);
  x.scale((double)1.0 / x.sum());

  Array y(_k);
  _gammanext.slice(0, i, y);
  y.scale((double)1.0 / y.sum());

  Array z(_k);
  sub(x, y, z);
  z.abs();
  return z.mean();
}

double
MMSBInfer::compute_mean_change_lambda(uint32_t i)
{
  double l = .0;
  for (uint32_t i = 0; i < _k; ++i) {
    Array x(_t);
    _lambda.slice(0, i, x);
    x.scale((double)1.0 / x.sum());
    Array y(_t);
    _lambdanext.slice(0, i, y);
    y.scale((double)1.0 / y.sum());
    Array z(_t);
    sub(x, y, z);
    l += z.abs().mean();
  }
  return l / _k;
}

void
MMSBInfer::batch_infer()
{
  EdgeList sample;

  while (1) {

    if (_env.max_iterations && _iter > _env.max_iterations) {
      printf("+ Quitting: reached max iterations.\n");
      Env::plog("maxiterations reached", true);
      exit(0);
    }

    set_dir_exp(_gamma, _Elogpi);
    set_dir_exp(_lambda, _Elogbeta);

    double **ld = _lambdanext.data();

    for (uint32_t p = 0; p < _n; ++p) {
      for (uint32_t q = 0; q < _n; ++q) {
	if (p >= q)
	  continue;

	Edge e(p,q);
	Network::order_edge(_env, e);
	const SampleMap::const_iterator u = _heldout_map.find(e);
	if (u != _heldout_map.end())
	  continue;

	const SampleMap::const_iterator v = _validation_map.find(e);
	if (v != _validation_map.end())
	  continue;

	yval_t y = get_y(p,q);

	_pcomp.reset(p,q,y);
	_pcomp.update_phis_until_conv();

	const Array &phi1 = _pcomp.phi1();
	const Array &phi2 = _pcomp.phi2();

	_gammanext.add_slice(p, phi1);
	_gammanext.add_slice(q, phi2);

	for (uint32_t k = 0; k < _k; ++k)
	  for (uint32_t t = 0; t < _t; ++t)
	    ld[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));

	mark_seen(p,q);
      }
    }
    _gamma.reset(_gammanext);
    _gammanext.set_elements(_env.alpha);

    if (!_env.nolambda) {
      _lambda.reset(_lambdanext);
      _lambdanext.copy_from(_eta);
    }

    _iter++;

    _rest.lock();
    _rest.broadcast();
    _rest.unlock();

    //printf("sample size = %d\n", (int)sample.size());
    //printf("iteration = %d took %d secs (family:%d)\n", _iter, duration(), _family);

    if (_iter % _env.reportfreq == 0) {
      set_dir_exp(_gamma, _Elogpi);
      set_dir_exp(_lambda, _Elogbeta);

      double a, b, c;
      heldout_likelihood(a, b, c);
      validation_likelihood(a, b, c);

#ifdef TRAINING_SAMPLE
      training_likelihood(a, b, c);
#endif

      //compute_modularity();
      if (_env.logl)
	approx_log_likelihood();

      printf("\riteration = %d took %d secs", _iter, duration());
      fflush(stdout);

      if (_env.terminate) {
	do_on_stop();
	_env.terminate = false;
      }
    }
    debug("gammat = %s", _gammat.s().c_str());
    debug("lambdat = %s", _lambdat.s().c_str());
    debug("gamma = %s", _gamma.s().c_str());
    debug("lambda = %s", _lambda.s().c_str());
  }
}

void
MMSBInfer::save_model()
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

void
MMSBInfer::compute_and_log_groups()
{
  FILE *groupsf = fopen(Env::file_str("/groups.txt").c_str(), "w");
  FILE *summaryf = fopen(Env::file_str("/summary.txt").c_str(), "a");
  FILE *commf = fopen(Env::file_str("/communities.txt").c_str(), "w");
  MapVec communities;

  estimate_all_pi();
  uint32_t unlikely = 0;

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
    for (uint32_t m = 0; m < _n; ++m) {
      if (i < m) { 
	yval_t y = get_y(i,m);

	if (y == 1) {
	  Array beta(_k);
	  estimate_beta(beta);
	  _Epi.slice(0, m, pi_m);
	  uint32_t max_k = 65535;
	  double max = inner_prod_max(pi_i, pi_m, beta, max_k);
	  if (max < 0.5) {
	    unlikely++;
	    continue;
	  }
	  assert (max_k < _k);
	  communities[max_k].push_back(i);
	  communities[max_k].push_back(m);
	}
      }
    }
    sa << groups[i] << "\n";
  }
  printf("unlikely = %d\n", unlikely);
  fflush(stdout);
  fprintf(groupsf, "%s", sa.str().c_str());

  D1Array<int> s(_k);
  std::map<uint32_t, vector<uint32_t> > comm;
  for (uint32_t i = 0; i < _n; ++i) {
    s[groups[i]]++;
    comm[groups[i]].push_back(i);
  }
  for (uint32_t i = 0; i < _k; ++i)
    fprintf(summaryf, "%d\t", s[i]);
  fprintf(summaryf, ":%d\n", unlikely);

  for (std::map<uint32_t, vector<uint32_t> >::const_iterator i = communities.begin();
       i != communities.end(); ++i) {
    //fprintf(commf, "%d\t", i->first);
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
	uniq[u[p]] = true;
      }
    }
    fprintf(commf, "\n");
  }

  fprintf(summaryf,"\n");
  fflush(groupsf);
  fflush(summaryf);
  fflush(commf);
  fclose(groupsf);
  fclose(summaryf);
  fclose(commf);
  
  if (_env.nmi) {
    if (!_env.benchmark) {
      char cmd[1024];
      sprintf(cmd, "/usr/local/bin/mutual %s %s >> %s", 
	      _env.ground_truth_fname.c_str(),
	      Env::file_str("/communities.txt").c_str(), 
	      Env::file_str("/mutual.txt").c_str());
      if (system(cmd) < 0)
	lerr("error spawning cmd %s:%s", cmd, strerror(errno));
    } else {
      char cmd[1024];
      sprintf(cmd, "/usr/local/bin/mutual %s %s >> %s", 
	      Env::file_str("/ground_truth.txt").c_str(),
	      Env::file_str("/communities.txt").c_str(), 
	      Env::file_str("/mutual.txt").c_str());
      if (system(cmd) < 0)
	lerr("error spawning cmd %s:%s", cmd, strerror(errno));
    }
  }
}

void
MMSBInfer::process(const EdgeList &sample)
{
  debug("process sample: Elogpi = %s", _Elogpi.s().c_str());
  debug("process sample: Elogbeta = %s", _Elogbeta.s().c_str());

  uint32_t p, q;
  //uint32_t c = 0;
#ifdef ALT_PAIRS
  _nodemap.clear();
#endif

  uint32_t cc = 0;
  for (EdgeList::const_iterator i = sample.begin(); i != sample.end(); ++i) {
    p = i->first;
    q = i->second;
    assert (p != q);

    _nodeupdatec[p]++;
    _nodeupdatec[q]++;

#ifdef ALT_PAIRS
    std::map<uint32_t, uint32_t>::const_iterator pi = _nodemap.find(p);
    std::map<uint32_t, uint32_t>::const_iterator qi = _nodemap.find(q);
    if (pi == _nodemap.end())
      _nodemap[p] = 1;
    else
      _nodemap[p]++;
    if (qi == _nodemap.end())
      _nodemap[q] = 1;
    else
      _nodemap[q]++;
#endif

    yval_t y = get_y(p,q);

    if (_env.randomnode) {
      if (y == 1 ||
	  (y == 0 && cc % _env.subsample_scale == 0)) {
	_pcomp.reset(p,q,y);
	_pcomp.update_phis_until_conv();
      }
      const Array &phi1 = _pcomp.phi1();
      const Array &phi2 = _pcomp.phi2();

      if (y == 1) {
	_gammat.add_slice(p, phi1);
	_gammat.add_slice(q, phi2);
      }

      if (y == 0) { // DEBUG
	if (cc % _env.subsample_scale == 0) {
	  _szgammat.add_slice(p, phi1);
	  _szgammat.add_slice(q, phi2);
	}
      }

      if (y == 1 ||
	  (y == 0 && (cc % _env.subsample_scale == 0))) {
	double **ldt = _lambdat.data();
	for (uint32_t k = 0; k < _k; ++k)
	  for (uint32_t t = 0; t < _t; ++t)
	    ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));

      }
      if (y == 0)
	cc++;
    } else { // NOT _env.randomnode
      _pcomp.reset(p,q,y);
      _pcomp.update_phis_until_conv();

      const Array &phi1 = _pcomp.phi1();
      const Array &phi2 = _pcomp.phi2();

      _gammat.add_slice(p, phi1);
      _gammat.add_slice(q, phi2);

      double **ldt = _lambdat.data();
      for (uint32_t k = 0; k < _k; ++k)
	for (uint32_t t = 0; t < _t; ++t)
	  ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
    }

    //_count[p]++;
    //_count[q]++;
#ifndef SPARSE_NETWORK
    yval_t **yd = _network.y().data();
    yd[p][q] = y | 0x80; // seen
#else
    // TODO: fix sparse network heldout
#endif
  }

  if (_env.randomnode) {
    _gammat.add_to(_szgammat.scale(_env.subsample_scale));
    double **ldt = _lambdat.data();
    for (uint32_t k = 0; k < _k; ++k)
      ldt[k][1] *= _env.subsample_scale;
  }
}

void
MMSBInfer::opt_process(const EdgeList &sample)
{
  uint32_t p, q;
  double **ldt = _lambdat.data();

  estimate_all_pi();
  
  Array sphi(_k);
  _Elogpi.slice(0, _start_node, sphi);
  sphi.exp();
  sphi.scale((double)1.0/sphi.sum());

  uint32_t n = 0;
  for (EdgeList::const_iterator i = sample.begin(); i != sample.end(); ++i) {
    p = i->first;
    q = i->second;
    assert (p != q);

    yval_t y = get_y(p,q);

    if (y == 1) {
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

    if (y == 0) {
      Array a(_k);
      _Epi.slice(0, p, a);
      Array b(_k);
      _Epi.slice(0, q, b);
      
      if (dot(a,b) < _env.infthresh) {
	Edge e(p, q);
	Network::order_edge(_env, e);
	
	uint32_t other_node = (p != _start_node) ? p : q;
	Array ophi(_k);
	_Elogpi.slice(0, other_node, ophi);
	ophi.exp();
	ophi.scale((double)1.0/ophi.sum());
	_szgammat.add_slice(other_node, ophi);
	n++;
      } else {
	_pcomp.reset(p,q,y);
	_pcomp.update_phis_until_conv();
	
	const Array &phi1 = _pcomp.phi1();
	const Array &phi2 = _pcomp.phi2();
	
	_szgammat.add_slice(p, phi1);
	_szgammat.add_slice(q, phi2);

	for (uint32_t k = 0; k < _k; ++k)
	  for (uint32_t t = 0; t < _t; ++t)
	    ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
      }
    }
    mark_seen(p,q);
  }
  
  if (n > 0) {
    Array lphi(_k);
    lphi.set_elements((double)_n/_k);

    for (uint32_t k = 0; k < _k; ++k)
      for (uint32_t t = 0; t < _t; ++t)
	ldt[k][t] += lphi[k] * sphi[k] * (t == 0 ? 0 : 1);

    if (false) {
      printf("(sphi) %s\n", sphi.s().c_str());
      printf("n = %d\n", n);
    }
    _szgammat.add_slice(_start_node, sphi.scale(n));
  }
  if (false) {
    Array a(_k);
    _gammat.slice(0, _start_node, a);
    printf("(ones)gammat=%s\n", a.s().c_str());
    fflush(stdout);
  }

  _gammat.add_to(_szgammat);
  _skipped += n;

  if (false) {
    printf("deg=%d\n", _network.deg(_start_node));
    printf("%s\n", sphi.s().c_str());
    Array a(_k);
    _gammat.slice(0, _start_node, a);
    Array b(_k);
    _gamma.slice(0, _start_node, b);
    printf("gammat=%s\n", a.s().c_str());
    printf("gamma=%s\n", b.s().c_str());
    fflush(stdout);
  }
}

void
MMSBInfer::opt_process2(vector<uint32_t> &nodes)
{
  _start_node = gsl_rng_uniform_int(_r, _n);
  double **ldt = _lambdat.data();
  
  // full inference only around the neighborhood _start_node
  set_dir_exp(_start_node, _gamma, _Elogpi);
  _gammat.zero(_start_node);
  nodes.push_back(_start_node);
  
  refresh(_start_node);

  Array sphi(_k);
  _Elogpi.slice(0, _start_node, sphi);
  sphi.exp();
  sphi.scale((double)1.0/sphi.sum());

  const vector<uint32_t> *edges = _network.get_edges(_start_node);
  bool singleton = false;
  if (!edges)  // singleton node
    singleton = true;

  struct timeval s;
  s.tv_sec = 0;
  s.tv_usec = 0;

  uint32_t l_size = 0;
  if (!singleton) {
    for (uint32_t i = 0; i < edges->size(); ++i) {
      uint32_t a = (*edges)[i];
      
      Edge e(_start_node,a);
      Network::order_edge(_env, e);
      const SampleMap::const_iterator u1 = _heldout_map.find(e);
      if (u1 != _heldout_map.end())
	continue;

      const SampleMap::const_iterator u2 = _validation_map.find(e);
      if (u2 != _validation_map.end())
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
      
      refresh(a);

      struct timeval tv3,tv4,r;
      gettimeofday(&tv3, NULL);

      _pcomp.reset(p,q,y);
      _pcomp.update_phis_until_conv();

      gettimeofday(&tv4, NULL);
      timeval_subtract(&r, &tv4, &tv3);
      timeval_add(&s, &r);

      const Array &phi1 = _pcomp.phi1();
      const Array &phi2 = _pcomp.phi2();

      //printf("1->phi1: %s\n", phi1.s().c_str());
      //printf("1->phi2: %s\n", phi2.s().c_str());
      
      _gammat.add_slice(p, phi1);
      _gammat.add_slice(q, phi2);

      for (uint32_t k = 0; k < _k; ++k)
	for (uint32_t t = 0; t < _t; ++t)
	  ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
    }
  }

  //printf("time for %ld edge updates = %ld:%ld\n", edges->size(), 
  //s.tv_sec, s.tv_usec);

  s.tv_sec = 0;
  s.tv_usec = 0;

  vector<uint32_t> neighbors;
  get_similar_nodes(_start_node, neighbors);

  uint32_t nl_inf_size = 0;
  for (uint32_t i = 0; i < neighbors.size(); ++i) {
    uint32_t a = neighbors[i];
    assert (a != _start_node);

    yval_t y = get_y(_start_node, a);
    if (y == 1)  // already processed links
      continue;
    
    Edge e(_start_node,a);
    Network::order_edge(_env, e);
    const SampleMap::const_iterator u1 = _heldout_map.find(e);
    if (u1 != _heldout_map.end())
      continue;
    
    const SampleMap::const_iterator u2 = _validation_map.find(e);
    if (u2 != _validation_map.end())
      continue;      

    set_dir_exp(a, _gamma, _Elogpi);
    nodes.push_back(a);
    _gammat.zero(a);
    
    uint32_t p = e.first;
    uint32_t q = e.second;

    mark_seen(p,q);
    nl_inf_size++;

    refresh(a);

    struct timeval tv3,tv4,r;
    gettimeofday(&tv3, NULL);
    
    _pcomp.reset(p,q,y);
    _pcomp.update_phis_until_conv();

    gettimeofday(&tv4, NULL);
    timeval_subtract(&r, &tv4, &tv3);
    timeval_add(&s, &r);

    const Array &phi1 = _pcomp.phi1();
    const Array &phi2 = _pcomp.phi2();

    Array ga(_k), gb(_k);
    _gamma.slice(0, _start_node, ga);
    _gamma.slice(0, a, gb);

    uint32_t i1, i2;
    ga.max(i1);
    gb.max(i2);
    if (false) {
      printf("startnode=%d\n", _start_node);
      printf("a=%d", a);
      printf("0->phi1: %s\n", phi1.s().c_str());
      printf("0->gamma1: %s\n", ga.s().c_str());
      printf("total gamma1: %.5f\n", ga.sum());
      printf("0->phi2: %s\n", phi2.s().c_str());
      printf("0->gamma2: %s\n", gb.s().c_str());
      printf("total gamma2: %.5f\n", gb.sum());
      printf("\n\n");
    }
    _gammat.add_slice(p, phi1);
    _gammat.add_slice(q, phi2);

    for (uint32_t k = 0; k < _k; ++k)
      for (uint32_t t = 0; t < _t; ++t)
	ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
  }
  
  //if (nl_inf_size)
  //printf("time for %u non-edge updates = %ld:%ld\n", nl_inf_size, 
  //s.tv_sec, s.tv_usec);

  _neighbors[_iter % _env.reportfreq] = nl_inf_size;
  
  uint32_t nl_ninf_size = (_n - 1) - (l_size + nl_inf_size);

  //printf("nl_inf_size=%d, nl_ninf_size=%d\n", nl_inf_size, nl_ninf_size);
  // - approx. inference over all other nodes
  // - we only need the count of other nodes
  // - we do not need to update their state as dirty 
  // - their last_up_iter will show that they are 'dirty'
  if (nl_ninf_size > 0 && false) { // XXX
    Array lphi(_k);
    lphi.set_elements((double)nl_ninf_size/_k);

    for (uint32_t k = 0; k < _k; ++k)
      for (uint32_t t = 0; t < _t; ++t)
	ldt[k][t] += lphi[k] * sphi[k] * (t == 0 ? 0 : 1);
    
    _gammat.add_slice(_start_node, sphi.scale(nl_ninf_size));
  }
}

void
MMSBInfer::refresh(uint32_t a)
{
  return;
  // refresh the node from last update iter to current iter
  NodeValMap::const_iterator i = _iter_map.find(a);
  uint32_t last_up_iter;
  if (i == _iter_map.end()) // nothing to do
    last_up_iter = 0;
  else
    last_up_iter = i->second;

  if (last_up_iter == _iter ||
      last_up_iter == _iter - 1)
    return;

  double **gd = _gamma.data();
  //for (uint32_t j = last_up_iter; j < _iter; ++j) {
  //printf("refreshing from %d to %d for node %d\n", last_up_iter, _iter, a);
  
  _noderhot[a] = pow(_nodetau0 + _nodec[a], -1 * _nodekappa);
  set_dir_exp(a, _gamma, _Elogpi);
  
  Array phi(_k);
  _Elogpi.slice(0, a, phi);
  phi.exp();
  phi.scale((double)1.0/phi.sum());
  
  double scale = (double)_n / 2;
  for (uint32_t k = 0; k < _k; ++k) {
    phi[k] = _alpha[k] + scale * phi[k];
    gd[a][k] = (1 - _noderhot[a]) * gd[a][k] + _noderhot[a] * phi[k];
  }
  _nodec[a]++;
  _iter_map[a]++;
}

void
MMSBInfer::opt_nonuniform_process(const EdgeList &sample)
{
  uint32_t p, q;
  double **ldt = _lambdat.data();

  estimate_all_pi();
  Array sum(_k);
  uint32_t n = 0;

  uint32_t nzeros_inf = 0, nzeros_noninf = 0;
  uint32_t nones = 0;
  
  ValueMap vmap;
  for (EdgeList::const_iterator i = sample.begin(); i != sample.end(); ++i) {
    p = i->first;
    q = i->second;
    assert (p != q);
    
    Array a(_k);
    _Epi.slice(0, p, a);
    Array b(_k);
    _Epi.slice(0, q, b);

    yval_t y = get_y(p,q);
    double p_i = dot(a,b);
    if (p_i > 1.0)
      p_i = 1.0;
    if (p_i < .0)
      p_i = .0;

    if (y == 1) {
      vmap[*i] = 1 - p_i;
      nones++;
    } else if (y == 0) {
      vmap[*i] = p_i;
      if (p_i > _env.infthresh) 
	nzeros_inf++;
      else 
	nzeros_noninf++;
    }
  }
  //printf("sum pi = %.5f\n", sum_pi);;
  //double f = nzeros_inf / (nzeros_noninf + nzeros_inf);
  //f = f < 0.01 ? 0.01 : f;
  uint32_t n1_max = nones;
  uint32_t n3_max = nzeros_inf * 0.001;
  uint32_t n2_max = 1; //nzeros_noninf * 0.01;
  //n2_max = n2_max < nzeros_inf ? nzeros_inf : n2_max;

  printf("cnt:%d\t%d\t%d\n", nones, nzeros_inf, nzeros_noninf);
  printf("max:%d\t%d\t%d\n", n1_max, n3_max, n2_max);
  
  //Array sphi(_k);
  //_Elogpi.slice(0, _start_node, sphi);
  //sphi.exp();
  //sphi.scale((double)1.0/sphi.sum());
  //sphi.scale((double)nzeros_noninf / n2_max);

  double pi1 = .0, pi2 = .0, pi3 = .0;
  for (EdgeList::const_iterator i = sample.begin(); i != sample.end(); ++i) {
    p = i->first;
    q = i->second;
    assert (p != q);
    
    double p_i = vmap[*i];

    _nodeupdatec[p]++;
    _nodeupdatec[q]++;

    yval_t y = get_y(p,q);    
    if (y == 1) {
      if (_n1 >= n1_max)
	continue;

      _n1++;
      
      pi1 += p_i;
      _pcomp.reset(p,q,y);
      _pcomp.update_phis_until_conv();
      
      Array &phi1 = _pcomp.mutable_phi1();
      Array &phi2 = _pcomp.mutable_phi2();

      phi1.scale((double)nones);
      phi2.scale((double)nones);

      _gammat.add_slice(p, phi1);
      _gammat.add_slice(q, phi2);
      
      for (uint32_t k = 0; k < _k; ++k)
	for (uint32_t t = 0; t < _t; ++t)
	  ldt[k][t] += ((double)1.0 / nones) * phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
    }

    if (y == 0) {      
      if (p_i < _env.infthresh) {
	if (_n2 >= n2_max)
	  continue;
	
	_n2++;
	pi2 += p_i;

	_pcomp.reset(p,q,y);
	_pcomp.update_phis_until_conv();
	
	Array &phi1 = _pcomp.mutable_phi1();
	Array &phi2 = _pcomp.mutable_phi2();
	
	phi1.scale((double)nzeros_noninf);
	phi2.scale((double)nzeros_noninf);

	_szgammat.add_slice(p, phi1);
	_szgammat.add_slice(q, phi2);

	Edge e(p, q);
	Network::order_edge(_env, e);

	for (uint32_t k = 0; k < _k; ++k)
	  for (uint32_t t = 0; t < _t; ++t)
	    ldt[k][t] += ((double)1.0 / nzeros_noninf) * \
	      phi1[k] * phi2[k] * (t == 0 ? y : (1-y));

      } else {
	if (_n3 >= n3_max)
	  continue;
	
	_n3++;
	pi3 += p_i;

	_pcomp.reset(p,q,y);
	_pcomp.update_phis_until_conv();
	
	Array &phi1 = _pcomp.mutable_phi1();
	Array &phi2 = _pcomp.mutable_phi2();
	
	phi1.scale((double)nzeros_inf);
	phi2.scale((double)nzeros_inf);

	_szgammat.add_slice(p, phi1);
	_szgammat.add_slice(q, phi2);

	for (uint32_t k = 0; k < _k; ++k)
	  for (uint32_t t = 0; t < _t; ++t)
	    ldt[k][t] += ((double)1.0 / nzeros_inf) * phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
      }
    }
#ifndef SPARSE_NETWORK
    yval_t **yd = _network.y().data();
    yd[p][q] = y | 0x80;
#endif
  }

  if (n > 0) {
    //Array lphi(_k);
    //lphi.set_elements((double)_n/_k);

    //for (uint32_t k = 0; k < _k; ++k)
    //for (uint32_t t = 0; t < _t; ++t)
    //ldt[k][t] += lphi[k] * sphi[k] * (t == 0 ? 0 : 1);
    
    //_szgammat.add_slice(_start_node, sphi.scale(n));
  }
  _gammat.add_to(_szgammat);
  printf("node:%d, degree:%d\n", _start_node, _network.deg(_start_node));
  printf("pi avg -> ones(%d):%.5f, inf0(%d):%.5f, ninf0(%d):%.5f\n", 
	 _n1, 
	 _n1 > .0 ? pi1 / _n1 : .0, 
	 _n3,
	 _n3 > .0 ? pi3 / _n3 : .0,
	 _n2,
	 _n2 > .0 ? pi2 / _n2 : .0); 

  //printf("sum_pi:%.5f\n", sum_pi);
  fflush(stdout);
}


void
MMSBInfer::multithreaded_process(const EdgeList &sample)
{
#ifdef PERF
      struct timeval tv3,tv4,r;
      gettimeofday(&tv3, NULL);
#endif

  info("multithreaded process");
  // split edgelist into _nthreads chunks
  uint32_t k = 0;
  uint32_t n = 0;
  uint32_t chunk_size = (int)(((double)sample.size()) / _nthreads);
  debug("chunk size = %u\n", chunk_size);

  info("samples size = %d", sample.size());
  for (EdgeList::const_iterator i = sample.begin(); i != sample.end(); ++i) {
    uint32_t p = i->first;
    uint32_t q = i->second;
    assert (p != q);

    _nodeupdatec[p]++;
    _nodeupdatec[q]++;

    ChunkMap::iterator it = _chunk_map.find(n);
    if (it == _chunk_map.end()) {
      EdgeList *v = new EdgeList;
      _chunk_map[n] = v;
    }
    EdgeList *v = _chunk_map[n];
    v->push_back(*i);
    k++;
    if (k >= chunk_size && n < (uint32_t)_nthreads - 1) {
      debug("last chunk size = %d\n", (int)v->size());
      k = 0;
      n++;
    }
  }
  debug("divided into %d chunks\n", n+1);

#ifdef PERF
      gettimeofday(&tv4, NULL);
      timeval_subtract(&r, &tv4, &tv3);
      printf("dividing took %ld:%ld secs\n",
	    r.tv_sec, r.tv_usec);
#endif

#ifdef MRSTATS
  gettimeofday(&_rt.init_tv_after, NULL);
  struct timeval res;
  timeval_subtract(&res, &_rt.init_tv_after, &_rt.init_tv_before);
  timeval_add(&_rt.init_tv_sum, &res);
  _rt.init_n++;
#endif


  for (ChunkMap::iterator it = _chunk_map.begin();
       it != _chunk_map.end(); ++it) {
    EdgeList *v = it->second;
    debug("pushing edges of size %d\n", (int)v->size());
    _out_q.push(v);
  }
  info("pushed all edges");


  int nt = 0;

  double **gd = _gammat.data();
  double **ld = _lambdat.data();
  do {
    pthread_t *p = _in_q.pop();
    assert(p);

    PhiRunner *t = _thread_map[*p];
    //printf("got results from thread %ld\n", t->id());
    assert(t);

    const EdgeList &sample = t->sample();
    const double ** const gtd = t->gammat().const_data();
    const double ** const ltd = t->lambdat().const_data();

#ifdef MRSTATS
    const MRStats *rt_thread = t->rt();
#endif

    std::map<uint32_t, uint32_t> nm;
    for (EdgeList::const_iterator i = sample.begin(); i != sample.end(); ++i) {
      uint32_t a = i->first;
      uint32_t b = i->second;
      if (nm.find(a) == nm.end())
	nm[a] = 1;
      else
	nm[a]++;
      if (nm.find(b) == nm.end())
	nm[b] = 1;
      else
	nm[b]++;
    }

    for (std::map<uint32_t, uint32_t>::const_iterator i = nm.begin();
	 i != nm.end(); ++i) {
      uint32_t node = i->first;
      //uint32_t val = i->second;
      for (uint32_t k = 0; k < _k; ++k)
	gd[node][k] += gtd[node][k];
    }

    for (uint32_t k = 0; k < _k; ++k)
      for (uint32_t t = 0; t < _t; ++t)
    	ld[k][t] += ltd[k][t];

    // do we need _count ?
    // _count.add_to(t->count());

#ifdef MRSTATS
    timeval_add(&_rt.map_tv_sum, &(rt_thread->map_tv_sum));
    _rt.map_n++;
    // we assume that reduce begins when the last map ends
    if (nt == _nthreads - 1)
      gettimeofday(&_rt.reduce_tv_before, NULL);
#endif

    delete p;
    nt++;
  } while (nt != _nthreads || !_in_q.empty());
  _chunk_map.clear();
}

#ifdef SPARSE_NETWORK
void
MMSBInfer::get_bfs_edges(uint32_t start_node, uint32_t limit, EdgeList &edges)
{
  SparseMatrix &sm = _network.sparse_y();
  _q.push(start_node);
  _visited[start_node] = true;
  uint32_t curr_node = start_node;
  uint32_t c = 0;
  while (!_q.empty() && c < limit) {
    const vector<uint32_t> *v = sm[curr_node];
    if (v)
      for (uint32_t i = 0; i < v->size(); ++i) {
	bool visited = _visited[v->at(i)];
	if (!visited) {
	  _q.push(v->at(i));
	  Edge e(curr_node, v->at(i));
	  if (edge_ok(e, false)) {
	    Network::order_edge(_env, e);
	    edges.push_back(e);
	    c++;
	    if (c >= limit)
	      break;
	  }
	  _visited[v->at(i)] = true;
	}
      }
    _q.pop();
  }
  for (uint32_t i = 0; i < _q.size(); ++i)
    _q.pop();
  _visited.clear();
}
#endif

void
MMSBInfer::get_randomnode_edges(uint32_t start_node, EdgeList &edges) const
{
  for (uint32_t i = 0; i < _n; ++i)
    if (i != start_node) {
      Edge e(start_node, i);
      Network::order_edge(_env, e);
      if (edge_ok(e, false))
	edges.push_back(e);
    }
}

void
MMSBInfer::get_randomnode_edges2(uint32_t start_node, EdgeList &edges) const
{
  vector<uint32_t> v;
  for (uint32_t i = 0; i < _n; ++i)
    if (i != start_node) {
      if (_network.y(start_node, i) == 1) {
	v.push_back(i);
	Edge e(start_node, i);
	Network::order_edge(_env, e);
	if (edge_ok(e, false))
	  edges.push_back(e);
      }
    }

  uint32_t rem = _n - edges.size();
  assert (rem >= 0);
  
  vector<uint32_t> u;
  for (uint32_t i = 0; i < v.size(); ++i) {
    uint32_t a = v[i];
    for (uint32_t j = 0; j < _n; ++j) {
      if (_network.y(start_node, j) == 0 &&
	  _network.y(a, j) == 1) {
	Edge e(start_node, j);
	Network::order_edge(_env, e);
	if (edge_ok(e, false))
	  edges.push_back(e);
      }
    }
  }
}

void
MMSBInfer::get_subsample(EdgeList &edges, uint32_t n)
{
  do {
    Edge e;
    if (_env.deterministic) {
      get_edge(e);
      edges.push_back(e);
    } else if (!_env.randomnode) {
      if (_env.stratified && _env.bfs && _family == 1) {
#ifdef SPARSE_NETWORK
	_start_node = gsl_rng_uniform_int(_r, _n);
	get_bfs_edges(_start_node, ((double)n) / _env.bfs_nodes, edges);
#endif
	assert(0);
      } else {
	get_random_edge(e);
	assert (e.first != e.second);
	edges.push_back(e);
      }
    } else if (_env.randomnode) {
      _start_node = gsl_rng_uniform_int(_r, _n);
      _nodeupdate_rn[_start_node] = 1;
      get_randomnode_edges(_start_node, edges);
      assert (edges.size() <= _n - 1);
      break;
    } else {
      fprintf(stderr, "unknown sampling option\n");
      assert(0);
    }
  } while (edges.size() < n);
  //printf("start node = %d\n", _start_node);
  debug("mini-batch edges = %s, size = %ld\n",
	 edgelist_s(edges).c_str(), edges.size());
}

double
MMSBInfer::approx_log_likelihood()
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
      const SampleMap::const_iterator u = _heldout_map.find(e);
      if (u != _heldout_map.end())
	continue;

      const SampleMap::const_iterator u2 = _validation_map.find(e);
      if (u2 != _validation_map.end())
	continue;      

      yval_t y = get_y(p,q);

      _pcomp.reset(p,q,y);
      _pcomp.update_phis_until_conv(); 
      const Array &phi1 = _pcomp.phi1();
      const Array &phi2 = _pcomp.phi2();
      
      Array Elogf(_k);
      PhiComp::compute_Elogf(p,q,y,_k,_t,_Elogbeta,Elogf);
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

  printf("approx. log likelihood = %f\n", s);
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
	  printf("error running cmd %s\n", strerror(errno));
      }
      //exit(0);
    }
  }
  _prev_w = w;
  return s;
}

void
MMSBInfer::heldout_likelihood(double &a, double &a0, double &a1)
{
  if (_env.accuracy)
    return;
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
    debug("edge likelihood for (%d,%d) is %f\n", p,q,u);
  }

  double nshol = (_zeros_prob * (szeros / kzeros)) + (_ones_prob * (sones / kones));
  fprintf(_hf, "%d\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%d\t%.9f\t%.9f\t%.9f\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones,
	  _zeros_prob * (szeros / kzeros),
	  _ones_prob * (sones / kones),
	  nshol);
  fflush(_hf);

  a = nshol;

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

    if (a > _max_h) {
      double av0, av1, av2;
      validation_likelihood(av0, av1, av2);
      
      double at0 = 0;
#ifdef TRAINING_SAMPLE
      double at1 = 0, at2 = 0;
      training_likelihood(at0, at1, at2);
#endif

      _max_h = a;
      _max_v = av0;
      _max_t = at0;
    }
    
    if (_nh > 2) { // be robust to small fluctuations in predictive likelihood
      why = 1;
      stop = true;
    }
  }
  _prev_h = nshol;
  FILE *f = fopen(Env::file_str("/max.txt").c_str(), "w");
  fprintf(f, "%d\t%d\t%.5f\t%.5f\t%.5f\t%.5f\t%d\n", 
	  _iter, duration(), 
	  a, _max_t, _max_h, _max_v, why);
  fclose(f);
  if (_env.use_validation_stop && stop) {
    do_on_stop();
    exit(0);
  }
}

void
MMSBInfer::validation_likelihood(double &av, double &av0, double &av1) const
{
  if (_env.accuracy)
    return;

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
    bool seen = false;
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
  fprintf(_vf, "%d\t%d\t%.5f\t%d\t%.5f\t%d\t%.5f\t%d\n",
	  _iter, duration(), s / k, k,
	  szeros / kzeros, kzeros, sones / kones, kones);
  fflush(_vf);

  av = s / k;
  av0 = szeros / kzeros;
  av1 = sones / kones;
}


#ifdef TRAINING_SAMPLE
void
MMSBInfer::training_likelihood(double &av, double &av0, double &av1)
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
    bool seen = false; // XXX
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

  av = s / k;
  av0 = szeros / kzeros;
  av1 = sones / kones;

  double thresh = _env.stopthresh;
  double w = av;
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
	if  (system(cmd) < 0)
	  lerr("error spawning %s:%s", cmd, strerror(errno));
    }
    //exit(0);
    _training_done = true;
  }
  _prev_t = w;
}
#endif


void
MMSBInfer::moving_heldout_likelihood(EdgeList &sample) const
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

void
MMSBInfer::compute_modularity()
{
  estimate_all_pi();
  const Array &deg = _network.deg();
  debug("deg = %s", deg.s().c_str());

  double q = .0;
  double qmax = .0;
  uint32_t ones = _network.ones();
  double assor = .0;
  for (uint32_t i = 0; i < _n; ++i)
    for (uint32_t j = 0; j < _n; ++j) {
      yval_t y = get_y(i,j);
      double v = ((double)deg[i] * deg[j]) / (2 * ones);
      double w = _Epi.dot(i, j);
      q += (y - v) * w;
      qmax += v * w;
    }

  qmax = 2 * ones - qmax;
  assor = q / qmax;
  q = q / (2*_network.ones());
  fprintf(_mf, "%d\t%d\t%.5f\t%.5f\n", _iter, duration(), q, assor);
  fflush(_mf);
}


// PhiRunner

int
PhiRunner::do_work()
{
  do  {
    EdgeList *e = _out_q.pop();

#ifdef MRSTATS
      gettimeofday(&_rt.map_tv_before, NULL);
#endif

    // fprintf(stdout, "thread %ld: popped a chunk of size %d\n",
    // pthread_self(), (int)e->size());

    assert(e);
    const EdgeList &sample = *e;

#ifdef PERF
    struct timeval tv1,tv2,result;
    gettimeofday(&tv1, NULL);
#endif

    process(sample);

#ifdef PERF
    gettimeofday(&tv2, NULL);
    timeval_subtract(&result, &tv2, &tv1);
    printf("thread %ld:process sample took %ld:%ld secs\n", pthread_self(),
	   result.tv_sec, result.tv_usec);
#endif

    _sample = e;

    uint32_t prev_iter  = _pcomp.iter();
    pthread_t *p = new pthread_t(pthread_self());
    _in_q.push(p);

    //_cm.lock();
    set_done();
    //_cm.signal();
    //_cm.unlock();

#ifdef MRSTATS
  gettimeofday(&_rt.map_tv_after, NULL);
  timeval_subtract(&_rt.map_tv_sum, &_rt.map_tv_after, &_rt.map_tv_before);
#endif
    _rest.lock();
    while (_pcomp.iter() == prev_iter) {
      info("checking iter %d %d", _pcomp.iter(), prev_iter);
      _rest.wait();
    }
    _rest.unlock();
    reset();
  } while (1);

  return 0;
}

void
PhiRunner::process(const EdgeList &sample)
{
  uint32_t p = 0, q = 0;
  yval_t y = 0;
  if (_env.stratified)
    y = _family;

  uint32_t cc = 0;
  for (EdgeList::const_iterator i = sample.begin(); i != sample.end(); ++i) {
    p = i->first;
    q = i->second;

    assert (p != q);
    if (!_env.stratified)
      y = _network.y(p,q);

    if (!_env.randomnode) {
      _pcomp.reset(p,q,y);
      _pcomp.update_phis_until_conv();

      const Array &phi1 = _pcomp.phi1();
      const Array &phi2 = _pcomp.phi2();

      debug("conv phi1  = %s", phi1.s().c_str());
      debug("conv phi2  = %s", phi2.s().c_str());

      _gammat.add_slice(p, phi1);
      _gammat.add_slice(q, phi2);

      double **ldt = _lambdat.data();
      for (uint32_t k = 0; k < _k; ++k)
	for (uint32_t t = 0; t < _t; ++t)
	  ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));
    } else { // env.randomnode

      if (y == 1 ||
	  (y == 0 && cc % _env.subsample_scale == 0)) {
	_pcomp.reset(p,q,y);
	_pcomp.update_phis_until_conv();
      }
      const Array &phi1 = _pcomp.phi1();
      const Array &phi2 = _pcomp.phi2();

      if (y == 1) {
	_gammat.add_slice(p, phi1);
	_gammat.add_slice(q, phi2);
      }

      if (y == 0) { // DEBUG
	if (cc % _env.subsample_scale == 0) {
	  _szgammat.add_slice(p, phi1);
	  _szgammat.add_slice(q, phi2);
	}
      }

      if (y == 1 ||
	  (y == 0 && (cc % _env.subsample_scale == 0))) {
	double **ldt = _lambdat.data();
	for (uint32_t k = 0; k < _k; ++k)
	  for (uint32_t t = 0; t < _t; ++t)
	    ldt[k][t] += phi1[k] * phi2[k] * (t == 0 ? y : (1-y));

      }
      if (y == 0)
	cc++;
    }

    //_count[p]++;
    //_count[q]++;

    if (!_env.stratified) {
#ifndef SPARSE_NETWORK
      yval_t **yd = _network.y().data();
      yd[p][q] = y | 0x80; // seen, thread safe operation
#else
      //TODO: fix sparse network heldout
#endif
    }
    debug("%ld: processed %d,%d\n", pthread_self(), p, q);
  }

  // printf("%ld: after processing gammat = %s\n", pthread_self(),
  // _gammat.s().c_str());

  if (_env.randomnode) {
    _gammat.add_to(_szgammat.scale(_env.subsample_scale));
    double **ldt = _lambdat.data();
    for (uint32_t k = 0; k < _k; ++k)
      ldt[k][1] *= _env.subsample_scale;
  }
}

int
MMSBInfer::load_model()
{
  fprintf(stderr, "+ loading gamma\n");
  double **gd = _gamma.data();
  FILE *gammaf = fopen("gamma.txt", "r");
  if (!gammaf)
    return -1;
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
  if (!lambdaf)
    return -1;
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

#ifdef MRSTATS
void
MMSBInfer::log_mrstats()
{
  _mrstatsf = fopen(Env::file_str("/mrstats.txt").c_str(), "w");
  if (!_mrstatsf) {
    printf("cannot open mrstats file:%s\n", strerror(errno));
    exit(-1);
  }

  struct timeval res;
  timeval_subtract(&res, &_rt.reduce_tv_after, &_rt.init_tv_before);

  char init_t[32];
  sprintf(init_t, "%ld.%06ld", (long)_rt.init_tv_sum.tv_sec,
	  (long)_rt.init_tv_sum.tv_usec);
  char map_t[32];
  sprintf(map_t, "%ld.%06ld", (long)_rt.map_tv_sum.tv_sec,
	  (long)_rt.map_tv_sum.tv_usec);
  char reduce_t[32];
  sprintf(reduce_t, "%ld.%02ld", (long)_rt.reduce_tv_sum.tv_sec,
	  (long)_rt.reduce_tv_sum.tv_usec);

  char all_t[32];
  sprintf(all_t, "%ld.%02ld", (long)res.tv_sec, (long)res.tv_usec);

  fprintf(_mrstatsf, "%d\t%d\t%d\t%d\t%d\t%d\t%s\t"
	  "%d\t%s\t"
	  "%d\t%s\t"
	  "%d\t%s\n",
	  _n, _s, _nthreads, _k,
	  _iter, duration(),
	  all_t,
	  _rt.init_n, init_t,
	  _rt.map_n, map_t,
	  _rt.reduce_n, reduce_t);
  fflush(_mrstatsf);
  fclose(_mrstatsf);
}
#endif

