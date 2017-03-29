//
// Created by cheyulin on 12/20/16.
//

#include "cis_sequential_algorithm.h"

namespace yche {
    Cis::Cis(unique_ptr<Cis::Graph> graph_ptr, double lambda) : lambda_(lambda), graph_ptr_(std::move(graph_ptr)) {
        for (auto vp = vertices(*graph_ptr_); vp.first != vp.second; ++vp.first) { vertices_.emplace_back(*vp.first); }
    }

    double Cis::CalDensity(int size, double w_in, double w_out, double lambda) {
        if (size < 1) {
            return numeric_limits<double>::min();
        } else {
            double partA = ((1 - lambda) * (w_in / (w_in + w_out)));
            double partB = (lambda * ((2 * w_in) / (size * (size - 1))));
            if (size == 1)
                partB = lambda;
            return partA + partB;
        }
    }

    double Cis::CalDensity(const Community &community) const {
        return CalDensity(static_cast<int>(community.member_indices_.size()),
                          community.w_in_, community.w_out_, lambda_);
    }

    double Cis::CalDensity(const Community &community, const Entity &member, MutationType mutation_type) const {
        if (mutation_type == MutationType::add_neighbor) {
            return CalDensity(static_cast<int>(community.member_indices_.size() + 1),
                              community.w_in_ + member.w_in_, community.w_out_ + member.w_out_, lambda_);

        } else {
            return CalDensity(static_cast<int>(community.member_indices_.size() - 1),
                              community.w_in_ - member.w_in_, community.w_out_ - member.w_out_, lambda_);
        }
    }

