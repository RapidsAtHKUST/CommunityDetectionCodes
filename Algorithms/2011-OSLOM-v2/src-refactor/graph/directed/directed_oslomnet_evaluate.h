#include <iostream>
#include <deque>
#include <graph/oslom_net_global.h>

using namespace std;

inline double log_zero(double a) {
    if (a <= 0)
        return -1e20;
    else
        return log(a);
}

class oslomnet_evaluate : public oslomnet_louvain {
public:
    oslomnet_evaluate(deque<deque<int> > &b, deque<deque<pair<int, double> > > &c, deque<int> &d)
            : oslomnet_louvain() {
        set_graph(b, c, d);
        set_maxbord();
        set_changendi_cum();
    };

    oslomnet_evaluate(string a) : oslomnet_louvain() {
        set_graph(a);
        set_maxbord();
        set_changendi_cum();
    };

    oslomnet_evaluate(map<int, map<int, pair<int, double> > > &A) : oslomnet_louvain() {
        set_graph(A);
        set_maxbord();
        set_changendi_cum();
    };

    ~oslomnet_evaluate() {};

    double CUP_both(const deque<int> &_c_, deque<int> &gr_cleaned, int);

    double CUP_check(const deque<int> &_c_, deque<int> &gr_cleaned, int);

    double group_inflation(const deque<int> &_c_, deque<int> &gr_cleaned, int);

    int try_to_assign_homeless_help(module_collection &module_coll, map<int, deque<int> > &to_check);

private:
    double CUP_iterative(const deque<int> &_c_, deque<int> &gr_cleaned, int);

    double CUP_search(const deque<int> &_c_, deque<int> &gr_cleaned, int);

    void erase_cgroup(int wnode);

    void insert_cgroup(int wnode);

    bool erase_the_worst(int &wnode);

    int set_maxbord();

    void set_cgroup_and_neighs(const deque<int> &G);

    double all_external_test(int kout_g_in, int tmin, int kout_g_out, int tmout,
                             int Nstar, int nneighs, const double &max_r_one, const double &maxr_two,
                             deque<int> &gr_cleaned, bool only_c, weighted_tabdeg &previous_tab_c);

    double cup_on_list(cup_data_struct &a, deque<int> &gr_cleaned);

    void get_external_scores(weighted_tabdeg &neighs, cup_data_struct &fitness_label_to_sort, int kout_g_in, int tmin,
                             int kout_g_out, int tmout,
                             int Nstar, int nneighs, const double &max_r, bool only_c, weighted_tabdeg &previous_tab_c);

    double CUP_runs(weighted_tabdeg &previous_tab_c, weighted_tabdeg &previous_tab_n, int kin_cgroup_prev,
                    int ktot_cgroup_prev_in, int ktot_cgroup_prev_out, deque<int> &gr_cleaned, bool only_c,
                    int number_of_runs);

    void
    initialize_for_evaluation(const deque<int> &_c_, weighted_tabdeg &previous_tab_c, weighted_tabdeg &previous_tab_n,
                              int &kin_cgroup_prev, int &ktot_cgroup_prev_in, int &ktot_cgroup_prev_out);

    void
    initialize_for_evaluation(weighted_tabdeg &previous_tab_c, weighted_tabdeg &previous_tab_n, int &kin_cgroup_prev,
                              int &ktot_cgroup_prev_in, int &ktot_cgroup_prev_out);

    double partial_CUP(weighted_tabdeg &previous_tab_c, weighted_tabdeg &previous_tab_n, int kin_cgroup_prev,
                       int ktot_cgroup_prev_in, int ktot_cgroup_prev_out, deque<int> &border_group, bool only_c);

    void set_changendi_cum();

    void insertion(int changendi);

    bool insert_the_best();

    /* DATA ***************************************************/
    double max_r_bord;                                // this is the maximum r allowed for the external nodes (we don't want to look at all the graph, it would take too long)
    int maxb_nodes;                                    // this is the maximum number of nodes allowed in the border (similar as above)
    deque<double> changendi_cum;                    // this is the cumulative distribution of the number of nodes to add to the cluster in the group_inflation function
    // ************* things to update *************************
    weighted_tabdeg cgroup;                                    //*
    weighted_tabdeg neighs;                                    //*
    //*
    int kin_cgroup;                                            //*
    int ktot_cgroup_in;                                        //*
    int ktot_cgroup_out;                                    //*
    /*********************************************************/
};
