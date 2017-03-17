#include <graph/undirected/undirected_network.h>
#include <graph/undirected/undir_weighted_tabdeg.h>
#include <graph/undirected/undirected_oslomnet_evaluate.h>

class oslom_net_global : public oslomnet_evaluate {

public:

    oslom_net_global(int_matrix &b, deque<deque<pair<int, double> >> &c, deque<int> &d);

    oslom_net_global(string a);

    oslom_net_global(map<int, map<int, pair<int, double> > > &A);

    ~oslom_net_global() {};

    void hint(module_collection &minimal_modules, string filename);

    void load(string filename, module_collection &Mall);


    void print_modules(bool not_homeless, string tp, module_collection &Mcoll);

    void print_modules(bool not_homeless, ostream &out1, module_collection &Mcoll);


    int try_to_assign_homeless(module_collection &Mcoll, bool anyway);

    void get_covers(string cover_file, int &soft_partitions_written, int gruns);

    void ultimate_cover(string cover_file, int soft_partitions_written, string final_cover_file);

    void print_statistics(ostream &outt, module_collection &Mcoll);


private:


    void from_matrix_to_module_collection(int_matrix &good_modules_to_prune, DD &bscores_good,
                                          module_collection &minimal_modules);

    void get_cover(module_collection &minimal_modules);

    int try_to_merge_discarded(int_matrix &discarded, int_matrix &good_modules_to_prune, int_matrix &new_discarded,
                               deque<double> &bscores_good);

    void get_single_trial_partition(int_matrix &good_modules_to_prune, deque<double> &bscores_good);

    void single_gather(int_matrix &good_modules_to_prune, deque<double> &bscores_good, int);

    void check_minimality_all(int_matrix &A, DD &bss, module_collection &minimal_modules);

    void
    check_minimality_matrix(int_matrix &A, DD &bss, module_collection &minimal_modules, int_matrix &suggestion_matrix,
                            deque<double> &suggestion_bs, int counter);

    bool check_minimality(deque<int> &group, double &bs_group, module_collection &minimal_modules,
                          int_matrix &suggestion_matrix, deque<double> &suggestion_bs);

    int check_unions_and_overlap(module_collection &mall, bool only_similar = false);

    void check_existing_unions(module_collection &mall);

    bool fusion_module_its_subs(const DI &A, int_matrix &its_submodules);

    bool fusion_with_empty_A(int_matrix &its_submodules, DI &grc1, double &bs);

    bool check_fusion_with_gather(module_collection &mall);

    int check_intersection(module_collection &Mcoll);

    int check_intersection(deque<int> &to_check, module_collection &Moll);

    int fusion_intersection(set<pair<int, int>> &pairs_to_check, module_collection &Mcoll);

    bool decision_fusion_intersection(int ai1, int ai2, deque<int> &new_insertions, module_collection &Mcoll,
                                      double prev_over_percentage);


};


class oslom_module {
public:
    oslom_module(int a) {
        nc = 1;
        kout = a;
        ktot = a;
    };

    ~oslom_module() {};
    int nc;
    int kout;
    int ktot;
};

typedef map<int, pair<int, double> > mapip;
typedef map<int, oslom_module> map_int_om;

void prints(map_int_om &M) {
    for (map_int_om::iterator itm = M.begin(); itm != M.end(); itm++)
        cout << "module: " << itm->first << "\t\t\t\tnc= " << itm->second.nc << "\t ktot= " << itm->second.ktot
             << "\t kout= " << itm->second.kout << endl;
}

class oslomnet_louvain : public static_network {
public:
    oslomnet_louvain() : static_network() {};

    ~oslomnet_louvain() {};

    int collect_raw_groups_once(deque<deque<int> > &);

private:
    void weighted_favorite_of(const int &node, int &fi, int &kp, int &kop);

    void unweighted_favorite_of(const int &node, int &fi, int &kp, int &kop);

    void single_pass_unweighted();

    void single_pass_weighted();

    inline void update_modules(const int &i, const int &fi, const int &kp, const int &kpo);

    void module_initializing();

    void set_partition_collected(deque<deque<int> > &M);

    //int check_all();
    map<int, oslom_module> label_module;
    deque<int> vertex_label;
    deque<int> vertex_order;
    deque<bool> vertex_to_check;
    deque<bool> vertex_to_check_next;
    int nodes_changed;
};





