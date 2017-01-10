#include <getopt.h>
#include <list>
#include <set>
#include <tr1/unordered_set>
#include <tr1/unordered_map>
#include <fstream>
#include <algorithm>

#include "graph_rep.hpp"
#include "aaron_utils.hpp"
#include "Range.hpp"

const char gitstatus[] = 
#include "comment.txt"
#include "gitstatus.txt"
;

using namespace std;
using namespace std;

int main(int argc, char **argv) {
	PP(gitstatus);
	for (int i=0; i<argc; i++) {
		PP(argv[i]);
	}
	{ int c, option_index; while (1)
		{
      static const struct option long_options[] = {
        // {"saveMOSESscores", required_argument,         0, 21},
        // {"seed", required_argument,       0, 22},
        {0, 0, 0, 0}
      };
      /* getopt_long stores the option index here. */
      c = getopt_long (argc, argv, "", long_options, &option_index);
      if (c == -1) break; /* Detect the end of the options. */
     
      switch (c) {
        case '?': /* getopt_long already printed an error message. */ break;
        default: abort (); break;
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg) printf (" with arg %s", optarg);
          printf ("\n");
          break;

/*
        case 21: // --saveMOSESscores= {"saveMOSESscores", required_argument,         0, 21},
					strcpy(option_saveMOSESscores, optarg);
					break;
        case 22: // --seed= {"seed", required_argument,         0, 22},
					option_seed = atol(optarg);
					break;
*/

      }
    }
	}

	if (argc - optind != 2) {
		cout << "Usage: " << endl;
		exit(1);
	}

	string graphFileName = argv[optind];
	string groupingFileName = argv[optind+1];

	PP(graphFileName);
	PP(groupingFileName);

	graph_rep::SGraph _g;
	graph_rep::loadSGraph(_g, graphFileName.c_str());
	const graph_rep::SGraph &g = _g;
	PP(g.num_edges);
	PP(g.num_vertices);

	typedef uset<string> GroupT;
	list<GroupT > groups;
	ifstream groupingFile(groupingFileName.c_str());
	if(!groupingFile.is_open())
		Die("grouping file '%s' doesn't exist. Exiting.", groupingFileName.c_str());
	while(groupingFile.peek() != EOF) {
		string line;
		getline(groupingFile, line);

		if(line.length()==0) continue; // ignore empty lines

		istringstream lineStream(line);
		if(lineStream.peek()=='"') {
			char ch;
			lineStream >> ch; // skip over that first "
			while(1) {
				if(lineStream.peek()==EOF) {
					Die("grouping line with unterminated \" ");
				}
				lineStream >> ch;
				if(ch=='"')
					break;
			}
		}

		uset<string> thisGroup;
		while(1) {
			string node;
			lineStream >> node;
			if(node.length()==0) break;
			thisGroup.insert(node);
			//PP(node);
		}
		//PP(thisGroup.size());
		groups.push_back(thisGroup);
	}
	//PP(groups.size());

	umap<string, set<const GroupT *> > whatGroupsAmIIn;
	ForeachContainer(const GroupT &group, groups) {
		for(GroupT::const_iterator it = group.begin(); it != group.end(); ++it) {
			const string &node = *it;
			whatGroupsAmIIn[node].insert(&group);
		}
	}

	//for(int v=0; v<g.num_vertices; v++) {
		//const string &node = g.vertexId2VertexName.at(v);
		//cout << node << "\t" << whatGroupsAmIIn[node].size() << endl;
	//}

#if 0
	for(int v=0; v<g.num_vertices; v++) {
		pair< const pair<int,int> *, const pair<int,int> *> neighs = g.neighbours(v);
		const pair<int,int> * currentNeighbour = neighs.first;
		const pair<int,int> * lastNeighbour = neighs.second;
		// PP(g.degree.at(v));
		for( ; currentNeighbour != lastNeighbour; ++currentNeighbour) {
			int v2 = currentNeighbour->second;
			if(v2 < v)
				continue;
			cout << v << "\t" << v2 << endl;
			int o = currentNeighbour - &(g.edges_both_directions[0]);
			assert(o >= 0);
			assert(o < 2*g.num_edges);
			int rel = g.relationship_Ids.at(o);
			PP(rel);
		}
	}
