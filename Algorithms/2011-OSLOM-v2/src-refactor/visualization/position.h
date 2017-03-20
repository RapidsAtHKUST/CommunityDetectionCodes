#ifndef POSITION_HPP
#define POSITION_HPP
#define bseparator 1.3

#include <cmath>
#include <algorithm>
#include <map>
#include <deque>
#include <set>
#include <iostream>

using namespace std;
typedef multimap<double, int>::iterator map_idi;
typedef pair<map_idi, map_idi> pmap_idi;

class xypos {
public:
    xypos() {};

    ~xypos() {};

    bool erase(int);

    void edinsert(int, double, double);

    int size() { return xlabel.size(); };

    void print_points(ostream &);

    void clear();

    int neighbors(int a, deque<int> &group_intsec);

    void set_all(map<int, double> &lab_x, map<int, double> &lab_y, map<int, double> &lab_r, double);

    int move(int a);

    int move_all();

    void get_new_pos(map<int, double> &lab_x, map<int, double> &lab_y);

private:
    int xy_close(multimap<double, int> &m, double x, double dx, set<int> &close);

    map<int, double> label_radius;
    multimap<double, int> xlabel;
    multimap<double, int> ylabel;
    map<int, pmap_idi> label_points;
    double center_x;
    double center_y;
};

#endif
