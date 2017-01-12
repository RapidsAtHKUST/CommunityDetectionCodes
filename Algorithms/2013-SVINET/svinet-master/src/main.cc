#include "env.hh"
#include "mmsbinfer.hh"
#include "linksampling.hh"
#include "fastamm.hh"
#include "fastinit.hh"
#include "fastamm2.hh"
#include "sbm.hh"
#include "mmsbinferorig.hh"
#include "mmsbgen.hh"
#include "log.hh"
#include "mmsborig.hh"
#include "fastqueue.hh"
#include <stdlib.h>

#include <string>
#include <iostream>
#include <sstream>
#include <signal.h>

string Env::prefix = "";
Logger::Level Env::level = Logger::DEBUG;
FILE *Env::_plogf = NULL;
void usage();
void test();

Env *env_global = NULL;
volatile sig_atomic_t sig_handler_active = 0;

void
term_handler(int sig)
{
  if (env_global) {
    printf("\nGot signal. Saving model and groups.\n");
    fflush(stdout);
    env_global->terminate = 1;
  } else {
    signal(sig, SIG_DFL);
    raise(sig);
  }
}

int 
main(int argc, char **argv)
{
  
  signal(SIGTERM, term_handler);

  bool run_gap = false;
  bool force_overwrite_dir = true;
  string datfname = "network.dat";
  bool gen = false, ppc = false, lcstats = false;
  bool gml = false;
  bool findk = false;
  string label = "mmsb";
  uint32_t nthreads = 0;
  int i = 1;
  uint32_t n = 0, k = 0;

  bool stratified = false, rnode = false, rpair = false;
  bool batch = false;
  bool online = true;
  bool link_sampling = false;
  bool nodelay = true;
  bool load = false; 
  bool val_load = false;
  string val_file_location = "";
  bool test_load = false;
  string test_file_location = "";
  bool adamic_adar = false; 
  string location = "";
  uint32_t scale = 1;
  bool disjoint = false;
  bool orig = false;
  bool massive = false;
  bool single = false;
  bool sparsek = false;
  bool logl = false;

  int itype = 0; // init type
  string eta_type = "uniform"; // "uniform", "fromdata", "sparse", "regular" or "dense"
  uint32_t rfreq = 1;
  bool accuracy = false;
  double stopthresh = 0.00001;

  double infthresh = 0;
  bool nonuniform = false;

  string ground_truth_fname = ""; // ground-truth communities file, if any
  bool nmi = false;
  bool bmark = false;
  bool randzeros = false;
  bool preprocess = false;
  bool strid = false;
  string groups_file = "";

  uint32_t max_iterations = 0;
  uint32_t use_validation_stop = true;
  double rand_seed = 0;

  double hol_ratio = 0.01;
  bool load_test_sets_opt = false;
  double link_thresh = 0.5;
  uint32_t lt_min_deg = 0;
  bool init_comm = false;
  string init_comm_fname = "";

  bool load_heldout_sets = false;

  if (argc == 1) {
    usage();
    exit(-1);
  }
  
  while (i <= argc - 1) {
    if (strcmp(argv[i], "-help") == 0) {
      usage();
      exit(0);
    } else if (strcmp(argv[i], "-gp") == 0) {
      run_gap = true;
    } else if (strcmp(argv[i], "-force") == 0) {
      force_overwrite_dir = true;
    } else if (strcmp(argv[i], "-online") == 0) {
      online = true;
      batch = false;
    } else if (strcmp(argv[i], "-file") == 0) {
      if (i + 1 > argc - 1) {
	fprintf(stderr, "+ insufficient arguments!\n");
	exit(-1);
      }
      datfname = string(argv[++i]);
    } else if (strcmp(argv[i], "-ppc") == 0) {
      ppc = true;
    } else if (strcmp(argv[i], "-lcstats") == 0) {
      lcstats = true;
    } else if (strcmp(argv[i], "-gml") == 0) {
      gml = true;
    } else if (strcmp(argv[i], "-findk") == 0) {
      findk = true;
    } else if (strcmp(argv[i], "-gen") == 0) {
      gen = true;
    } else if (strcmp(argv[i], "-stratified") == 0) {
      stratified = true;
      if (rfreq == 1)
	rfreq = 100;
    } else if (strcmp(argv[i], "-batch") == 0) {
      batch = true;
      online = false;
      rfreq = 1;
    } else if (strcmp(argv[i], "-link-sampling") == 0) {
      link_sampling = true;
      online = true;
      batch = false;
      rfreq = 1;
    } else if (strcmp(argv[i], "-nodelay") == 0) {
      nodelay = true;
    } else if (strcmp(argv[i], "-rnode") == 0) {
      rnode = true;
      if (rfreq == 1)
	rfreq = 100;
    } else if (strcmp(argv[i], "-rpair") == 0) {
      rpair = true;
      if (rfreq == 1)
	rfreq = 100;
    } else if (strcmp(argv[i], "-load") == 0) {
      load = true;
      location = string(argv[++i]);
    } else if (strcmp(argv[i], "-load-validation") == 0) {
      val_load = true;
      val_file_location = string(argv[++i]);
    } else if (strcmp(argv[i], "-load-test") == 0) {
      test_load = true;
      test_file_location = string(argv[++i]);
    } else if (strcmp(argv[i], "-adamic-adar") == 0) {
      adamic_adar = true;
    } else if (strcmp(argv[i], "-scale") == 0) {
      scale = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-n") == 0) {
      n = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-k") == 0) {
      k = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-disjoint") == 0) {
      disjoint = true;
    } else if (strcmp(argv[i], "-label") == 0) {
      label = string(argv[++i]);
    } else if (strcmp(argv[i], "-nthreads") == 0) {
      nthreads = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-orig") == 0) {
      orig = true;
    } else if (strcmp(argv[i], "-infset") == 0) {
      massive = true;
    } else if (strcmp(argv[i], "-single") == 0) {
      single = true;
    } else if (strcmp(argv[i], "-itype") == 0) {
      itype = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-eta-type") == 0) {
      eta_type = string(argv[++i]);
    } else if (strcmp(argv[i], "-nmi") == 0) {
      ground_truth_fname = string(argv[++i]);
      nmi = true;
    } else if (strcmp(argv[i], "-rfreq") == 0) {
      rfreq = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-accuracy") == 0) {
      accuracy = true;
    } else if (strcmp(argv[i], "-stopthresh") == 0) {
      stopthresh = atof(argv[++i]);
    } else if (strcmp(argv[i], "-inf") == 0) {
      infthresh = atof(argv[++i]);
    } else if (strcmp(argv[i], "-nonuniform") == 0) {
      nonuniform = true;
    } else if (strcmp(argv[i], "-bmark") == 0) {
      bmark = true;
    } else if (strcmp(argv[i], "-randzeros") == 0) {
      randzeros = true;
    } else if (strcmp(argv[i], "-preprocess") == 0) {
      preprocess = true;
      massive = true;
    } else if (strcmp(argv[i], "-strid") == 0) {
      strid = true;
    } else if (strcmp(argv[i], "-groups-file") == 0) {
      groups_file = string(argv[++i]);
    } else if (strcmp(argv[i], "-logl") == 0) {
      logl = true;
    } else if (strcmp(argv[i], "-max-iterations") == 0) {
      max_iterations = atoi(argv[++i]);
    } else if (strcmp(argv[i], "-no-stop") == 0) {
      use_validation_stop = false;
    } else if (strcmp(argv[i], "-seed") == 0) {
      rand_seed = atof(argv[++i]);
    } else if (strcmp(argv[i], "-heldout-ratio") == 0) {
      hol_ratio = atof(argv[++i]);
    } else if (strcmp(argv[i], "-load-test-sets") == 0) {
      load_test_sets_opt = true;
    } else if (strcmp(argv[i], "-link-thresh") == 0) {
      link_thresh = atof(argv[++i]);
    } else if (strcmp(argv[i], "-lt-min-deg") == 0) {
      lt_min_deg = atof(argv[++i]);
    } else if (strcmp(argv[i], "-init-communities") == 0) {
      init_comm = true;
      init_comm_fname = string(argv[++i]);
    }
    ++i;
  };

  assert (!(batch && online));

  Env env(n, k, massive, single, batch, stratified, 
	  nodelay, rpair, rnode, 
	  load, location, 
	  val_load, val_file_location, 
	  test_load, test_file_location,
	  load_test_sets_opt,
	  hol_ratio,
	  adamic_adar,
	  scale, disjoint,
	  force_overwrite_dir, datfname, 
	  ppc, run_gap, gen, label, nthreads, itype, eta_type,
	  nmi, ground_truth_fname, rfreq, 
	  accuracy, stopthresh, infthresh, 
	  nonuniform, bmark, randzeros, preprocess, 
	  strid, groups_file, logl,
	  max_iterations, use_validation_stop, rand_seed,
	  link_thresh, lt_min_deg,
	  init_comm, init_comm_fname,
	  link_sampling, gml, findk);

  env_global = &env;
  Network network(env);
  if (!run_gap && gen) {
    if (orig) {
      info("+ generating mmsb network (with full blockmodel)\n");
      double alpha = (double)1.0 / env.k;
      MMSBOrig mmsborig(env, network);
      mmsborig.gen(alpha);
      exit(0);
    } else {
      info("+ generating mmsb network\n");
      double alpha = 0.05; //(double)1.0 / env.k;
      MMSBGen mmsbgen(env, network, false);
      mmsbgen.gen(alpha);
      exit(0);
    }
  }
  if (network.read(datfname.c_str()) < 0) {
    fprintf(stderr, "error reading %s; quitting\n", datfname.c_str());
    return -1;
  }
  info("+ network: n = %d, ones = %d, singles = %d\n", 
	  network.n(), 
	  network.ones(), network.singles());

  env.n = network.n() - network.singles();

  if (!gml) {
    info("+ logging and assessing convergence "
	    "every %d iterations\n", env.reportfreq);
  }

  if (ppc && run_gap)
    assert(0);
  else if (ppc) {
    info("+ running mmsb posterior predictive checks\n");
    MMSBGen mmsbgen(env, network, true);
    mmsbgen.ppc();
    exit(0);
  }

  if (lcstats) {
    info("+ computing lc stats\n");
    MMSBGen mmsbgen(env, network, true);
    mmsbgen.get_lc_stats();
    exit(0);
  }

  if (gml) {
    info("+ generating GML file\n");
    MMSBGen mmsbgen(env, network, false);
    mmsbgen.gml();
    exit(0);
  }

  if (findk) {
    uint32_t max;
    double avg;
    network.deg_stats(max, avg);
    FastInit fastinit(env, network, max);
    fastinit.batch_infer();
    exit(0);
  }

  if (orig) { // Airoldi et al.
    info("+ running mmsb inference with full blockmodel\n");
    MMSBInferOrig mmsb(env, network, itype);
    mmsb.batch_infer();
    exit(0);
  }

  if (link_sampling) {
    info("+ running link sampling \n");
    LinkSampling ls(env, network);
    ls.infer();
    exit(0);
  }
  
  if (single) {
    SBM sbm(env, network);
    info("+ running stochastic blockmodel inference\n");
    if (!massive)
      sbm.batch_infer();
    else
      sbm.infer();
    exit(0);
  }
  
  if (batch) {
    MMSBInfer mmsb(env, network);
    info("+ running mmsb batch inference\n");
    mmsb.batch_infer();
    exit(0);
  }

  if (massive) {
    FastAMM fastamm(env, network);
    info("+ running mmsb inference (with infset network option)\n");
    fastamm.infer();
    exit(0);
  } 
  
  if (stratified && rnode) {
    FastAMM2 fastamm2(env, network);
    info("+ running mmsb inference (with stratified random node option)\n");
    fastamm2.infer();
    exit(0);
  } else {
    MMSBInfer mmsb(env, network);
    info("+ running mmsb inference\n");
    mmsb.infer();
  }
}

