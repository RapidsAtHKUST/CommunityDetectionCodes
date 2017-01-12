#ifndef _GRAPH_HPP_
#define _GRAPH_HPP_

#include <utility>
#include <algorithm>
#include <vector>
#include <cassert>
#include <stdint.h>
#include <iterator>

namespace graph {

class VerySimpleGraphInterface;

class VerySimpleGraphInterface { // consecutive ints. No attributes, or directions, or anything
public:
	virtual int numNodes() const = 0;
	virtual int numRels() const = 0;
	virtual const std :: pair<int32_t, int32_t> & EndPoints(int relId) const = 0;
	virtual ~ VerySimpleGraphInterface() {}
	virtual const std :: vector<int32_t> & neighbouring_rels_in_order(const int32_t node_id) const = 0;
	virtual const std :: vector<int32_t> & neighbouring_nodes_in_order(const int32_t node_id) const = 0;
	virtual int32_t degree(const int32_t node_id) const { return int32_t(this->neighbouring_rels_in_order(node_id).size()); }
	virtual bool are_connected(int32_t node_id_1, int32_t node_id_2) const { // TODO: could be faster, making use of the fact that various structures are sorted (binary_search)
		if(this->degree(node_id_2) < this->degree(node_id_1))
			std :: swap(node_id_2, node_id_1);
		// node 1 is the low-degree node. Check through it's neighbours for node 2
		const std :: vector<int32_t> & neigh_nodes = this->neighbouring_nodes_in_order(node_id_1);
		return std :: binary_search(neigh_nodes.begin(), neigh_nodes.end(), node_id_2);
	}
};

class neighbouring_rel_id_iterator : public std :: iterator< std :: forward_iterator_tag, const int32_t> { // given a node_id, we can iterate over its neighbours and get the node_id of them
private:
	std :: vector<int32_t> :: const_iterator i; // This will point at the current relationship
	std :: vector<int32_t> :: const_iterator i_end; // the final relationship of this node
public:
	neighbouring_rel_id_iterator(const VerySimpleGraphInterface * vsg, const int32_t node_id) {
		assert(vsg);
		this -> i = vsg->neighbouring_rels_in_order(node_id).begin();
		this -> i_end = vsg->neighbouring_rels_in_order(node_id).end();
	}
	neighbouring_rel_id_iterator & operator++() {
#ifndef NDEBUG
		value_type old_value = this -> operator* ();
#endif
		this->i ++;
#ifndef NDEBUG
		if( ! this->at_end()) {
			value_type new_value = this -> operator* ();
			assert(new_value > old_value);
		}
#endif
		return *this;
	}
	neighbouring_rel_id_iterator   operator++(int) { // postfix. Should return the old iterator, but I'm going to return void :-)
		neighbouring_rel_id_iterator old_value(*this);
		this -> operator++();
		return old_value;
	}
	bool at_end() const {
		return i == i_end;
	}
	int32_t operator*() {
		assert( ! this->at_end() );
		return * this->i;
	}
};

class neighbouring_node_id_iterator : public std :: iterator< std :: forward_iterator_tag, const int32_t> { // given a node_id, we can iterate over its neighbours and get the node_id of them
private:
	const VerySimpleGraphInterface * const vsg;
	const int32_t node_id;
	std :: vector<int32_t> :: const_iterator i; // This will point at the current relationship
	std :: vector<int32_t> :: const_iterator i_end; // the final relationship of this node
public:
	neighbouring_node_id_iterator() : vsg(NULL), node_id(0) { // constructor used to define the magic 'end-marker' iterator. Don't try to use it for real
	}
	neighbouring_node_id_iterator(const VerySimpleGraphInterface * _vsg, const int32_t _node_id) : vsg(_vsg), node_id(_node_id) {
		assert(vsg);
		this -> i = this->vsg->neighbouring_rels_in_order(node_id).begin();
		this -> i_end = this->vsg->neighbouring_rels_in_order(node_id).end();
	}
	neighbouring_node_id_iterator & operator++() {
		assert(vsg);
#ifndef NDEBUG
		value_type old_value = this -> operator* ();
#endif
		this->i ++;
#ifndef NDEBUG
		if( ! this->at_end()) {
			value_type new_value = this -> operator* ();
			assert(new_value > old_value);
		}
#endif
		return *this;
	}
	neighbouring_node_id_iterator   operator++(int) { // postfix. Should return the old iterator, but I'm going to return void :-)
		assert(vsg);
		neighbouring_node_id_iterator old_value(*this);
		this -> operator++();
		return old_value;
	}
	bool at_end() const {
		assert(vsg);
		return i == i_end;
	}
	int32_t operator*() {
		assert(vsg);
		assert( ! this->at_end() );
		const int32_t rel_id = *this->i;
		const std :: pair <int32_t, int32_t> eps = this->vsg->EndPoints(rel_id);
		if(eps.first == this->node_id) {
			return eps.second;
		} else {
			assert(eps.second == this->node_id);
			return eps.first;
		}
	}
	neighbouring_node_id_iterator end_marker() const;
	bool operator == (const neighbouring_node_id_iterator &r) const {
		if(this->vsg == NULL && r.vsg == NULL)
			return true;
		if(this->vsg == NULL)
			return r.at_end();
		if(r.vsg == NULL)
			return this->at_end();
		assert(this->vsg);
		assert(r.vsg);
		assert(this->vsg == r.vsg);
		assert(this->node_id == r.node_id);
		assert(this->i_end == r.i_end);

		return this->i == r.i;
	}
	bool operator != (const neighbouring_node_id_iterator &r) const {
		return ! this->operator== (r);
	}
};

} // namespace graph
#endif
