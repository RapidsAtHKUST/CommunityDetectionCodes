#ifndef ENV_HH
#define ENV_HH

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <map>
#include <list>
#include <vector>
#include "matrix.hh"
#include "log.hh"

typedef uint8_t yval_t;

typedef D2Array<yval_t> AdjMatrix;
typedef D2Array<double> Matrix;
typedef D3Array<double> D3;
typedef D2Array<KV> MatrixKV;
typedef D1Array<KV> KVArray;
typedef map<uint32_t, KV *> KVMap;
typedef list<KV *> KVList;

typedef std::map<uint32_t, uint32_t> IDMap;
typedef std::map<uint32_t, double> DoubleMap;
typedef std::map<uint32_t, uint32_t> FreqMap;
typedef std::map<string, uint32_t> FreqStrMap;
typedef std::map<string, uint32_t> StrMap;
typedef std::map<uint32_t, string> StrMapInv;
typedef D1Array<std::vector<uint32_t> *> SparseMatrix;
typedef D1Array<std::list<uint16_t> > StaticSparseMatrix16;
typedef std::vector<Edge> EdgeList;
typedef std::map<uint32_t, bool> NodeMap;
typedef std::map<uint32_t, bool> BoolMap;
typedef std::map<uint32_t, uint32_t> NodeValMap;
typedef std::map<uint32_t, vector<uint32_t> > MapVec;
typedef std::map<uint32_t, std::list<uint16_t> > MapList;
typedef std::map<uint32_t, vector<KV> > MapVecKV;
typedef MapVec SparseMatrix2;
typedef std::map<Edge, bool> SampleMap;
typedef std::map<Edge, int> CountMap;
typedef std::map<Edge, double> ValueMap;
typedef std::map<uint32_t, string> StrMapInv;
typedef D1Array<IDMap> KMap;

class Env {
public:
  Env(uint32_t N, uint32_t K, bool massive,
      bool sbm, bool batch, bool strat, bool nodelay,
      bool rpair, bool rnode, bool load, string location, 
      bool val_load, string val_file_location, 
      bool test_load, string test_file_location, 
      bool load_test_sets,
      double hol_ratio,
      bool adamic,
      uint32_t scale,
      bool dis, bool force_overwrite_dir, string dfname,
      bool ppc, bool run_gap, bool gen, string label,
      uint32_t nthrs, uint32_t itype, string etype,
      bool nmi, string ground_truth_fname, uint32_t rfreq,
      bool accuracy, double sth, double inf, bool nu,
      bool bmark, bool randzeros, bool preprocess,
      bool strid, string groups_fname, bool alogl,
      uint32_t max_iterations, bool uvstop,
      double rand_seed, 
      double link_thresh, uint32_t lt_min_deg,
      bool init_comm, string init_comm_fname,
      bool link_sampling, bool gml, bool findk);

  ~Env() { fclose(_plogf); }

  static string prefix;
  static Logger::Level level;

  uint32_t n;
  uint32_t k;
  uint32_t t;
  uint32_t s;
  uint32_t m;
  uint32_t sets_mini_batch;

  bool informative_sampling;
  bool single;
  bool batch_mode;

  int illegal_likelihood;
  int max_draw_edges;
  double meanchangethresh;
  double alpha;
  double sbm_alpha;
  double infer_alpha;
  bool model_load;
  string gamma_location;
  bool load_heldout;
  string load_heldout_fname;
  bool load_test;
  string load_test_fname;
  bool create_test_precision_sets;
  bool load_test_sets;
  bool adamic_adar;
  uint32_t subsample_scale;

  double eta0;
  double eta1;
  double heldout_ratio;
  double precision_ratio;

  double eta0_dense;
  double eta1_dense;
  double eta0_uniform;
  double eta1_uniform;
  double eta0_sparse;
  double eta1_sparse;
  double eta0_gen;
  double eta1_gen;

  int reportfreq;
  double epsilon;
  double logepsilon;

  double tau0;
  double nodetau0;
  double nodekappa;
  double kappa;
  uint32_t online_iterations;
  uint32_t conv_nupdates;
  double conv_thresh1;
  double conv_thresh2;

  bool stratified;
  bool delaylearn;
  bool nolambda;
  bool undirected;
  bool randompair;
  bool randomnode;
  bool bfs;
  uint32_t bfs_nodes;
  bool citation;
  bool accuracy;
  double stopthresh;
  double infthresh;
  bool nonuniform;
  bool benchmark;
  bool randzeros;
  bool preprocess;
  bool strid;
  string groups_file;
  bool logl;
  uint32_t max_iterations;
  double seed;