void
usage()
{
  fprintf(stdout, "\nSVINET: fast stochastic variational inference of undirected networks\n"
	  "svinet [OPTIONS]\n"
	  "\t-help\t\tusage\n\n"
	  "\t-file <name>\tinput tab-separated file with a list of undirected links\n\n"
	  "\t-n <N>\t\tnumber of nodes in network\n\n"
	  "\t-k <K>\t\tnumber of communities\n\n"
	  "\t-batch\t\trun batch variational inference\n\n"
	  "\t-stratified\tuse stratified sampling\n\t * use with rpair or rnode options\n\n"
	  "\t-rnode\t\tinference using random node sampling\n\n"
	  "\t-rpair\t\tinference using random pair sampling\n\n"
	  "\t-load-validation <fname>\t\tuse the pairs in the file as the validation set for convergence\n\n"
	  "\t-load-test <fname>\t\tuse the pairs in the file as the test set\n\n"
	  "\t-label\t\ttag output directory\n\n"
	  "\t-link-sampling\tinference using link sampling \n\n"
	  "\t-infset\t\tinference using informative set sampling\n\n"
	  "\t-preprocess\tpreprocess to run informative set sampling\n\n"
	  "\t-rfreq\t\tset the frequency at which\n\t * convergence is estimated\n\t * statistics, e.g., heldout likelihood are computed\n\n"
	  "\t-max-iterations\t\tmaximum number of iterations (use with -no-stop to avoid stopping in an earlier iteration)\n\n"
	  "\t-no-stop\t\tdisable stopping criteria\n\n"
	  "\t-seed\t\tset GSL random generator seed\n\n"
	  "\t-gml\t\tgenerate a GML format file that visualizes link communities\n\n"
	  );
  fflush(stdout);
}

void
test()
{
  FastQueue::static_initialize(100, 0.0001);
  FastQueue f;
  f.update(0, 10);
  f.update(1, 70);
  f.update(2, 15);
  f.update(3, 3);
  f.update(4, 1);
  
  double v = .0;
  f.find(0, v);
  printf("key:%d,val:%.3f\n", 0, v);
  f.find(1, v);
  printf("key:%d,val:%.3f\n", 1, v);
  f.find(2, v);
  printf("key:%d,val:%.3f\n", 2, v);
  v = .0;
  f.find(3, v);
  printf("key:%d,val:%.3f\n", 3, v);
  f.find(4, v);
  printf("key:%d,val:%.3f\n", 4, v);

  f.update(1, 80);
  f.update(1, 90);
  f.update(2, 30);

  exit(-1);
}
