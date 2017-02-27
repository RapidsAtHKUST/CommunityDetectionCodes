//
// Created by cheyulin on 2/27/17.
//

#ifndef INC_2009_LFM_TABDEGREE_H
#define INC_2009_LFM_TABDEGREE_H

#include <cmath>
#include <ctime>

#include <iostream>
#include <deque>
#include <set>
#include <vector>
#include <map>
#include <iterator>
#include <algorithm>

using namespace std;
typedef multiset<pair<double, int>> muspi;

class TabDegree {
public:
    bool is_internal(int);

    void edinsert(int, double);

    bool erase(int);

    double indegof(int);

    int size() { return nodes_indeg.size(); };

    void print_nodes(ostream &);

    int best_node();

    int worst_node();

private:
    map<int, muspi::iterator> nodes_indeg;                // for each node belonging to the cluster, here you have where to find the internal degree
    muspi number_label;
};


#endif //INC_2009_LFM_TABDEGREE_H
