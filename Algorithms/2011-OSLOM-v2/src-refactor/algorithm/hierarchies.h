#include <string>
#include <deque>
#include <iostream>
#include <fstream>

#include <util/graph/louvain/oslom_net_global.h>
#include <util/common/cast.h>
#include <util/collection/module_collection.h>

using namespace std;

bool manipulate_string(string s, string netfile, string &outs);


/* this function is to call a different program */
void external_program_to_call(string network_file, oslom_net_global &matteo,
                              string plz_out, int &soft_partitions_written);


void translate_covers(string previous_tp, string new_tp, string short_tp, ostream &stout, int dim);

void no_singletons(char *directory_char, oslom_net_global &luca, module_collection &Mcoll);

bool write_tp_of_this_level(int level, oslom_net_global &luca, char *directory_char, int original_dim);

void oslom_level(oslom_net_global &luca, char *directory_char);




