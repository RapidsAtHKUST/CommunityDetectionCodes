#include "cliques.hpp"
#include <set>
#include <map>
#include <list>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <sys/stat.h>
#include "pp.hpp"

using namespace std;


namespace cliques {

    typedef int32_t V;

//typedef list<V> list_of_ints;
    struct list_of_ints : private list<int32_t> {
        size_t sz;
        typedef list<int32_t>::const_iterator const_iterator;
        typedef list<int32_t>::iterator iterator;
        typedef list<int32_t>::const_reference const_reference;

        explicit list_of_ints() : list<int32_t>(), sz(0) {
        }

        const list <int32_t> &get() const {
            return *this;
        }

        void push_back(int32_t v) {
            ++sz;
            this->list<int32_t>::push_back(v);
        }

        size_t size() const {
            // assert (sz == this -> list<int32_t> :: size()) ;
            return sz;
        }

        bool empty() const {
            return this->list<int32_t>::empty();
        }

        iterator begin() {
            return this->list<int32_t>::begin();
        }

        iterator end() {
            return this->list<int32_t>::end();
        }

        iterator erase(iterator i) {
            --sz;
            return this->list<int32_t>::erase(i);
        }

        void insert(iterator i, int32_t v) {
            ++sz;
            this->list<int32_t>::insert(i, v);
        }

    private:
        list_of_ints(
                const list <int32_t> &) {} // this should never happen. It'd be a problem as this->sz would then be wrong, I think.

    };

    typedef set<V> not_type;

    struct CliqueReceiver;

    static void cliquesWorker(const SimpleIntGraph &g, CliqueReceiver *send_cliques_here, unsigned int minimumSize,
                              vector<V> &Compsub, list_of_ints Not, list_of_ints Candidates);

    static void findCliques(const SimpleIntGraph &g, CliqueReceiver *cliquesOut, unsigned int minimumSize);

    static void cliquesForOneNode(const SimpleIntGraph &g, CliqueReceiver *send_cliques_here, int minimumSize, V v);

    static void
    find_node_with_fewest_discs(int &fewestDisc, int &fewestDiscVertex, bool &fewestIsInCands, const list_of_ints &Not,
                                const list_of_ints &Candidates, const SimpleIntGraph &g);

    static const bool verbose = false;

/*
 * Candidates is always sorted
 * Compsub isn't sorted, but it's a vector and doens't need to be looked up anyway.
 * Not ?
 */

    struct CliqueReceiver {
        virtual void operator()(const std::vector<V> &clique) = 0;

        virtual ~CliqueReceiver() {}
    };

    static void cliquesForOneNode(const SimpleIntGraph &g, CliqueReceiver *send_cliques_here, int minimumSize, V v) {
        const int d = g->degree(v);
        if (d + 1 < minimumSize)
            return; // Obviously no chance of a clique if the degree is too small.


        vector<V> Compsub;
        list_of_ints Not, Candidates;
        Compsub.push_back(v);


        // copy those below the split into Not
        // copy those above the split into Candidates
        // there shouldn't ever be a neighbour equal to the split, this'd mean a self-loop
        {
            const vector<int32_t> &neighs_of_v = g->neighbouring_nodes_in_order(v);
            int32_t last_neighbour_id = -1;
            for (vector<int32_t>::const_iterator i = neighs_of_v.begin(); i != neighs_of_v.end(); i++) {
                const int neighbour_id = *i;

                if (neighbour_id < v)
                    Not.push_back(neighbour_id);
                if (neighbour_id > v)
                    Candidates.push_back(neighbour_id);

                assert(last_neighbour_id < neighbour_id);
                last_neighbour_id = neighbour_id;
            }
        }

        assert(d == int(Not.size() + Candidates.size()));

        cliquesWorker(g, send_cliques_here, minimumSize, Compsub, Not, Candidates);
    }

    static inline void
    tryCandidate(const SimpleIntGraph &g, CliqueReceiver *send_cliques_here, unsigned int minimumSize,
                 vector<V> &Compsub, const list_of_ints &Not, const list_of_ints &Candidates, const V selected) {
        // it *might* be the case that the 'selected' node is still in Candidates, but we can rely on the intersection to remove it (assuming no self loops! )
        assert(!Compsub.empty());
        Compsub.push_back(
                selected); // Compsub does *not* have to be ordered. I might try to enforce that in future though.

        list_of_ints CandidatesNew_;
        list_of_ints NotNew_;

        const vector<int32_t> &neighs_of_selected = g->neighbouring_nodes_in_order(selected);
        set_intersection(Candidates.get().begin(), Candidates.get().end(), neighs_of_selected.begin(),
                         neighs_of_selected.end(), back_inserter(CandidatesNew_));
        set_intersection(Not.get().begin(), Not.get().end(), neighs_of_selected.begin(), neighs_of_selected.end(),
                         back_inserter(NotNew_));

        cliquesWorker(g, send_cliques_here, minimumSize, Compsub, NotNew_, CandidatesNew_);

        Compsub.pop_back(); // we must restore Compsub, it was passed by reference
    }

