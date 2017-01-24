#include "weights.hpp"
#include "../pp.hpp"
#include<stdint.h>

using namespace std;

namespace graph {
namespace weights {

	template <class W>
	EdgeDetails<W> :: ~EdgeDetails() {
	}
	EdgeDetailsInterface :: ~EdgeDetailsInterface() {
	}
	template <class W>
	int32_t EdgeDetails<W> :: size() const {
		return int32_t(this->dw.size());
	}
	template <class W>
	void EdgeDetails<W> :: new_rel(const int relId, std :: pair<int,int> nodeIds, std :: string &weight) {
		if(relId >= (int)this->dw.size()) // no guarantee we'll be notified of the rels in order
			this->dw.resize(relId+1);
		assert(relId <  (int)this->dw.size());
		this->dw.at(relId).inform(nodeIds.first > nodeIds.second, weight);
		// this->dw.back().inform(nodeIds.first > nodeIds.second, weight);
	}
	template <class W>
	long double EdgeDetails<W> :: getl2h(const int relId) const { // this returns the value in the undirected case, and it handles self loops
		return this->dw.at(relId).getl2h();
	}
	template <class W>
	long double EdgeDetails<W> :: geth2l(const int relId) const { // this is only relevant in directed graphs.
		return this->dw.at(relId).geth2l();
	}

// template  // <> here would forward-declare template (or something like that). We need to leave out the <> in order to ensure it's actually created
template long double EdgeDetails<NoDetails>              :: geth2l(const int relId) const;
template long double EdgeDetails<DirectedLDoubleWeights> :: geth2l(const int relId) const;
template long double EdgeDetails<DirectedNoWeights>      :: geth2l(const int relId) const;
template long double EdgeDetails<WeightNoDir>            :: geth2l(const int relId) const;
template long double EdgeDetails<NoDetails>              :: getl2h(const int relId) const;
template long double EdgeDetails<DirectedLDoubleWeights> :: getl2h(const int relId) const;
template long double EdgeDetails<DirectedNoWeights>      :: getl2h(const int relId) const;
template long double EdgeDetails<WeightNoDir>            :: getl2h(const int relId) const;
template void        EdgeDetails<NoDetails>              :: new_rel(int relId, std :: pair<int,int> nodeIds, std :: string &weight);
template void        EdgeDetails<DirectedLDoubleWeights> :: new_rel(int relId, std :: pair<int,int> nodeIds, std :: string &weight);
template void        EdgeDetails<DirectedNoWeights>      :: new_rel(int relId, std :: pair<int,int> nodeIds, std :: string &weight);
template void        EdgeDetails<WeightNoDir>            :: new_rel(int relId, std :: pair<int,int> nodeIds, std :: string &weight);
template int         EdgeDetails<NoDetails>              :: size()  const;
template int         EdgeDetails<DirectedLDoubleWeights> :: size()  const;
template int         EdgeDetails<DirectedNoWeights>      :: size()  const;
template int         EdgeDetails<WeightNoDir>            :: size()  const;
template             EdgeDetails<NoDetails>              :: ~EdgeDetails();
template             EdgeDetails<DirectedLDoubleWeights> :: ~EdgeDetails();
template             EdgeDetails<DirectedNoWeights>      :: ~EdgeDetails();
template             EdgeDetails<WeightNoDir>            :: ~EdgeDetails();
} // namespace weights
} // namespace graph