#endif

	umap<const GroupT *, int> howManyEdgesInEachGroup;
	umap<const GroupT *, int> howManyEdgesInEachGroupsFrontier;
	umap<const GroupT *, GroupT> nodesInEachGroupsFrontier;
	umap<const GroupT *, umap<string, int> > nodeAndFreqInEachGroupsFrontier;

	for (int rel=0; rel < g.num_edges; rel++) {
		int off = g.relId2EdgeId.at(rel);
		pair<int,int> thePair = g.edges_both_directions.at(off);
		const string &node1 = g.vertexId2VertexName.at(thePair.first);
		const string &node2 = g.vertexId2VertexName.at(thePair.second);
		//cout << node1 << "<>" << node2 << "\t"; // << endl;

		const set<const GroupT *> & node1Groups = whatGroupsAmIIn[node1];
		const set<const GroupT *> & node2Groups = whatGroupsAmIIn[node2];
/*
		for(set<const GroupT *>::const_iterator it = node1Groups.begin(); it != node1Groups.end(); ++it) {
			const GroupT * grp = *it;
			Pn("name %s is in %p", node1.c_str(), grp);
		}

		for(set<const GroupT *>::const_iterator it = node2Groups.begin(); it != node2Groups.end(); ++it) {
			const GroupT * grp = *it;
			Pn("name %s is in %p", node2.c_str(), grp);
		}
*/

		set<const GroupT *> sharedGroups;

		set_intersection(
			node1Groups.begin(), node1Groups.end(),
			node2Groups.begin(), node2Groups.end(),
			inserter(sharedGroups, sharedGroups.begin())
		);

		ForeachContainer(const GroupT *grp, sharedGroups) {
			// Pn("sharedGroup %p", grp);
			howManyEdgesInEachGroup[grp]++;
		}
		//PP(sharedGroups.size());

		{ // the Frontier stats
			set<const GroupT *> onlyNode1;
			set<const GroupT *> onlyNode2;
			set_difference(
				node1Groups.begin(), node1Groups.end(),
				node2Groups.begin(), node2Groups.end(),
				inserter(onlyNode1, onlyNode1.begin())
			);
			set_difference(
				node2Groups.begin(), node2Groups.end(),
				node1Groups.begin(), node1Groups.end(),
				inserter(onlyNode2, onlyNode2.begin())
			);
			assert(onlyNode1.size() + sharedGroups.size() == node1Groups.size());
			assert(onlyNode2.size() + sharedGroups.size() == node2Groups.size());
			ForeachContainer(const GroupT *grp, onlyNode1) {
				howManyEdgesInEachGroupsFrontier[grp]++;
				// node2 is in the frontier of each of the onlyNode1 groups
				nodesInEachGroupsFrontier[grp].insert(node2);
				nodeAndFreqInEachGroupsFrontier[grp][node2]++;
			}
			ForeachContainer(const GroupT *grp, onlyNode2) {
				howManyEdgesInEachGroupsFrontier[grp]++;
				nodesInEachGroupsFrontier[grp].insert(node1);
				nodeAndFreqInEachGroupsFrontier[grp][node1]++;
			}
		}

	}

	ForeachContainer(const GroupT &group, groups) {
		map<int, int> freqOfFreqs;
		const umap<string, int> &nodeAndFreqInMyFrontier = nodeAndFreqInEachGroupsFrontier[&group];
		for(umap<string, int>::const_iterator it = nodeAndFreqInMyFrontier.begin(); it != nodeAndFreqInMyFrontier.end(); ++it) {
			// const string &node = it->first;
			const int freq = it->second;
			freqOfFreqs[freq]++;
		}

		const int64 n = group.size();
		cout /*<< "#nodes: "*/ << n
			<< "\t" /*<< "#edges"*/ << howManyEdgesInEachGroup[&group];
		P("\t%8f" /*<< "density"*/ , double(howManyEdgesInEachGroup[&group]) / (n*(n-1)/2));
		cout << "\t" /*<< "#edges"*/ << howManyEdgesInEachGroupsFrontier[&group]
			// << "\t" /*<< "#edges"*/ << nodesInEachGroupsFrontier[&group].size()
			<< "\t" /*<< "#edges"*/ << nodeAndFreqInMyFrontier.size();
		pair<int, int> ff;
		ForeachContainer(ff, freqOfFreqs) {
			cout << "\t{" << ff.first << "}" << ff.second;
		}
		cout << endl;
	}
}
