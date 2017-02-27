#include "util/input_output/tab_degree.h"

int main() {
    TabDegree C;
    C.edinsert(2, 14);
    C.edinsert(7, 24);
    C.edinsert(10, -1.2);
    C.edinsert(11, 12.8);
    cout << "--------------------" << endl;
    C.print_nodes(cout);
    C.erase(11);
    cout << "--------------------" << endl;
    C.print_nodes(cout);
    cout << "indegof " << C.indegof(10) << endl;
    cout << "best node " << C.worst_node() << endl;
    int best_node = C.worst_node();
    C.erase(best_node);
    cout << "--------------------" << endl;
    C.print_nodes(cout);
    return 0;
}
