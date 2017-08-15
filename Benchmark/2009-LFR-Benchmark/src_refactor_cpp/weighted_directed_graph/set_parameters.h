//
// Created by cheyulin on 2/27/17.
//

#ifndef INC_2009_LFM_SET_PARAMETERS_H
#define INC_2009_LFM_SET_PARAMETERS_H


#ifndef unlikely
#define unlikely -214741
#endif

#include <fstream>
#include <iostream>
#include <string>
#include <deque>

using namespace std;

#include "../util/cast.h"

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
    double mixing_parameter2;
    double beta;
    int overlapping_nodes;
    int overlap_membership;
    int nmin;
    int nmax;
    bool fixed_range;
    bool excess;
    bool defect;
    bool randomf;

    bool set(string &, string &);

    void set_random();

    bool arrange();

    deque<string> command_flags;
};


void statement();

bool set_from_file(string &file_name, Parameters &par1);

bool set_parameters(int argc, char *argv[], Parameters &par1);

#endif //INC_2009_LFM_SET_PARAMETERS_H
