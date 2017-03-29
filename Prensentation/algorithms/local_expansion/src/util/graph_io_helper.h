//
// Created by cheyulin on 5/12/16.
//

#ifndef CODES_YCHE_INPUT_OUTPUT_HANDLER_H
#define CODES_YCHE_INPUT_OUTPUT_HANDLER_H

#include <fstream>
#include <sstream>
#include <memory>
#include <iostream>
#include <map>
#include <vector>
#include <boost/regex.hpp>

namespace yche {
    using namespace std;

    struct Edge {
        int src_index_;
        int dst_index_;
        double edge_weight_;

        Edge(int src_index_, int dst_index_, double edge_weight_) :
                src_index_(src_index_), dst_index_(dst_index_), edge_weight_(edge_weight_) {}
    };

    vector<Edge> ReadWeightedEdgeList(string file_name_str);

    vector<pair<int, int>> ReadEdgeList(string file_name_str);
}

namespace std {
    ostream &operator<<(ostream &my_output_stream, yche::Edge &edge);
}
#endif //CODES_YCHE_INPUT_OUTPUT_HANDLER_H
