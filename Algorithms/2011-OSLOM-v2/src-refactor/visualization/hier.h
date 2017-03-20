#if !defined(HIER_network_INCLUDED)
#define HIER_network_INCLUDED

#include "position.h"
#include "visual_net.h"

#define LL 1.5

int check_position(map<int, double> &lab_x, map<int, double> &lab_y, map<int, double> &lab_r);

int get_partition_from_file_tp_format(string S, map<int, deque<int> > &Mlab);

int get_sizes(string file_for_sizes, map<int, int> &sizes);

int insert_isolated(visual_network &luca, map<int, int> &sizes);

int o_level(double origin_x, double origin_y, double radius, visual_network &luca, map<int, int> &sizes,
            map<int, double> &lab_x, map<int, double> &lab_y, map<int, double> &lab_r);

int another_level(string short_tpn, string netn, string tpn, map<int, double> &lab_x, map<int, double> &lab_y,
                  map<int, double> &lab_r, map<int, double> &lab_x_next, map<int, double> &lab_y_next,
                  map<int, double> &lab_r_next, int level, char *b1, deque<string> &edge_append);

int all_levels(int levels, string network_file);

#endif