  bool terminate;
  string datfname;

  // debug
  bool deterministic;

  // gen
  bool disjoint;
  bool gen;

  // ppc
  uint32_t ppc_ndraws;

  // gamma poisson
  double default_shape;
  double default_rate;

  uint16_t nthreads;
  string label;
  string eta_type;

  bool nmi;
  string ground_truth_fname;

  bool use_validation_stop;
  bool use_training_stop;
  bool single_heldout_set;
  double link_thresh;
  double lt_min_deg;
  bool use_init_communities;
  string init_communities_fname;
  bool compute_auc;
  bool link_sampling;

  uint64_t total_pairs;
  double ones_prob;
  double zeros_prob;

  template<class T> static void plog(string s, const T &v);
  static string file_str(string fname);
  static string file_str(string dir, string fname);

private:
  static FILE *_plogf;
};


template<class T> inline void
Env::plog(string s, const T &v)
{
  fprintf(_plogf, "%s: %s\n", s.c_str(), v.s().c_str());
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const string &v)
{
  fprintf(_plogf, "%s: %s\n", s.c_str(), v.c_str());
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const double &v)
{
  fprintf(_plogf, "%s: %.9f\n", s.c_str(), v);
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const bool &v)
{
  fprintf(_plogf, "%s: %s\n", s.c_str(), v ? "True": "False");
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const int &v)
{
  fprintf(_plogf, "%s: %d\n", s.c_str(), v);
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const unsigned &v)
{
  fprintf(_plogf, "%s: %d\n", s.c_str(), v);
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const short unsigned int &v)
{
  fprintf(_plogf, "%s: %d\n", s.c_str(), v);
  fflush(_plogf);
}

template<> inline void
Env::plog(string s, const uint64_t &v)
{
  fprintf(_plogf, "%s: %" PRIu64 "\n", s.c_str(), v);
  fflush(_plogf);
}

#ifdef __APPLE__
template<> inline void
Env::plog(string s, const long unsigned int &v)
{
  fprintf(_plogf, "%s: %lu\n", s.c_str(), v);
  fflush(_plogf);
}
#endif

inline string
Env::file_str(string fname)
{
  string s = prefix + fname;
  return s;
}

inline string
Env::file_str(string dir, string fname)
{
  string s = dir + fname;
  return s;
}

