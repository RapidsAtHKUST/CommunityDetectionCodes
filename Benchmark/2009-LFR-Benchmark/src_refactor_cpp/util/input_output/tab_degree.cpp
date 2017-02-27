//
// Created by cheyulin on 2/27/17.
//

#include "tab_degree.h"

bool TabDegree::is_internal(int a) {
    map<int, muspi::iterator>::iterator itm = nodes_indeg.find(a);
    if (itm == nodes_indeg.end())
        return false;
    else
        return true;
}


// this function inserts element a (or edit it if it was already inserted)
void TabDegree::edinsert(int a, double kp) {
    erase(a);
    muspi::iterator itms = number_label.insert(make_pair(kp, a));
    nodes_indeg.insert(make_pair(a, itms));
}

bool TabDegree::erase(int a) {        // this function erases element a if exists (and returns true)
    map<int, muspi::iterator>::iterator itm = nodes_indeg.find(a);
    if (itm != nodes_indeg.end()) {
        number_label.erase(itm->second);
        nodes_indeg.erase(itm);
        return true;
    }
    return false;
}

double TabDegree::indegof(int a) {        // return the internal degree of a, 0 if it's not internal
    map<int, muspi::iterator>::iterator itm = nodes_indeg.find(a);
    if (itm != nodes_indeg.end())
        return itm->second->first;
    else
        return 0;
}

void TabDegree::print_nodes(ostream &outb) {
    for (map<int, muspi::iterator>::iterator itm = nodes_indeg.begin(); itm != nodes_indeg.end(); itm++)
        outb << itm->first << "\t" << itm->second->first << endl;
}

int TabDegree::best_node() {
    muspi::iterator itm = number_label.end();
    if (number_label.size() == 0)
        return -1;
    itm--;
    return itm->second;
}

int TabDegree::worst_node() {
    muspi::iterator itm = number_label.begin();
    if (number_label.size() == 0)
        return -1;
    return itm->second;
}