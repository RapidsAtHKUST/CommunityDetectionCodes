#include "graph_representation.hpp"

namespace iterative {

void planted(const SimpleIntGraph &g);
void planted(const SimpleStringGraph &g);


// I'm exporting these in order for overlapping.cpp to be able to use the planted partition to seed itself.

struct OneNode {
	V c; // Community
	V next; // next node in this same community.
	// NOTE: Ensure you swap all the following entries in make_singleton. I should probably make this a struct
	V order; // number of nodes in this community (only defined for the head-node)
	V sigma_d;
	int m_c;
	long double internalEntropy; // this is for the lambda/(n_c-1) model
};
struct PartitionStats {
	const int m;
	const int64 N;
	int m_i; // number of those edges that are within a community at the moment
	int64 a; // count of all pairs (connected or not connected) within each community.
	int64 sigmaD2; // sum of squares of total degree (useful for modularity)
	long double entropy; // number of bits needed to encode the partition.
	int number_of_communities;
	long double sumOfInternalEntropy;
	const long double mu; // I'll fix this to the average degree at first, but I suppose we should modify it

	PartitionStats(const GraphWithoutNames &g_);
};
struct Partition : public PartitionStats {
	const GraphWithoutNames &g;
	vector<OneNode> p;


	explicit Partition(const GraphWithoutNames &g_);
	bool is_singleton(V v) const;
	int make_singleton(V v);
	void move_singleton_to_comm(V v, int best_c);
	void move_singleton_to_comm(V v, int best_c, int delta_m_i);
	void internalCheck() const; // assertions to validate that the Partitions internal structure is sound.
	void fixEntropy(); // assertions to validate that the Partitions internal structure is sound.
};

void findPartition(Partition &p, const SimpleStringGraph g);
void findPartition(Partition &p, const SimpleIntGraph g);
template <class Name> void savePartition(const bloomGraph<Name> &g, const Partition &p, int iteration);

} // namespace iterative 
