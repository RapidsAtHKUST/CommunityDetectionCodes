#include <util/graph/undirected_network.h>
#include <algorithm/undir_weighted_tabdeg.h>

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
