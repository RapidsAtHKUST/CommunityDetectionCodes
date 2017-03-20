#include <iostream>
#include <map>

#include <util/common/cast.h>
#include <util/common/random.h>

using namespace std;

// to insert a new parameter there are four steps:
// 1- define it in the class
// 2- initialize it
// 3- set the flag
// 4- set the parameter
void general_program_statement(char *b);

void error_statement(char *b);

class Parameters {
public:
    Parameters();

    ~Parameters() {};

    void print();

    bool _set_(int argc, char *argv[]);

    //*******************************************************
    string file1;
    string file2;
    string file_load;
    int seed_random;
    double threshold;
    int clean_up_runs;
    int inflate_runs;
    int inflate_stopper;
    double equivalence_parameter;
    int CUT_Off;
    int maxborder_nodes;
    double maxbg_ordinary;
    int iterative_stopper;
    int minimality_stopper;
    double hierarchy_convergence;
    int Or;
    int hier_gather_runs;
    double coverage_inclusion_module_collection;
    double coverage_percentage_fusion_left;
    double check_inter_p;
    double coverage_percentage_fusion_or_submodules;
    bool print_flag_subgraph;
    bool value;
    bool value_load;
    bool fast;
    bool weighted;
    bool homeless_anyway;
    int max_iteration_convergence;
    deque<string> to_run;
    deque<string> to_run_part;
    int infomap_runs;
    int copra_runs;
    int louvain_runs;
private:
    map<string, int> command_flags;

    bool set_flag_and_number(double &threshold, int &argct, int argc, char *argv[], double min_v, double max_v,
                             string warning);

    bool set_flag_and_number(int &, int &argct, int argc, char *argv[], int min_v, int max_v, string warning);

    bool
    set_flag_and_number_external_program(string program_name, int &argct, int &number_to_set, int argc, char *argv[]);
};