inline
Env::Env(uint32_t N, uint32_t K, bool massive,
	 bool sbm, bool batch, bool strat, bool nodelay,
	 bool rpair, bool rnode, bool load, string location, 
	 bool val_load, string val_file_location, 
	 bool test_load, string test_file_location, 
	 bool load_test_sets_opt, double hol_ratio,
	 bool adamic,
	 uint32_t scale,
	 bool dis, bool force_overwrite_dir, string dfname,
	 bool ppc, bool gap, bool gen1, string lbl,
	 uint32_t nthrs, uint32_t itype, string etype,
	 bool nmival, string gfname, uint32_t rfreq,
	 bool accuracy, double sth, double inf, bool nu,
	 bool bmark, bool rzeros, bool pre,
	 bool sid, string groups_fname, bool alogl,
	 uint32_t max_itr, bool uvstop,
	 double rand_seed,
	 double link_thresh, uint32_t lt_min_deg,
	 bool init_comm, string init_comm_fname,
	 bool link_sampling_opt, bool gml, bool findk)
  : n(N),
    k(K),
    t(2),

    /* 
       PARAMETER: [mini-batch]

       mini-batch size is #nodes/2 for random pair and stratified random pair
       options. Change the below option to modify this. For the other options,
       fixed sizes are used. The mini-batch size is 1 for random node sampling
       option, i.e., the single set associated with a node consisting of (n-1)
       node pairs is sampled. The mini-batch stratified random node option is
       also 1, i.e., the link or non-link set is sampled. Each set may have a
       variable number of node pairs.  With the massive option, an informative
       or non-informative set is sampled.
    */
    s(n/2),
    m(1),
    sets_mini_batch(N/100),

    informative_sampling(massive),
    single(sbm),
    batch_mode(batch),

    illegal_likelihood(-1),
    max_draw_edges(4096),

    /* 
       PARAMETER: [meanchangethresh]

       The threshold for assessing the convergence of the local step.
     */
    meanchangethresh(0.00001),

    /* 
       PARAMETER: [alpha]
       
       The Dirichlet prior parameter on the node mixed-memberships.
    */
    alpha((double)1 / k),
    sbm_alpha(0.5),

    infer_alpha(alpha),
    model_load(load),
    gamma_location(location),
    load_heldout(val_load),
    load_heldout_fname(val_file_location),
    load_test(test_load),
    load_test_fname(test_file_location),
    create_test_precision_sets(false),
    load_test_sets(load_test_sets_opt),
    adamic_adar(adamic),
    subsample_scale(scale),

    eta0(0),
    eta1(0),

    /* 
       PARAMETER: [heldout_ratio]

       The fraction of links that are heldout. An equal number of randomly
       chosen non-links are always held out.
    */
    heldout_ratio(hol_ratio),
    precision_ratio(0.001),

    eta0_dense(4700.59),
    eta1_dense(0.77),
    eta0_uniform(1.00),
    eta1_uniform(1.00),
    eta0_sparse(0.97),
    eta1_sparse(6.33),
    eta0_gen(eta0_dense),
    eta1_gen(eta1_dense),

    /* 
       PARAMETER: [reportfreq]

       The parameter governs how frequently (in iterations) convergence is
       assessed and various statistics are logged. Use the -rfreq option
       to control this. Do not change this here.
    */
    reportfreq(rfreq),

    /* 
       PARAMETER: [epsilon]

       The fixed value of the epsilon constant in the assortative MMSB model.
       See Gopalan et al. [2012]
    */
    epsilon(1e-30),
    logepsilon(log(epsilon)),

    /* 
       PARAMETER: [nodetau0, nodekappa, tau0, kappa]

       The learning parameters for the node membership (gamma) and community
       strength (lambda) variational parameter updates, respectively.

    */
    tau0(1024),
    nodetau0(1024),
    nodekappa(0.5),
    kappa(0.9),

    /* 
       PARAMETER: [online_iterations]

       The maximum number of iterations to run in each local step.
    */
    online_iterations(50),
    conv_nupdates(1000),
    conv_thresh1(1e-04),
    conv_thresh2(1e-02),

    nolambda(false),   // unused
    undirected(true),  // must always be true; unused
    
    randompair(false), 
    randomnode(false),

    // bfs
    bfs(false),
    bfs_nodes(10),
    citation(true),
    accuracy(accuracy),
    stopthresh(sth),
    infthresh(inf),
    nonuniform(nu),
    benchmark(bmark),
    randzeros(rzeros),
    preprocess(pre),
    strid(sid),
    groups_file(groups_fname),
    logl(alogl),
    max_iterations(max_itr),
    seed(rand_seed),
    
    terminate(false),

    datfname(dfname),
    deterministic(false),

    disjoint(dis),
    gen(gen1),

    //ppc
    ppc_ndraws(100),
    nthreads(nthrs),
    label(lbl),

    /*
      PARAMETER: [eta_type]
      
      This sets the prior on the community strengths. Default option computes
      these strengths based on link density of the network. You can also set 
      this is to "dense", "sparse", "uniform" or "fromdata" which sets the
      Beta parameters corresponding to dense communities, sparse communities
      etc. To further control this prior, change the Beta parameters of one of
      these types (i.e., eta0_dense, eta1_dense above) and set eta_type to that
      type.
    */
    eta_type(etype),
    nmi(nmival),
    ground_truth_fname(gfname),

    /* 
       PARAMETER: [use_validation_stop]

       Uses a heldout set of links and non-links to assess convergence.
       Do not change this.
    */
    use_validation_stop(uvstop),
    use_training_stop(false),
    single_heldout_set(true),
    use_init_communities(init_comm),
    init_communities_fname(init_comm_fname),
    compute_auc(false),
    link_sampling(link_sampling_opt)
{
  assert (!(batch && (strat || rnode || rpair)));

  if (nodelay)
    delaylearn = false;
  else
    delaylearn = true;

  if (strat)
    stratified = true;
  else
    stratified = false;

  if (rnode)
    randomnode = true;

  if (rpair)
    randompair = true;

  ostringstream sa;
  if (!gen && !gml) {
    sa << "n" << n << "-";
    sa << "k" << k;
    if (label != "")
      sa << "-" << label;
    else if (datfname.length() > 3 &&
	     datfname.find("mmsb_gen.dat") == string::npos) {
      string q = datfname.substr(0,2);
      if (q == "..")
	q = "xx";
      sa << "-" << q;
    }
    if (seed)
      sa << "-seed" << seed;
    if (batch)  {
      sa << "-batch";
      reportfreq = 1;
    } else if (single)
      sa << "-sbm";
    else if (massive)
      sa << "-infset";
    else if (link_sampling)
      sa << "-linksampling";
    else if (findk)
      sa << "-findk";
    else {
      if (stratified || delaylearn || nolambda || undirected || randomnode)
	sa << "-";
      if (subsample_scale > 1)
	sa << "scale" << subsample_scale << "-";
      if (stratified)
	sa << "S";
      if (delaylearn)
	sa << "U";
      if (nolambda)
	sa << "P";
      if (randompair)
	sa << "rpair";
      if (randomnode)
	sa << "rnode";
      if (nonuniform)
	sa << "R";
    }
    if (gap)
      sa << "-GAP";
    if (nthreads > 0)
      sa << "-T" << nthreads;
    if (itype > 0)
      sa << "-i" << itype;
  } else if (!gml) {
    if (disjoint)
      sa << "gend-";
    else
      sa << "gen-";
    sa << "n" << n << "-";
    sa << "k" << k << "-";

    if (eta0_gen == eta0_sparse)
      sa << "sparse";
    else if (eta0_gen == eta0_dense)
      sa << "dense";
    else
      sa << "regular";
  }
  prefix = gml ? "gml" : sa.str();
  level = Logger::INFO;

  if (!ppc) {
    fprintf(stdout, "+ Output directory: %s\n", prefix.c_str());
    fflush(stdout);
    assert (Logger::initialize(prefix, "infer.log",
			       force_overwrite_dir, level) >= 0);
    info("prefix = %s", prefix.c_str());
    _plogf = fopen(file_str("/param.txt").c_str(), "w");
    if (!_plogf)  {
      printf("cannot open param file:%s\n",  strerror(errno));
      exit(-1);
    }
    plog("nodes", n);
    plog("groups", k);
    plog("t", t);
    plog("minibatch (rpair or stratified rpair options only)", s);
    plog("mbsize", m);
    plog("alpha", alpha);
    plog("sbm_alpha", alpha);
    plog("heldout_ratio", heldout_ratio);
    plog("precision_ratio", precision_ratio);
    plog("stratified", stratified);
    plog("delaylearn", delaylearn);
    plog("nolambda", nolambda);
    plog("randomnode", randomnode);
    plog("gen", gen);
    plog("undirected", undirected);
    plog("gap", gap);
    plog("nthreads", nthreads);
    plog("stopthresh", stopthresh);
    plog("infthresh", infthresh);
    plog("randzeros", randzeros);
    plog("benchmark", benchmark);
    plog("max iterations", max_iterations);
    plog("seed", seed);
    plog("use validation stop", use_validation_stop);
    plog("gamma location", gamma_location);
    plog("link_thresh", link_thresh);
    plog("lt_min_deg", lt_min_deg);
    plog("epsilon", epsilon);
    plog("sets_mini_batch", sets_mini_batch);
    plog("use_init_communities", use_init_communities);
    plog("load_test_sets", load_test_sets);
    plog("val_load", val_load);
    plog("val_file_location", val_file_location);
    plog("test_load", test_load);
    plog("test_file_location", test_file_location);
    plog("reportfreq", reportfreq);
    plog("eta_type", eta_type);
  }

  if (!gen && !ppc) {
    string ndatfname = file_str("/network.dat");
    unlink(ndatfname.c_str());
    assert (symlink(datfname.c_str(), ndatfname.c_str()) >= 0);
  }
  unlink(file_str("/mutual.txt").c_str());
  info("+ done initializing env\n");
}

