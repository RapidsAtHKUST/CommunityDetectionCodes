//
// Created by cheyulin on 1/19/17.
//
#include "graph_io_helper.h"

namespace std {
    ostream &operator<<(ostream &my_output_stream, yche::Edge &edge) {
        cout << "src:" << edge.src_index_ << ",dst:" << edge.dst_index_ << ",weight:" << edge.edge_weight_ << endl;
        return my_output_stream;
    }
}

namespace yche {
    vector<Edge> ReadWeightedEdgeList(string file_name_str) {
        auto edges_vec = vector<Edge>();
        auto in_stream = ifstream(file_name_str);
        auto s = string();
        if (!in_stream) {
            cout << "Error opening " << string(file_name_str) << " for input" << endl;
            exit(-1);
        } else {
            while (getline(in_stream, s)) {
                boost::regex pat("#.*edge_weight.*");
                boost::smatch matches;
                cout << s << endl;
                if (boost::regex_match(s, matches, pat))
                    continue;

                auto first_vertex_name = -1;
                auto second_vertex_name = -1;
                auto edge_weight = 1.0;
                auto string_stream = stringstream(s);
                string_stream >> first_vertex_name >> second_vertex_name >> edge_weight;
                edges_vec.emplace_back(first_vertex_name, second_vertex_name, edge_weight);
            }
        }
        return edges_vec;
    }

    vector<pair<int, int>> ReadEdgeList(string file_name_str) {
        auto edges_vec = vector<pair<int, int>>();
        auto in_stream = ifstream(file_name_str);
        auto s = string();
        if (!in_stream) {
            cout << "Error opening " << string(file_name_str) << " for input" << endl;
            exit(-1);
        } else {
            while (getline(in_stream, s)) {
                boost::regex pat("#.*");
                boost::smatch matches;
                cout << s << endl;
                if (boost::regex_match(s, matches, pat))
                    continue;

                auto first_vertex_name = -1;
                auto second_vertex_name = -1;
                auto string_stream = stringstream(s);
                string_stream >> first_vertex_name >> second_vertex_name;
                edges_vec.emplace_back(first_vertex_name, second_vertex_name);
            }
        }
        return edges_vec;
    }
}
