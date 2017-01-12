#include "grouping.hpp"
#include <boost/unordered_set.hpp>
#include "Range.hpp"

namespace grouping {

Grouping::Grouping(const GraphWithoutNames &g, long double p_in, long double p_out)
	: _p_in(p_in), _p_out(p_out), _g(g), groups(_groups)
		, _vgroups(g.vcount())
		, comm_count_per_edge2(_comm_count_per_edge2)
		, _sigma_shared(0)
		, value_of_objective_with_no_communities(1) // This should never be positive. Hence 1 will signify that it's undefined
		, nodes_in_at_least_one_comm(0)
		 {
	global_edge_counts[0] = g.ecount();
	_comm_count_per_edge2.resize(2*g.ecount());
	V offset = 0;
	for(V v=0; v<g.vcount(); v++) {
		IteratorRange<const V*> ns(_g.neighbours(v));
		assert(_g.neighbours(0).first + offset == _g.neighbours(v).first);
		Foreach(V n, ns) {
			const V * n_pointer = lower_bound( _g.neighbours(v).first ,_g.neighbours(v).second ,n);
			assert( _g.neighbours(0).first + offset == n_pointer);
			assert(*n_pointer == n);
			if(n>v) {
				// we know the offset of v -> n is offset
				const V * other_way = lower_bound( _g.neighbours(n).first ,_g.neighbours(n).second ,v);
				V other_offset = other_way - _g.neighbours(0).first;
				assert(_comm_count_per_edge2[offset].other_index==0);
				assert(_comm_count_per_edge2[other_offset].other_index==0);
				_comm_count_per_edge2[offset      ].other_index = other_offset;
				_comm_count_per_edge2[other_offset].other_index = offset;
			}
			++offset;
		}
	}
	assert(offset == 2*g.ecount());
}
int Group::arbitraryID = 0;
Group::Group() :vs(_vs),_id(arbitraryID++) {
}

Group::~Group() {
	assert(vs.empty());
}
Group* Grouping::newG () { return this->newG(false); }
Group* Grouping::newG(bool randomized_p_in) {
		Group *g = new Group();
		g->_randomized_p_in = randomized_p_in;
		_groups.insert(g);
		return g;
}
void Grouping::isolate(V v) {
	set <Group *> grps = vgroups(v); // I'm copying it in here, as we'll be modifying the node's vgroup as we delete nodes.
	ForeachContainer (Group *grp, grps) {
		this->delV(grp, v);
	}
}

void Grouping::FixGlobalEdgeCounts(const V* edgeVN_ptr, bool AddNotDel) {
			const int shared_comms = this->_comm_count_per_edge2[edgeVN_ptr - this->_g.neighbours(0).first].shared_comms;
			const V edgeNV_offset = _comm_count_per_edge2[edgeVN_ptr - _g.neighbours(0).first].other_index;
			if (AddNotDel) {
				this->global_edge_counts[shared_comms]  --;
				this->global_edge_counts[shared_comms+1]++;
				_comm_count_per_edge2[edgeVN_ptr - _g.neighbours(0).first].shared_comms++;

				//assert(_g.neighbours(0).first[edgeNV_offset] == v);
				_comm_count_per_edge2[edgeNV_offset].shared_comms++;
			} else {
				this->global_edge_counts[shared_comms]  --;
				this->global_edge_counts[shared_comms-1]++;
				_comm_count_per_edge2[edgeVN_ptr - _g.neighbours(0).first].shared_comms--;

				//assert(_g.neighbours(0).first[edgeNV_offset] == v);
				_comm_count_per_edge2[edgeNV_offset].shared_comms--;
			}
			assert(_comm_count_per_edge2[edgeVN_ptr - _g.neighbours(0).first].shared_comms == _comm_count_per_edge2[edgeNV_offset].shared_comms);
}

void Grouping::addV(Group *grp, V v) {
	this->_sigma_shared += int64(grp->vs.size());

	const V* edgeVN_ptr = _g.neighbours(v).first;
	const V* last = _g.neighbours(v).second;

	boost::unordered_set<V> unset;
	unset.insert(grp->vs.begin(), grp->vs.end());

	for(;edgeVN_ptr != last; ++edgeVN_ptr)
	{
		if(unset.count(*edgeVN_ptr)) {
			FixGlobalEdgeCounts(edgeVN_ptr, true);
		}
	}
	bool was_inserted = grp->_vs.insert(v).second;
	assert(was_inserted); // ensure the node wasn't there previously

	if(_vgroups[v].empty())
		++nodes_in_at_least_one_comm;
	_vgroups[v].insert(grp);
}
void Grouping::delV(Group *grp, V v) {
	this->_sigma_shared -= int64(grp->vs.size()-1);

	const V* edgeVN_ptr = _g.neighbours(v).first;
	const V* last = _g.neighbours(v).second;

	boost::unordered_set<V> unset;
	unset.insert(grp->vs.begin(), grp->vs.end());

	for(;edgeVN_ptr != last; ++edgeVN_ptr)
	{
		if(unset.count(*edgeVN_ptr)) {
			FixGlobalEdgeCounts(edgeVN_ptr, false);
		}
	}
	set<Group *> &vgroup = _vgroups.at(v);
	vgroup.erase(grp);
	if(vgroup.size()==0) {
		--nodes_in_at_least_one_comm;
	}
	bool erased = grp->_vs.erase(v);
	assert(erased==1);
	if(grp->vs.size()==0)
		deleteEmptyGroup(this, grp);
}

void deleteEmptyGroup(Grouping *pging, Group *grp) { // TODO: This is only called in one place, and shouldn't be called from outside. Merge this into delV
	assert(grp->vs.size()==0); // pging->_sigma_shared -= int64(grp->vs.size()) * int64(grp->vs.size()-1);

	Grouping &ging = *pging;
	ForeachContainer(V v, grp->vs) {
		ging._vgroups.at(v).erase(grp);
		if(ging._vgroups.at(v).size()==0) {
			--ging.nodes_in_at_least_one_comm;
		}
	}
	grp->_vs.clear();
	ging._groups.erase(grp);
	delete grp;
}
const set<Group*> & Grouping::vgroups(V v) const {
	return _vgroups[v];
}
size_t Grouping::vgroups_size() const {
	return nodes_in_at_least_one_comm;
}
int Grouping::comm_count_per_edge(V r, const V *edgeLR_ptr) const { // TODO: No need for the first parameter here
	assert(*edgeLR_ptr == r);
	return _comm_count_per_edge2[edgeLR_ptr - _g.neighbours(0).first ].shared_comms;
}

int orderGroup::operator() (const Group *l, const Group *r) const {
		if( l->_id == r->_id)
			assert(l == r);
		return l->_id < r->_id;
}


} // namespace grouping