/*
   src: http://www.delorie.com/gnu/docs/glibc/libc_428.html
   Subtract the `struct timeval' values X and Y,
   storing the result in RESULT.
   Return 1 if the difference is negative, otherwise 0.
*/
inline int
timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
{
  /* Perform the carry for the later subtraction by updating y. */
  if (x->tv_usec < y->tv_usec) {
    int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
    y->tv_usec -= 1000000 * nsec;
    y->tv_sec += nsec;
  }
  if (x->tv_usec - y->tv_usec > 1000000) {
    int nsec = (x->tv_usec - y->tv_usec) / 1000000;
    y->tv_usec += 1000000 * nsec;
    y->tv_sec -= nsec;
  }

  /* Compute the time remaining to wait.
     tv_usec is certainly positive. */
  result->tv_sec = x->tv_sec - y->tv_sec;
  result->tv_usec = x->tv_usec - y->tv_usec;

  /* Return 1 if result is negative. */
  return x->tv_sec < y->tv_sec;
}

inline void
timeval_add (struct timeval *result, const struct timeval *x)
{
  result->tv_sec  += x->tv_sec;
  result->tv_usec += x->tv_usec;

  if (result->tv_usec >= 1000000) {
    result->tv_sec++;
    result->tv_usec -= 1000000;
  }
}

#endif
