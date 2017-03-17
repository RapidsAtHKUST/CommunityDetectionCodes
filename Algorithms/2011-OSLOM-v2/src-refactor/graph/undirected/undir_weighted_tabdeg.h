#include <cmath>
#include <map>
#include <algorithm>
#include <common/random.h>

using namespace std;

# define sqrt_two 1.41421356237
# define num_up_to 5
# define bisection_precision 1e-2

typedef multimap<double, pair<int, double> > cup_data_struct;

double compare_r_variables(double a, double b, double c, double d);

double right_error_function(double x);

double log_together(double minus_log_total, int number);

double fitted_exponent(int N);

double inverse_order_statistics(int sample_dim, int pos, const double &zerof, double lo, double hi);

bool equivalent_check(int pos_first, int pos_last, double &A_average, double &B_average, int equivalents, int Nstar,
                      const double &critical_xi);

bool equivalent_check_gather(cup_data_struct &a, int &until, const double &probability_a, const double &probability_b,
                             int Nstar, const double &critical_xi);

inline double hyper_table(int kin_node, int kout_g, int tm, int degree_node) {
    return LOG_TABLE.hyper(kin_node, kout_g, tm, degree_node);
}

inline double topological_05(int kin_node, int kout_g, int tm, int degree_node) {
    return LOG_TABLE.right_cumulative_function(degree_node, kout_g, tm, kin_node + 1) +
           ran4() * (hyper_table(kin_node, kout_g, tm, degree_node));
}

double
compute_global_fitness(int kin_node, int kout_g, int tm, int degree_node, double minus_log_total, int number_of_neighs,
                       int Nstar, double &boot_interval);

double compute_global_fitness_step(int kin_node, int kout_g, int tm, int degree_node, double minus_log_total,
                                   int number_of_neighs, int Nstar, double _step_);

double
compute_global_fitness_randomized_short(int kin_node, int kout_g, int tm, int degree_node, double minus_log_total);

inline double order_statistics_left_cumulative(int N, int pos, double x) {
    // the routine computes the probality c_pos=  p(X_pos <= x)
    // N is the total number of variables, pos is from 1 to N. N is the smallest.
    return LOG_TABLE.cum_binomial_right(N - pos + 1, N, x);
}

inline double pron_min_exp(int N, double xi) {
    // this should return the probability that the minimum of the quantiles p(c_min<=xi)
    //cout<<"-> "<<fitted_exponent(N)<<endl;
    return 1 - exp(-fitted_exponent(N) * xi);
}

inline double
compute_probability_to_stop(const double &a, const double &b, const double &critical_xi, int Nstar, int pos) {
    /*	this routine is to compute the bootstrap probaility that the node with extremes a and b will be below threshold
        when I already know that the average is below critical_xi */
    if (order_statistics_left_cumulative(Nstar, pos, b) <= critical_xi)
        return 1;
    return (inverse_order_statistics(Nstar, pos, critical_xi, (a + b) * 0.5, b) - a) / (b - a);
}

inline double compute_global_fitness_ofive(int kin_node, int kout_g, int tm, int degree_node, double minus_log_total,
                                           int number_of_neighs, int Nstar) {
    return compute_global_fitness_step(kin_node, kout_g, tm, degree_node, minus_log_total, number_of_neighs, Nstar,
                                       0.5);
}

inline double
compute_global_fitness_randomized(int kin_node, int kout_g, int tm, int degree_node, double minus_log_total,
                                  int number_of_neighs, int Nstar) {
    return compute_global_fitness_step(kin_node, kout_g, tm, degree_node, minus_log_total, number_of_neighs, Nstar,
                                       ran4());
}

class facts {
public:
    facts(int a, double b, multimap<double, int>::iterator c, int d) {
        internal_degree = a;
        minus_log_total_wr = b;
        fitness_iterator = c;
        degree = d;
    };

    ~facts() {};
    int degree;
    int internal_degree;
    double minus_log_total_wr;                                // wr is the right part of the exponential for the weights, this is the sum over the internal stubs of that
    multimap<double, int>::iterator fitness_iterator;
};

class weighted_tabdeg {
public:
    weighted_tabdeg() {};

    ~weighted_tabdeg() {};

    void _set_(weighted_tabdeg &);

    void clear();

    void edinsert(int a, int kp, int kt, double mtlw, double fit);

    bool erase(int a);

    void set_deque(deque<int> &);

    int size() { return lab_facts.size(); };

    void print_nodes(ostream &, deque<int> &);

    void set_and_update_group(int nstar, int nn, int kout_g, int tm, weighted_tabdeg &one);

    void set_and_update_neighs(int nstar, int nn, int kout_g, int tm, weighted_tabdeg &one);

    bool update_group(int a, int delta_degree, double delta_mtlw, int nstar, int nn, int kout_g, int tm, int kt,
                      deque<int> &to_be_erased);

    bool update_neighs(int a, int delta_degree, double delta_mtlw, int nstar, int kout_g, int tm, int kt);

    int best_node(int &lab, double &best_fitness, int kout_g, int Nstar, int nneighs, int tm);

    int worst_node(int &lab, double &worst_fitness, int kout_g, int Nstar, int nneighs, int tm);

    bool is_internal(int a);

    map<int, facts> lab_facts;                    // maps the label into the facts
    multimap<double, int> fitness_lab;            // maps the fitness into the label  (this can be optimized)

};