    void Cis::InitializeSeeds(const EntityIdxSet &seed, Community &community,
                              EntityDict &member_dict, EntityDict &neighbor_dict,
                              property_map<Graph, vertex_index_t>::type &vertex_index_map,
                              property_map<Graph, edge_weight_t>::type &edge_weight_map) const {
        auto neighbor_indices = EntityIdxSet();

        for (auto &seed_vertex_index :seed) {
            community.member_indices_.emplace(seed_vertex_index);

            auto member = Entity(seed_vertex_index);
            auto seed_vertex = vertices_[seed_vertex_index];
            for (auto vp = adjacent_vertices(seed_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
                auto adjacent_vertex_index = static_cast<int>(vertex_index_map[*vp.first]);
                auto edge_weight = edge_weight_map[edge(seed_vertex, vertices_[adjacent_vertex_index],
                                                        *graph_ptr_).first];
                if (seed.find(adjacent_vertex_index) != seed.end()) {
                    member.w_in_ += edge_weight;
                    community.w_in_ += edge_weight;
                } else {
                    member.w_out_ += edge_weight;
                    community.w_out_ += edge_weight;
                    neighbor_indices.emplace(adjacent_vertex_index);
                }
            }
            member_dict.emplace(member.entity_index_, member);
        }

        for (auto &neighbor_vertex_index :neighbor_indices) {
            auto neighbor = Entity(neighbor_vertex_index);
            Vertex neighbor_vertex = vertices_[neighbor_vertex_index];
            for (auto vp = adjacent_vertices(neighbor_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
                auto adjacent_vertex_index = static_cast<int>(vertex_index_map[*vp.first]);
                auto edge_weight = edge_weight_map[edge(neighbor_vertex, vertices_[adjacent_vertex_index],
                                                        *graph_ptr_).first];
                if (seed.find(adjacent_vertex_index) != seed.end()) {
                    neighbor.w_in_ += edge_weight;
                } else {
                    neighbor.w_out_ += edge_weight;
                }
            }
            neighbor_dict.emplace(neighbor.entity_index_, neighbor);
        }
    }

    void Cis::UpdateForAddNeighbor(const Cis::Vertex &mutate_vertex, Community &community,
                                   EntityDict &member_dict, EntityDict &neighbor_dict,
                                   property_map<Graph, vertex_index_t>::type &vertex_index_map,
                                   property_map<Graph, edge_weight_t>::type &edge_weight_map) const {
        //Update Member and Neighbor List
        for (auto vp = adjacent_vertices(mutate_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
            auto check_neighbor_vertex = *vp.first;
            auto check_neighbor_vertex_index = static_cast<int>(vertex_index_map[check_neighbor_vertex]);
            auto check_neighbor = Entity(check_neighbor_vertex_index);
            auto edge_weight = edge_weight_map[edge(mutate_vertex, check_neighbor_vertex, *graph_ptr_).first];

            auto iter = member_dict.find(check_neighbor.entity_index_);
            if (iter != member_dict.end() ||
                (iter = neighbor_dict.find(check_neighbor.entity_index_)) != neighbor_dict.end()) {
                //Update Info In Members and Neighbors
                iter->second.w_in_ += edge_weight;
                iter->second.w_out_ -= edge_weight;
            } else {
                //Add New Neighbor
                auto member = Entity(check_neighbor_vertex_index);
                for (auto vp_inner = adjacent_vertices(check_neighbor_vertex, *graph_ptr_);
                     vp_inner.first != vp_inner.second; ++vp_inner.first) {
                    auto neighbor_neighbor_vertex_index = static_cast<int>(vertex_index_map[*vp_inner.first]);
                    edge_weight = edge_weight_map[edge(check_neighbor_vertex,
                                                       vertices_[neighbor_neighbor_vertex_index], *graph_ptr_).first];
                    if (community.member_indices_.find(neighbor_neighbor_vertex_index) !=
                        community.member_indices_.end()) {
                        member.w_in_ += edge_weight;
                    } else {
                        member.w_out_ += edge_weight;
                    }
                }
                neighbor_dict.emplace(member.entity_index_, member);
            }
        }
    }

    void Cis::UpdateForRemoveMember(const Cis::Vertex &mutate_vertex, Community &community,
                                    EntityDict &member_dict, EntityDict &neighbor_dict,
                                    property_map<Graph, vertex_index_t>::type &vertex_index_map,
                                    property_map<Graph, edge_weight_t>::type &edge_weight_map) const {
        //Update Member and Neighbor List
        for (auto vp = adjacent_vertices(mutate_vertex, *graph_ptr_); vp.first != vp.second; ++vp.first) {
            auto check_neighbor_vertex = *vp.first;
            auto check_neighbor_vertex_index = static_cast<int>(vertex_index_map[check_neighbor_vertex]);
            auto check_neighbor_ptr = Entity(check_neighbor_vertex_index);
            auto edge_weight = edge_weight_map[edge(mutate_vertex, check_neighbor_vertex, *graph_ptr_).first];

            auto iter = member_dict.find(check_neighbor_ptr.entity_index_);
            if (iter != member_dict.end() ||
                (iter = neighbor_dict.find(check_neighbor_ptr.entity_index_)) != neighbor_dict.end()) {
                //Update Info In Members and Neighbors
                iter->second.w_in_ -= edge_weight;
                iter->second.w_out_ += edge_weight;
            }
        }
    }

    void Cis::MutateStates(MutationType mutation_type, Community &community, EntityDict &expand_entity_dict,
                           EntityDict &shrink_entity_dict, auto &&degree_cmp_obj, bool &change_flag,
                           property_map<Graph, vertex_index_t>::type &vertex_index_map,
                           property_map<Graph, edge_weight_t>::type &edge_weight_map) const {
        auto to_check_list = vector<Entity>();
        for (auto &neighbor_pair:shrink_entity_dict) { to_check_list.emplace_back(neighbor_pair.second); }
        sort(to_check_list.begin(), to_check_list.end(), degree_cmp_obj);
        for (auto &check_member:to_check_list) {
            if (CalDensity(community) < CalDensity(community, check_member, mutation_type)) {
                change_flag = true;
                community.UpdateInfoForMutation(check_member, mutation_type);
                shrink_entity_dict.erase(check_member.entity_index_);
                auto check_vertex = vertices_[check_member.entity_index_];
                expand_entity_dict.emplace(check_member.entity_index_, check_member);
                if (mutation_type == MutationType::add_neighbor) {
                    UpdateForAddNeighbor(check_vertex, community, expand_entity_dict, shrink_entity_dict,
                                         vertex_index_map, edge_weight_map);
                } else {
                    UpdateForRemoveMember(check_vertex, community, shrink_entity_dict, expand_entity_dict,
                                          vertex_index_map, edge_weight_map);
                }
            }
        }

    }

    Community Cis::FindConnectedComponent(EntityIdxSet &member_set, EntityIdxSet &mark_set, int first_mem_idx,
                                          property_map<Graph, vertex_index_t>::type &vertex_index_map,
                                          property_map<Graph, edge_weight_t>::type &edge_weight_map) const {
        auto community = Community();
        auto frontier = queue<int>();
        frontier.emplace(first_mem_idx);
        mark_set.emplace(first_mem_idx);
        while (frontier.size() > 0) {
            auto expand_vertex_index = frontier.front();
            auto expand_vertex = vertices_[expand_vertex_index];

            community.member_indices_.emplace(expand_vertex_index);
            for (auto vp = adjacent_vertices(vertices_[expand_vertex_index], *graph_ptr_);
                 vp.first != vp.second; ++vp.first) {
                auto neighbor_vertex = *vp.first;
                auto adjacency_vertex_index = static_cast<int>(vertex_index_map[neighbor_vertex]);
                auto edge_flag_pair = boost::edge(expand_vertex, neighbor_vertex, *graph_ptr_);
                auto &my_edge = edge_flag_pair.first;

                auto iter = member_set.find(adjacency_vertex_index);
                if (mark_set.find(adjacency_vertex_index) == mark_set.end() && iter != member_set.end()) {
                    community.w_in_ += edge_weight_map[my_edge];
                    frontier.emplace(adjacency_vertex_index);
                    mark_set.emplace(adjacency_vertex_index);
                } else {
                    community.w_out_ = edge_weight_map[my_edge];
                }
            }
            member_set.erase(expand_vertex_index);
            frontier.pop();
        }
        return community;
    }

    Community Cis::SplitAndChoose(EntityIdxSet &member_set) const {
        auto community_vec = vector<Community>();
        auto mark_set = std::unordered_set<int>();
        auto vertex_index_map = boost::get(vertex_index, *graph_ptr_);
        auto edge_weight_map = boost::get(edge_weight, *graph_ptr_);

        while (member_set.size() > 0) {
            auto first_mem_idx = *member_set.begin();
            community_vec.emplace_back(
                    FindConnectedComponent(member_set, mark_set, first_mem_idx, vertex_index_map, edge_weight_map));
        }

        sort(community_vec.begin(), community_vec.end(),
             [this](auto &left_comm, auto &right_comm) {
                 double left_density = this->CalDensity(left_comm);
                 double right_density = this->CalDensity(right_comm);
                 if (left_density != right_density) {
                     return left_density > right_density;
                 } else {
                     return (left_comm.member_indices_).size() > (right_comm.member_indices_).size();
                 }
             });
        return community_vec[0];
    }

    EntityIdxVec Cis::ExpandSeed(EntityIdxSet &entity_idx_set) const {
        auto community = Community();
        auto member_dict = EntityDict();
        auto neighbor_dict = EntityDict();
        auto vertex_index_map = boost::get(vertex_index, *graph_ptr_);
        auto edge_weight_map = boost::get(edge_weight, *graph_ptr_);

        InitializeSeeds(entity_idx_set, community, member_dict, neighbor_dict, vertex_index_map, edge_weight_map);

        auto degree_cmp_obj = [this](auto &left_member, auto &right_member) {
            return degree(this->vertices_[left_member.entity_index_], *this->graph_ptr_) <
                   degree(this->vertices_[right_member.entity_index_], *this->graph_ptr_);
        };

        for (auto is_change = true; is_change;) {
            is_change = false;
            MutateStates(MutationType::add_neighbor, community, member_dict, neighbor_dict,
                         degree_cmp_obj, is_change, vertex_index_map, edge_weight_map);
            MutateStates(MutationType::remove_member, community, neighbor_dict, member_dict,
                         degree_cmp_obj, is_change, vertex_index_map, edge_weight_map);
            community = SplitAndChoose(community.member_indices_);

        }
        //For Later Merge Usage
        auto member_vec = EntityIdxVec();
        member_vec.reserve(community.member_indices_.size());
        for (auto entity_index:community.member_indices_) { member_vec.emplace_back(entity_index); }
        sort(member_vec.begin(), member_vec.end());
        return member_vec;
    }

    double Cis::GetIntersectRatio(const EntityIdxVec &left_community, const EntityIdxVec &right_community) {
        auto intersect_set = vector<int>(left_community.size() + right_community.size());
        auto iter_end = set_intersection(left_community.begin(), left_community.end(),
                                         right_community.begin(), right_community.end(), intersect_set.begin());
        auto intersect_set_size = iter_end - intersect_set.begin();
        auto rate = static_cast<double>(intersect_set_size) / min(left_community.size(), right_community.size());
        return rate;
    }

    EntityIdxVec Cis::GetUnion(const EntityIdxVec &left_community, const EntityIdxVec &right_community) {
        auto union_set = vector<int>(left_community.size() + right_community.size());
        auto iter_end = set_union(left_community.begin(), left_community.end(),
                                  right_community.begin(), right_community.end(), union_set.begin());
        union_set.resize(iter_end - union_set.begin());
        return union_set;
    }

    void Cis::MergeCommToGlobal(EntityIdxVec &result_community) {
        if (overlap_community_vec_.size() == 0) {
            overlap_community_vec_.emplace_back(std::move(result_community));
        } else {
            auto is_insert = true;
            for (auto &community:overlap_community_vec_) {
                if (GetIntersectRatio(community, result_community) > 1 - DOUBLE_ACCURACY) {
                    community = GetUnion(community, result_community);
                    is_insert = false;
                    break;
                }
            }
            if (is_insert) { overlap_community_vec_.emplace_back(std::move(result_community)); }
        }
    }

    Cis::CommunityVec Cis::ExecuteCis() {
        auto vertex_index_map = boost::get(vertex_index, *graph_ptr_);
        for (auto vp = vertices(*graph_ptr_); vp.first != vp.second; ++vp.first) {
            auto vertex = *vp.first;
            auto partial_comm_members = EntityIdxSet();
            partial_comm_members.emplace(vertex_index_map[vertex]);
            auto result_community = ExpandSeed(partial_comm_members);
            MergeCommToGlobal(result_community);
        }
        return overlap_community_vec_;
    }
}
