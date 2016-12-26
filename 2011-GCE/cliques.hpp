#ifndef _CLIQUES_HPP_
#define _CLIQUES_HPP_

#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <cerrno>
#include <stdexcept>

#include "Range.hpp"
#include "graph_representation.hpp"

namespace cliques {

struct CliqueFunctionAdaptor {
	virtual void operator() (const vector<V> &clique) = 0;
	virtual ~CliqueFunctionAdaptor() {}
};

template <class T, bool OriginalVertexNames> void findCliques(const bloomGraph<int> &, T &cliquesOut, unsigned int minimumSize);
template <class T> void findCliquesJustIDs            (const bloomGraph<int> &g, T &cliquesOut, unsigned int minimumSize);
template <class T> void findCliquesOriginalVertexNames(const bloomGraph<int> &g, T &cliquesOut, unsigned int minimumSize);
void cliquesForOneNode(const SimpleIntGraph &g, CliqueFunctionAdaptor &cliquesOut, unsigned int minimumSize, V v);
void create_directory(const string& directory) throw();
void cliquesToDirectory          (const bloomGraph<int> &, const string &outputDirectory, unsigned int minimumSize); // You're not allowed to ask for the 2-cliques


template <class T, bool OriginalVertexNames> void findCliques(const SimpleIntGraph &g, T & cliquesOut, unsigned int minimumSize) {
	unless(minimumSize >= 3) throw std::invalid_argument("the minimumSize for findCliques() must be at least 3");

	struct CliqueFunctionAdaptor_ : CliqueFunctionAdaptor {
		T & callThis;
		const SimpleIntGraph &g;
		CliqueFunctionAdaptor_(T &c, const SimpleIntGraph &g_) : callThis(c), g(g_) {}
		void operator() (const vector<V> &clique) {
			vector<V> copy = clique;
			sort(copy.begin(), copy.end());
			if(OriginalVertexNames)
				ForeachContainer (V &v, copy) {
					v = g.name_of_one_node(v);
				}
			callThis(copy);
		}
	};
	CliqueFunctionAdaptor_ cfa(cliquesOut, g);

	for(V v = 0; v < (V) g.vcount(); v++) {
		cliquesForOneNode(g, cfa, minimumSize, v);
	}
}

template <class T> void findCliquesJustIDs            (const bloomGraph<int> &g, T &cliquesOut, unsigned int minimumSize) { findCliques<T, false>(g, cliquesOut, minimumSize); }
template <class T> void findCliquesOriginalVertexNames(const bloomGraph<int> &g, T &cliquesOut, unsigned int minimumSize) { findCliques<T, true>(g, cliquesOut, minimumSize); }

struct CliqueSink { // Dump the cliques to a file in the CFinder format
	const bloomGraph<int> &g;
	int n;
	ofstream out;
	CliqueSink(const bloomGraph<int> &_g, const string& fileName) : g(_g), n(0), out(fileName.c_str()) { }
	void operator () (const vector<V> & Compsub) {
		if(Compsub.size() >= 3) {
			out << n << ": ";
			ForeachContainer(V v, Compsub) {
				out << g.name_of_one_node(v) << ' ';
			}
			out << endl;
			n++;
		}
	}
};

} // namespace cliques


#endif
