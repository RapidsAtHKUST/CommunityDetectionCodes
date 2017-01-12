#include <set>
#include <boost/unordered_map.hpp>
#include "graph_representation.hpp"

namespace grouping {

class Group;
class Grouping;

struct orderGroup {
	int operator() (const Group *l, const Group *rl) const;
};

class Grouping {
public:
	long double _p_in;
	long double _p_out;
	const GraphWithoutNames &_g;
	const std::set<Group*, orderGroup> &groups;
	map<int, V > global_edge_counts;

	explicit Grouping(const GraphWithoutNames &g, long double p_in, long double p_out);

	Group* newG(bool randomized_p_in);
	Group* newG ();
	void isolate(V v);
	void addV(Group *, V v);
	void delV(Group *, V v); // warning. This will delete the group if v is the last node in it.
	const set<Group*> & vgroups(V v) const;
	size_t vgroups_size() const;
	void FixGlobalEdgeCounts(const V* edgeVN_ptr, bool AddNotDel);

private:
	struct edgeHash {
		long operator() (const pair<V,V> &edge) const {
			return boost::hash<V>()(edge.first) + boost::hash<V>()(edge.second);
		}
	};
	std::set<Group*, orderGroup> _groups;
	vector<set<Group*> > _vgroups; // TODO: Make this a multimap?

public:
	int comm_count_per_edge(V r, const V *edgeLR_ptr) const;
	struct EdgeCountTriple { // TODO: make this private
		V shared_comms;
		V other_index;
		EdgeCountTriple() : shared_comms(0), other_index(0) {}
	};
	const vector<EdgeCountTriple> &comm_count_per_edge2;
	int64 _sigma_shared; // summed across all pairs of nodes, even the disconnected ones
	long double value_of_objective_with_no_communities;
private:
	vector<EdgeCountTriple>_comm_count_per_edge2;
	V nodes_in_at_least_one_comm;

	friend void deleteEmptyGroup(Grouping *ging, Group *grp);
};
class Group {
public: 
	const set<V> &vs; // read-only public member . But we mustn't ever copy construct - I should avoid this idea, unless I am sure I've got the copy constructor(s) right.
	const int _id;
	int _randomized_p_in; // how was this group created. Was it still when I was using random p_ins?
	static int arbitraryID;
private:
	set<V> _vs; // vertices in this group
	explicit Group();
	~Group();
	friend class Grouping;
	friend void deleteEmptyGroup(Grouping *ging, Group *grp);
};


} // namespace grouping
