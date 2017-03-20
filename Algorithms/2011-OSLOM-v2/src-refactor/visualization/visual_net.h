#if !defined(VISUAL_network_INCLUDED)
#define VISUAL_network_INCLUDED

#include "static_network.h"

class visual_network : public static_network {
public:
    visual_network() : static_network() {
        edge_width = 0.1;
        label_size = 0;
    };

    ~visual_network() {};

    void internal_link_map(multimap<int, int> &int_edges, deque<int> &group);

    //double edge_list_map(map<int, int> & edge_list_app);
    double draw_gnuplot(map<int, double> &lab_x, map<int, double> &lab_y, map<int, double> &lab_r, string position_file,
                        string gfile, deque<string> &edge_append);

    double
    draw_gnuplot(map<int, double> &lab_x, map<int, double> &lab_y, map<int, double> &lab_r, deque<string> &edge_append,
                 multimap<int, int> &internal_links);

    double draw_gnuplot(map<int, double> &lab_x, map<int, double> &lab_y, map<int, double> &lab_r, string position_file,
                        string gfile);

    double draw_pajek(map<int, double> &lab_x, map<int, double> &lab_y, map<int, double> &lab_r);

    double draw_pajek(deque<double> &angles, deque<double> &radii, double radius, double origin_x, double origin_y);

    double with_colors(deque<deque<int> > &ten);

    int find_position_goniometric(int secs);

    int circles(int secs, map<int, int> &lab_sizes, deque<double> &angles, deque<double> &radii, double radius);

private:
    double label_size;
    double edge_width;

    double relabel(map<int, double> &lab_x, map<int, double> &lab_y, map<int, double> &lab_r, map<int, double> &labx_h,
                   map<int, double> &laby_h, map<int, double> &labr_h);

    //***************************** methods of find_position_goniometric **************************
    double distance_node(int node1);

    bool swap();

    deque<int> pos;
    deque<double> sin_tab;
    double theta;
    //***************************** methods of find_position_goniometric **************************
};

#endif
