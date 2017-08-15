//
// Created by cheyulin on 1/4/17.
//
#ifndef INC_2009_LFM_SET_PARAMETERS_H
#define INC_2009_LFM_SET_PARAMETERS_H

#include <string>
#include <deque>
#include <iostream>
#include <fstream>

#include "../util/cast.h"

#ifndef unlikely
#define unlikely -214741
#endif

using namespace std;

class Parameters {
public:
    Parameters();

    ~Parameters() {};
    int num_nodes;
    double average_k;
    int max_degree;
    double tau;
    double tau2;
    double mixing_parameter;
    int overlapping_nodes;
    int overlap_membership;
    int nmin;
    int nmax;
    bool fixed_range;
    bool excess;
    bool defect;
    bool randomf;
    double clustering_coeff;

    bool set(string &, string &);

    void set_random();

    bool arrange();

    deque<string> command_flags;
};


void statement();

bool set_from_file(string &file_name, Parameters &par1);

bool set_parameters(int argc, char *argv[], Parameters &par1);

#endif //INC_2009_LFM_SET_PARAMETERS_H
