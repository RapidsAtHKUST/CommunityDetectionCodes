#if !defined(STATIC_static_network_INCLUDED)
#define STATIC_static_network_INCLUDED

#include <map>
#include <set>
#include <deque>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace std;

#include <collection/module_collection.h>
#include <collection/wsarray.h>
#include <common/cast.h>
#include <common/combinatorics.h>
#include <common/histograms.h>

class static_network {
public:
    static_network() {};

    ~static_network();

    //int draw(string, bool);
    int draw(string);

    int draw_consecutive(string file_name1, string file_name2);

    int draw_with_weight_probability(string file_name);

    void print_id(const deque<int> &a, ostream &);

    void print_id(const deque<deque<int> > &, ostream &);

    void print_id(const deque<set<int> > &, ostream &);

    void print_id(const set<int> &, ostream &);

    void deque_id(deque<int> &);

    void set_subgraph(deque<int> &group, deque<deque<int> > &link_per_node,
                      deque<deque<pair<int, double> > > &weights_per_node);

    int translate(deque<int> &a);

    int translate(deque<deque<int> > &);

    int translate_anyway(deque<deque<int> > &ten);

    void get_id_label(map<int, int> &);

    int id_of(int a) { return vertices[a]->id_num; };

    int size() { return dim; };

    double stubs() { return oneM; };

    int kin_m(const deque<int> &);

    int kin_m(const set<int> &);

    pair<int, int> ktot_m(const deque<int> &);

    pair<int, int> ktot_m(const set<int> &);

    void set_graph(map<int, map<int, pair<int, double> > > &A);

    bool set_graph(string file_name);

    void set_graph(deque<deque<int> > &link_per_node, deque<deque<pair<int, double> > > &weights_per_node,
                   deque<int> &label_rows);

    void clear();

    void set_proper_weights();

    void set_connected_components(deque<deque<int> > &);

    int propagate_distances(deque<int> &new_shell, set<int> &already_gone, deque<pair<int, int> > &distances_node,
                            int shell, deque<double> &ML, int &, int);

    void same_component(int, set<int> &);

    int set_upper_network(map<int, map<int, pair<int, double> > > &neigh_weight_f, module_collection &module_coll);

    void print_degree_of_homeless(DI &homel, ostream &outt);

protected:
    class vertex {
    public:
        vertex(int b, int c, int preall_i, int preall_o);

        ~vertex();

        pair<int, int> kplus_m(const deque<int> &);

        pair<double, double> kplus_w(const deque<int> &);

        pair<int, int> kplus_m(const set<int> &);

        int id_num;                            // id
        double instrength;                    // sum of the weights
        int instub_number;                    // number of stubs
        double outstrength;                    // sum of the weights
        int outstub_number;                    // number of stubs
        wsarray *inlinks;                    // array with label of neighbor, multiple links, sum of the weights towards it
        wsarray *outlinks;                    // array with label of neighbor, multiple links, sum of the weights towards it
        //deque<double> in_original_weights;
        deque<double> out_original_weights;
    };

    int dim;                                    // number of nodes
    int oneM;                                    // number of in(out)-stubs
    deque<vertex *> vertices;

    void set_oneM_etc();
};

#endif
