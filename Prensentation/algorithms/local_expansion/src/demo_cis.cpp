//
// Created by cheyulin on 10/5/16.
//

#include <chrono>

#include "cis_sequential_algorithm.h"
#include "util/pretty_print.h"
#include "util/graph_io_helper.h"
#include "util/basic_io_helper.h"

using yche::Cis;
using yche::Edge;
using Vertex=Cis::Vertex;
using Graph=Cis::Graph;
using namespace std;

unique_ptr<Graph> ConstructGraph(map<int, Vertex> &vertex_dict, map<int, int> &name_dict, vector<Edge> &edges_vec) {
    auto graph_ptr = make_unique<Graph>();
    auto edge_weight_map = boost::get(boost::edge_weight, *graph_ptr);
    for (auto &my_edge : edges_vec) {
        if (vertex_dict.find(my_edge.src_index_) == vertex_dict.end()) {
            Vertex vertex = boost::add_vertex(*graph_ptr);
            vertex_dict.emplace(my_edge.src_index_, vertex);
        }
        if (vertex_dict.find(my_edge.dst_index_) == vertex_dict.end()) {
            Vertex vertex = boost::add_vertex(*graph_ptr);
            vertex_dict.emplace(my_edge.dst_index_, vertex);
        }
        auto edge_flag_pair = add_edge(vertex_dict[my_edge.src_index_], vertex_dict[my_edge.dst_index_], *graph_ptr);
        if (edge_flag_pair.second) {
            edge_weight_map[edge_flag_pair.first] = my_edge.edge_weight_;
            cout << my_edge;
        }
    }

    auto vertex_index_map = boost::get(boost::vertex_index, *graph_ptr);
    for (auto iter = vertex_dict.begin(); iter != vertex_dict.end(); ++iter) {
        name_dict.emplace(vertex_index_map[iter->second], iter->first);
    }
    return graph_ptr;
}

int main(int argc, char *argv[]) {
    auto edges_vec = yche::ReadWeightedEdgeList(argv[1]);

    auto vertex_dict = map<int, Vertex>();
    auto name_dict = map<int, int>();
    auto cis = Cis(ConstructGraph(vertex_dict, name_dict, edges_vec), 0);

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    auto arr_2d = cis.ExecuteCis();
    auto end = high_resolution_clock::now();
    cout << "whole execution time:" << duration_cast<milliseconds>(end - start).count() << " ms" << endl;

    auto name_arr_2d = yche::Map2DArrWithDict(arr_2d, name_dict);

    cout << "idx result:" << arr_2d << endl;
    cout << "name result:" << name_arr_2d << endl;
    cout << "comm size:" << name_arr_2d.size() << endl;
    return 0;
}