    static void cliquesWorker(const SimpleIntGraph &g, CliqueReceiver *send_cliques_here, unsigned int minimumSize,
                              vector<V> &Compsub, list_of_ints Not, list_of_ints Candidates) {
        assert(g != NULL);
        // p2p         511462                   (10)
        // authors000                  (250)    (<4)
        // authors010  212489     5.3s (4.013)


        unless(Candidates.size() + Compsub.size() >= minimumSize) return;

        if (Candidates.empty()) { // No more cliques to be found. This is the (local) maximal clique.
            if (Not.empty() && Compsub.size() >= minimumSize)
                send_cliques_here->operator()(Compsub);
            return;
        }

        assert(!Candidates.empty());


        /*
         * version 2. Count disconnections-to-Candidates
         */

        // We know Candidates is not empty. Must find the element, in Not or in Candidates, that is most connected to the (other) Candidates
        int32_t fewestDisc = numeric_limits<int32_t>::max();
        V fewestDiscVertex = -1;
        bool fewestIsInCands = false;
        find_node_with_fewest_discs(fewestDisc, fewestDiscVertex, fewestIsInCands, Not, Candidates, g);
        if (!fewestIsInCands && fewestDisc == 0)
            return; // something in Not is connected to everything in Cands. Just give up now!
        {
            // list_of_ints CandidatesCopy(Candidates);
            for (list_of_ints::iterator i = Candidates.begin(); i != Candidates.end();) {
                V v = *i;
                unless(Candidates.size() + Compsub.size() >= minimumSize) return;
                if (
                        fewestDisc > 0 // speed trick. if it's zero, the call to are_connected is redundant
                        && v != fewestDiscVertex // deal with it later - see { if(fewestIsInCands) ... } below
                        && !g->are_connected(v, fewestDiscVertex)
                        ) { // just in case fewestDiscVertex is in Cands
                    unless(Candidates.size() + Compsub.size() >= minimumSize) return;
                    i = Candidates.erase(i);
                    tryCandidate(g, send_cliques_here, minimumSize, Compsub, Not, Candidates, v);
                    list_of_ints::iterator insertHere = lower_bound(Not.begin(), Not.end(), v);
                    Not.insert(insertHere, v); // we MUST keep the list Not in order
                    --fewestDisc;
                } else
                    ++i;
            }
        }
        // assert(fewestDisc == 0);
        if (fewestIsInCands) { // The most disconnected node was in the Cands.
            unless(Candidates.size() + Compsub.size() >= minimumSize) return;
            // Allow fewestDiscVertex to slip through. Candidates.erase(lower_bound(Candidates.begin(),Candidates.end(),fewestDiscVertex));
            tryCandidate(g, send_cliques_here, minimumSize, Compsub, Not, Candidates, fewestDiscVertex);
            // No need as we're about to return...  Not.insert(lower_bound(Not.begin(), Not.end(), fewestDiscVertex) ,fewestDiscVertex); // we MUST keep the list Not in order

            // Note: fewestDiscVertex is still in Candidates, but it's OK because tryCandidate can handle it.
        }
        /*
         * At this stage:
         *   - fewestDiscVertex is in Not
         *   - all the Candidates that are *not* connected to fewestDiscVertex are in Not (let's "pretend" there's a self loop on fewestDiscVertex, if that helps you)
         *   - hence the remaining Candidates are all connected to fewestDiscVertex, and hence can't be in a maximal clique
         *   - so we can return now, even though Candidates is not empty.
         */
    }

    struct CliquesToStdout : public CliqueReceiver {
        int n;
        std::map<size_t, int32_t> cliqueFrequencies;
        const graph::NetworkInterfaceConvertedToString *g;

        CliquesToStdout(const graph::NetworkInterfaceConvertedToString *_g) : n(0), g(_g) {}

        virtual void operator()(const vector<V> &Compsub) {
            vector<V> Compsub_ordered(Compsub);
            sort(Compsub_ordered.begin(), Compsub_ordered.end());
            bool firstField = true;
            if (Compsub_ordered.size() >= 3) {
                ++this->cliqueFrequencies[Compsub_ordered.size()];
                for (vector<V>::const_iterator v = Compsub_ordered.begin(); v != Compsub_ordered.end(); ++v) {
                    if (!firstField)
                        std::cout << ' ';
                    std::cout << g->node_name_as_string(*v);
                    firstField = false;
                }
                std::cout << endl;
                this->n++;
            }
        }
    };

    struct SelfLoopsNotSupportedException {
    };

    static void findCliques(const SimpleIntGraph &g, CliqueReceiver *send_cliques_here, unsigned int minimumSize) {
        unless(minimumSize >= 3) throw std::invalid_argument("the minimumSize for findCliques() must be at least 3");

        for (int32_t r = 0; r < g->numRels(); r++) {
            const pair<int32_t, int32_t> &eps = g->EndPoints(r);
            unless(eps.first < eps.second) // no selfloops allowed
                throw SelfLoopsNotSupportedException();
        }

        for (V v = 0; v < (V) g->numNodes(); v++) {
            if (v && v % 100 == 0)
                cerr << "processing node: " << v << " ..." << endl;
            cliquesForOneNode(g, send_cliques_here, minimumSize, v);
        }
    }

