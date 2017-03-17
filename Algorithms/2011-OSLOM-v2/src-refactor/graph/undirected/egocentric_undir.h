#include <deque>

#include <collection/module_collection.h>
#include <graph/louvain_oslomnet.h>

using namespace std;

class egocentric_net : public oslomnet_louvain {
public:
    egocentric_net() : oslomnet_louvain() {};

    ~egocentric_net() {};

    int collect_ego_groups_once(deque<deque<int> > &);

private:
    int add_this_egomodules(int node, module_collection &Mego);
};
