#if !defined(NET_vi_included)
#define NET_vi_included

#include <util/input_output/partition.h>

#include "hier.h"
#include "static_network.h"


#define UNFF -7412128

class netvi : public static_network {

public:
    netvi() : static_network() {};

    ~netvi() {};

    int set_all(int level, string network_file);

    int print_positions(ostream &go, int color, double siz, string outfile, int level, string network_file, bool first);

    int print_internal(ostream &go, int color, string outfile, int level, string network_file, double edge_width);

    int print_external(ostream &go, int color, string outfile, int level, string network_file, double edge_width);

    int print_label(ostream &go, int color, string outfile, int level, string network_file, double siz);

    int print_module_id(int module);

    int print_module_names(int module, map<int, string> &names);

    int print_names_file(ostream &go, int color, string outfile, int level, string network_file, double siz,
                         map<int, string> &names);

    int print_names_file(ostream &go, int color, string outfile, int level, string network_file, double siz,
                         map<int, string> &names, deque<int> &group_ids);

    map<int, deque<int> > node_modules;        // this maps the id into its submodule

private:
    deque<double> label_x;        // label, not id!
    deque<double> label_y;
    deque<pair<double, double> > internal_links;
    deque<pair<double, double> > external_links;

};

class netgnu {

public:
    netgnu() {};

    ~netgnu();

    int set_networks(int levels, string network_file);

    int set_position(int net, int color, double siz);

    int erase_position(int);

    int set_internal(int net, double edge_width);

    int erase_internal(int);

    int set_external(int net, double edge_width);

    int erase_external(int);

    int set_labs(int net, double siz);

    int erase_labs(int);

    int interface();

    int plot_commands();

    int read_names(string s);

    int print_id(int net, int module);

    int print_names(int net, int module);

    int print_names_on_graph(double siz);

private:
    deque<netvi *> networks;
    map<int, deque<double> > positions_commands;        // each row corresponds to a network - deque<double> are parameters
    map<int, deque<double> > internal_commands;        // each row corresponds to a network - deque<double> are parameters
    map<int, deque<double> > external_commands;        // each row corresponds to a network - deque<double> are parameters
    map<int, deque<double> > label_commands;        // each row corresponds to a network - deque<double> are parameters
    string network_file_saved;
    string outfile_saved;
    double minx;
    double maxx;
    double miny;
    double maxy;
    bool unset_key;
    double names_size;
    map<int, string> id_names;
};

#endif
