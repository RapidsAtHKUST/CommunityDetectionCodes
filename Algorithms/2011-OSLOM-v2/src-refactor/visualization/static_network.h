#if !defined(STATIC_static_network_INCLUDED)
#define STATIC_static_network_INCLUDED

#include <iostream>
#include <map>
#include <set>
#include <deque>
#include <string>

#include <util/common/cast.h>
#include <util/common/combinatorics.h>
#include <util/collection/wsarray.h>

using namespace std;

void statement();

int parse_command_line(bool &value, string &s, string &s2, int argc, char *argv[]);

class static_network {
public:
    static_network() {};

    static_network(deque<deque<int> > &, deque<int> &);

    static_network(bool, deque<deque<int> > &, deque<deque<double> > &, deque<int> &);

    static_network(string);

    static_network(string file_name, bool &good_file);

    static_network(map<int, map<int, double> > &A);

    ~static_network();

    void add_isolated(int id_iso);

    int draw(string, bool);

    int draw(string);

    int draw_consecutive(string, string, bool);

    int draw_consecutive(string, string);

    void print_id(const deque<int> &a, ostream &);

    void print_id(const deque<deque<int> > &, ostream &);

    void print_id(const deque<set<int> > &, ostream &);

    void print_id(const set<int> &, ostream &);

    void deque_id(deque<int> &);

    int connected(string);

    void pajek_print_cluster(string file_name, deque<int> group, int number_of_shells);

    void print_subgraph(string, const deque<int> &);

    void print_component(string file_name, const deque<int> &group);

    void set_subgraph(deque<int> &, deque<deque<int> > &, deque<deque<double> > &);

    void print_random_version(string);

    void print_connected_components(ostream &);

    void same_component(int, set<int> &);

    void set_connected_components(deque<deque<int> > &);

    void set_connected_components(deque<set<int> > &);

    int translate(deque<deque<int> > &);

    void get_id_label(map<int, int> &);

    int get_degree(deque<int> &);

    int size() { return dim; };

    double edges() { return tstrength; };

    double kin(const deque<int> &);

    double kin(const set<int> &);

    double ktot(const deque<int> &);

    double ktot(const set<int> &);

    void erase_link(int, int);

    double newman_modularity(const double kin_g, const double ktot_g);

    double newman_modularity(set<int> &s);

    double newman_modularity(deque<set<int> > &);

    double newman_modularity(deque<int> &s);

    double newman_modularity(deque<deque<int> > &);

    int compute_betweeness_single_source(int, map<pair<int, int>, double> &);

    int component_betweeness(int, map<pair<int, int>, double> &, set<int> &);

    int all_betweeness(map<pair<int, int>, double> &);

    int id_of(int a) { return vertices[a]->id_num; };

    int set_mem_adj(deque<deque<int> > &);        // unweighted networks!
    int distances_from_i(int node, double &, deque<double> &ML, int);

    int propagate_distances(deque<int> &new_shell, set<int> &already_gone, deque<pair<int, int> > &distances_node,
                            int shell, deque<double> &ML, int &, int);

    int diameter_and_asp(double &average, int &diameter, deque<double> &);

    int knn(map<int, double> &);

    int knn(map<int, deque<double> > &knn_hist);

    void clustering_coefficient(map<int, deque<double> > &c_k);

    double clustering_coefficient();

    void set_graph(map<int, map<int, double> > &A);

    void set_graph(multimap<int, int> &A);

    bool set_graph(string file_name);

    void set_graph(bool weighted_, deque<deque<int> > &link_per_node, deque<deque<double> > &weights_per_node,
                   deque<int> &label_rows);

    void set_graph(deque<deque<int> > &link_per_node, deque<int> &label_rows);

    void clear();

    bool GN_bench(int nodes, int modules, double kout, double m_degree, deque<deque<int> > &ten);

    bool set_binary_random(const deque<int> &degrees);

    bool set_random_powlaw(int, double, double, int);

    bool set_random_powlaw_multiple(int num_nodes, double tau, double average_k, int max_degree, bool);

    bool set_multiple_random(const deque<int> &degrees, bool);

    void community_netknn(deque<deque<int> > &);

    void community_net(deque<deque<int> > &ten);

    double monte_carlo_asp();

    double
    draw_gnuplot(map<int, double> &lab_x, map<int, double> &lab_y, ostream &goo, bool internal, deque<deque<int> > M,
                 double width);

    void draw_pajek(map<int, double> &lab_x, map<int, double> &lab_y, string file_name, deque<deque<int> > M,
                    double scale_factor);

    void draw_pajek_directed(map<int, double> &lab_x, map<int, double> &lab_y, string file_name, deque<deque<int> > M,
                             double scale_factor,
                             deque<int> &node1, deque<int> &a2, deque<double> &w12);

protected:
    class vertex {
    public:
        vertex(int, int, int);

        ~vertex();

        double kplus(const deque<int> &);

        double kplus(const set<int> &);

        int id_num;
        double strength;
        wsarray *links;
    };

    int dim;                                    // number of nodes
    double tstrength;                            // number of links (weighted)
    bool weighted;
    deque<vertex *> vertices;
private:
    void propagate_bw(int *, int *, int, set<int> &, set<int> &, deque<int> &);

    void rewiring(deque<set<int> > &en);

    void next_shell(const set<int> &shell0, set<int> &shell1, set<int> &already);
};

#endif