    void cliquesToStdout(const graph::NetworkInterfaceConvertedToString *net, unsigned int minimumSize /* = 3*/ ) {

        CliquesToStdout send_cliques_here(net);
        findCliques(net->get_plain_graph(), &send_cliques_here, minimumSize);
        cerr << send_cliques_here.n << " cliques found" << endl;
        if (send_cliques_here.n > 0) {
            assert(!send_cliques_here.cliqueFrequencies.empty());
            const size_t biggest_clique_found = send_cliques_here.cliqueFrequencies.rbegin()->first;
            for (size_t i = minimumSize; i <= biggest_clique_found; i++) {
                cerr << send_cliques_here.cliqueFrequencies[i] << "\t#" << i << endl;
            }
        }

    }

    static int32_t count_disconnections(const set<int> &cands, const int32_t v, const SimpleIntGraph &g) {
        const vector<int> &v_neighs = g->neighbouring_nodes_in_order(v);
        vector<int32_t> intersection;
        set_intersection(cands.begin(), cands.end(), v_neighs.begin(), v_neighs.end(), back_inserter(intersection)
        );
        const int32_t num_connections = int32_t(intersection.size());

        /*
        int currentDiscs = 0;
        for( list_of_ints :: const_iterator i = Candidates.get().begin(); i != Candidates.get().end(); i++) {
            V v2 = *i;
            if(!g->are_connected(v, v2)) {
                ++currentDiscs;
            }
        }
        PP3( currentDiscs , num_connections , cands.size() );
        assert( currentDiscs + num_connections == int(cands.size()) );
        return currentDiscs;
        */
        return int32_t(cands.size() - num_connections);


    }

    static void
    find_node_with_fewest_discs(int &fewestDisc, int &fewestDiscVertex, bool &fewestIsInCands, const list_of_ints &Not,
                                const list_of_ints &Candidates, const SimpleIntGraph &g) {
        set<int32_t> cands(Candidates.get().begin(), Candidates.get().end());
        assert(!Candidates.empty());
        // TODO: Make use of degree, or something like that, to speed up this counting of disconnects?
        const list_of_ints::const_iterator not_end = Not.get().end();
        for (list_of_ints::const_iterator i = Not.get().begin(); i != not_end; i++) {
            V v = *i;
            const int currentDiscs = count_disconnections(cands, v, g);
            if (currentDiscs < fewestDisc) {
                fewestDisc = currentDiscs;
                fewestDiscVertex = v;
                fewestIsInCands = false;
                if (!fewestIsInCands && fewestDisc == 0)
                    return; // something in Not is connected to everything in Cands. Just give up now!
            }
        }
        const list_of_ints::const_iterator cands_end = Candidates.get().end();
        for (list_of_ints::const_iterator i = Candidates.get().begin(); i != cands_end; i++) {
            V v = *i;
            const int currentDiscs = count_disconnections(cands, v, g);
            if (currentDiscs < fewestDisc) {
                fewestDisc = currentDiscs;
                fewestDiscVertex = v;
                fewestIsInCands = true;
                if (!fewestIsInCands && fewestDisc == 0)
                    return; // something in Not is connected to everything in Cands. Just give up now!
            }
        }
        assert(fewestDisc <= int(Candidates.size()));
        assert(fewestDiscVertex >= 0);
    }

    typedef vector<vector<int64_t> > all_cliques_by_orig_name_t;

    struct StoreCliquesInVector : public CliqueReceiver {
        all_cliques_by_orig_name_t &all_cliques_by_orig_name;
        const graph::NetworkInterfaceConvertedToString *net;

        StoreCliquesInVector(all_cliques_by_orig_name_t &x, graph::NetworkInterfaceConvertedToString *net_)
                : all_cliques_by_orig_name(x), net(net_) {}

        virtual void operator()(const std::vector<V> &clique) {
            vector<int64_t> clique_nodes_using_the_original_name;
            for (std::vector<V>::const_iterator node = clique.begin(); node != clique.end(); ++node) {
                const string as_string = this->net->node_name_as_string(*node);
                int64_t original_name;
                istringstream iss(as_string);
                iss >> original_name;
                clique_nodes_using_the_original_name.push_back(original_name);
            }
            this->all_cliques_by_orig_name.push_back(clique_nodes_using_the_original_name);
        }
    };

// cliquesToStdout
    void
    cliquesToVector(vector<vector<int64_t> > &all_cliques_by_orig_name, graph::NetworkInterfaceConvertedToString *net,
                    unsigned int minimumSize /* = 3*/ ) {
        StoreCliquesInVector send_cliques_here(all_cliques_by_orig_name, net);
        findCliques(net->get_plain_graph(), &send_cliques_here, minimumSize);
    }

} // namespace cliques
