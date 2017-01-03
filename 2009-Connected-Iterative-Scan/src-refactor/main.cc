#include <algorithm>

#include "graph/temporal_network.h"
#include "util/parameters_helper.h"

using namespace std;

/**
 *void Print( const set < shared_ptr < string >, cmp_str_ptr >& seed )
 *
 * Simple helper function. Prints the members of a set on the same line (with following newline).
 *
 *@param seed Vertices to print
 */
void Print(const set<shared_ptr<string>, cmp_str_ptr> &seed) {
    set<shared_ptr<string> >::const_iterator it_s = seed.begin();
    while (it_s != seed.end()) {
        cout << **it_s << "|";
        ++it_s;
    }
    cout << endl;
}

/**
 *double CalcDensity( const int& size, const double& Win, const double& Wout, const double& lambda )
 *
 *   Function to calculate the density of a given set of vertices
 *
 *@param size Number of vertices in the set
 *@param Win Total weight of edges between two vertices in the set
 *@param Wout Total weight of edges with only 1 vertex in the set
 *@param lambda Weight value between the two parts of the density measure
 *
 *@return Density of set of vertices
 */
double CalcDensity(const int &size, const double &Win, const double &Wout, const double &lambda) {
    if (size < 1) return numeric_limits<double>::min();

    double partA = ((1 - lambda) * (Win / (Win + Wout)));

    double partB = (lambda * ((2 * Win) / (size * (size - 1))));
    if (size == 1) partB = lambda;

    return partA + partB;
}

/**
 *map < double, set < shared_ptr < string >, cmp_str_ptr > > Components ( set < shared_ptr < string >, cmp_str_ptr > seed, shared_ptr < Network > G, double lambda )
 *
 *  Splits a set of vertices into the connected components that it is made up of. Associates each component
 * with its density value.
 *
 *@param seed Set of vertices to split
 *@param G Network structure
 *@param lambda Value for density calculation (see CalcDensity)
 *
 *@return A map associating density values with the components
 */
map<double, set<shared_ptr<string>, cmp_str_ptr> >
Components(set<shared_ptr<string>, cmp_str_ptr> seed, shared_ptr<network> G, double lambda) {
    map<double, set<shared_ptr<string>, cmp_str_ptr> > result;

    set<shared_ptr<string>, cmp_str_ptr>::iterator it_s, it_u, it_v;
    set<shared_ptr<string>, cmp_str_ptr> seen;
    double win = 0, wout = 0;

    for (it_s = seed.begin(); it_s != seed.end(); it_s++) {                   //Go through each vertex in set
        if (seen.find(*it_s) != seen.end()) continue;                         //If already assigned to partition, skip

        win = 0;
        wout = 0;
        set<shared_ptr<string>, cmp_str_ptr> component, to_check;
        to_check.insert(*it_s);                                                  //Initialize stats for new component

        //Keep expanding the component based off the neighborhoods of members until no new members can be added
        while (to_check.size() != 0) {
            //Add the first vertex to component
            it_u = to_check.begin();
            component.insert(*it_u);
            seen.insert(*it_u);
            map<shared_ptr<string>, double, cmp_str_ptr> N = G->GetNeighborhood(*it_u);
            to_check.erase(it_u);

            //Go through vertex neighborhodd to find vertices that are also in the component while tracking density measures
            map<shared_ptr<string>, double, cmp_str_ptr>::iterator it_n;
            for (it_n = N.begin(); it_n != N.end(); it_n++) {
                if ((seen.find(it_n->first) == seen.end()) && (seed.find(it_n->first) != seed.end())) {
                    to_check.insert(it_n->first);
                    win += it_n->second;
                } else {
                    wout += it_n->second;
                }
            }
        }

        //document results
        result.insert(
                pair<double, set<shared_ptr<string>, cmp_str_ptr> >(CalcDensity(component.size(), win, wout, lambda),
                                                                    component));
    }

    //Return full results
    return result;
}

/**
 *void ExpandSeed ( set < shared_ptr < string >, cmp_str_ptr >& seed, shared_ptr < Network > G, double lambda )
 *
 * Main portion of CIS. Takes a seed and iteratively adds neighbors/removes members in order to maximize the
 *  CalcDensity() value of the set of vertices. Only rule is that the component must remain connected. If it
 *  becomes disconnected, the component that has the highest independant density is taken.
 *
 *@param seed Set of vertices to start from
 *@param G Network structure
 *@param lambda Value for density calculation
 */
void ExpandSeed(set<shared_ptr<string>, cmp_str_ptr> &seed, shared_ptr<network> G, double lambda) {
    cout << *(*seed.begin()) << ",,,";
    map<shared_ptr<string>, pair<double, double>, cmp_str_ptr> members, neighbors;
    set<shared_ptr<string>, cmp_str_ptr> fringe;
    set<shared_ptr<string>, cmp_str_ptr>::iterator it_s;
    map<shared_ptr<string>, double, cmp_str_ptr>::iterator it_n;
    map<shared_ptr<string>, pair<double, double>, cmp_str_ptr>::iterator it_d, it_d2;

    double seed_win = 0, seed_wout = 0;

    for (it_s = seed.begin(); it_s != seed.end(); it_s++) {  //Tally members of the seed, calculating individual
        // Win and Wout measures and noting neighborhood
        map<shared_ptr<string>, double, cmp_str_ptr> N = G->GetNeighborhood(*it_s);
        double Win = 0, Wout = 0;
        for (it_n = N.begin(); it_n != N.end(); it_n++) {
            if (seed.find(it_n->first) != seed.end()) {     //If the neighbor is also in the seed, increase weight_in
                Win += it_n->second;
                seed_win += it_n->second;
            } else {                                            //Else increase weight out
                Wout += it_n->second;
                seed_wout += it_n->second;
                fringe.insert(it_n->first);
            }
        }

        members.insert(pair<shared_ptr<string>, pair<double, double> >(shared_ptr<string>(*it_s),
                                                                       pair<double, double>(Win, Wout)));
        //cout << **it_s << " : " << Win << " " << Wout << endl;
    }

    seed_win /= 2.0;  //Internal edges were counted twice (assumed undirected)

    //cout << "Fringe: " << endl;

    for (it_s = fringe.begin(); it_s != fringe.end(); it_s++) { //Tally same information for neighborhood
        map<shared_ptr<string>, double, cmp_str_ptr> N = G->GetNeighborhood(*it_s);
        double Win = 0, Wout = 0;
        for (it_n = N.begin(); it_n != N.end(); it_n++) {
            if (seed.find(it_n->first) != seed.end()) {
                Win += it_n->second;
            } else {
                Wout += it_n->second;
            }
        }

        neighbors.insert(pair<shared_ptr<string>, pair<double, double> >(shared_ptr<string>(*it_s),
                                                                         pair<double, double>(Win, Wout)));
        //cout << **it_s << " : " << Win << " " << Wout << endl;
    }

    bool changed = true;

    srand(time(NULL));

    //cout << endl << " NEW SEED " << endl << endl;

    //While the seed is changing, add new members and remove poor members
    while (changed) {
        changed = false;
        vector<shared_ptr<string> > to_check;
        vector<set<shared_ptr<string>, cmp_str_ptr> > order_by_degree;
        set<shared_ptr<string>, cmp_str_ptr>::iterator it_deg;

        for (it_d = neighbors.begin(); it_d != neighbors.end(); it_d++) {
            int deg = G->Degree(it_d->first);
            if (order_by_degree.size() < deg + 1) order_by_degree.resize(deg + 1);
            order_by_degree[deg].insert(it_d->first);
        }

        for (int k = 0; k < order_by_degree.size(); k++) {
            for (it_deg = order_by_degree[k].begin(); it_deg != order_by_degree[k].end(); it_deg++) {
                to_check.push_back(*it_deg);
            }
        }

        /*for ( it_d = neighbors.begin(); it_d != neighbors.end(); it_d++ ){
          to_check.push_back(it_d->first);
        }

        random_shuffle ( to_check.begin(), to_check.end() );   ///Get the neighborhood in a random order*/

        for (unsigned int i = 0; i < to_check.size(); i++) { // Go through all the neighbors
            it_d = neighbors.find(to_check[i]);

            //cout << *to_check[i] << " to be checked for addition : " << seed_win << " " << seed_wout << " " << it_d->second.first << " " << it_d->second.second << " " << CalcDensity(seed.size(), seed_win, seed_wout, lambda) << " " <<  CalcDensity(seed.size() + 1, seed_win + it_d->second.first, seed_wout + it_d->second.second - it_d->second.first, lambda) << endl;

            if (CalcDensity(seed.size(), seed_win, seed_wout, lambda) <
                CalcDensity(seed.size() + 1, seed_win + it_d->second.first,
                            seed_wout + it_d->second.second - it_d->second.first, lambda)) {
                //If the density would increase by includeing the vertex - do it
                //cout << "...Added" << endl;
                changed = true; //Mark the change in seed
                seed_win += it_d->second.first;
                seed_wout = seed_wout - it_d->second.first + it_d->second.second;
                seed.insert(it_d->first);  //Update seed
                members.insert(pair<shared_ptr<string>, pair<double, double> >(it_d->first, it_d->second));
                neighbors.erase(to_check[i]); //Update local trackers

                //UPDATE MEMBER AND NEIGHBOR LISTS
                // The Win and Wout values of vertices connected to the added vertex have changed...
                map<shared_ptr<string>, double, cmp_str_ptr> N = G->GetNeighborhood(to_check[i]);

                for (it_n = N.begin(); it_n != N.end(); it_n++) {
                    if ((it_d2 = members.find(it_n->first)) != members.end()) { //Update member
                        it_d2->second.first += it_n->second;
                        it_d2->second.second -= it_n->second;
                    } else if ((it_d2 = neighbors.find(it_n->first)) != neighbors.end()) { //Update current neighbor
                        it_d2->second.first += it_n->second;
                        it_d2->second.second -= it_n->second;
                    } else { //Add new neighbor
                        map<shared_ptr<string>, double, cmp_str_ptr> N2 = G->GetNeighborhood(it_n->first);
                        map<shared_ptr<string>, double, cmp_str_ptr>::iterator it_d3;
                        double newWin = 0, newWout = 0;
                        for (it_d3 = N2.begin(); it_d3 != N2.end(); it_d3++) {
                            if (members.find(it_d3->first) != members.end()) newWin += it_d3->second;
                            else newWout += it_d3->second;
                        }

                        neighbors.insert(pair<shared_ptr<string>, pair<double, double> >(it_n->first,
                                                                                         pair<double, double>(newWin,
                                                                                                              newWout)));
                    }
                }
            }

            /*Print ( seed );
            for ( it_d2 = members.begin(); it_d2 != members.end(); it_d2++ ){
          cout << *(it_d2->first) << "|";
            }
            cout << endl;*/
        }

        //REPEAT FOR MEMBERS (reversing mathematical signs where necessary, of course)
        to_check.clear();
        order_by_degree.clear();

        for (it_d = members.begin(); it_d != members.end(); it_d++) {
            int deg = G->Degree(it_d->first);
            if (order_by_degree.size() < deg + 1) order_by_degree.resize(deg + 1);
            order_by_degree[deg].insert(it_d->first);
        }

        for (int k = 0; k < order_by_degree.size(); k++) {
            for (it_deg = order_by_degree[k].begin(); it_deg != order_by_degree[k].end(); it_deg++) {
                to_check.push_back(*it_deg);
            }
        }

        /* for ( it_d = members.begin(); it_d != members.end(); it_d++ ){
          to_check.push_back(it_d->first);
        }
        random_shuffle ( to_check.begin(), to_check.end() );*/

        for (unsigned int i = 0; i < to_check.size(); i++) {
            it_d = members.find(to_check[i]);

            //cout << *to_check[i] << " to be checked for removal : " << seed_win << " " << seed_wout << " " << it_d->second.first << " " << it_d->second.second << " " << CalcDensity(seed.size(), seed_win, seed_wout, lambda) << " " <<  CalcDensity(seed.size() + 1, seed_win + it_d->second.first, seed_wout + it_d->second.second - it_d->second.first, lambda) << endl;

            if (CalcDensity(seed.size(), seed_win, seed_wout, lambda) <
                CalcDensity(seed.size() - 1, seed_win - it_d->second.first,
                            seed_wout - it_d->second.second + it_d->second.first, lambda)) {
                //cout << "...Removed" << endl;
                changed = true;
                seed_win -= it_d->second.first;
                seed_wout = seed_wout + it_d->second.first - it_d->second.second;
                seed.erase(it_d->first);
                neighbors.insert(pair<shared_ptr<string>, pair<double, double> >(it_d->first, it_d->second));
                members.erase(to_check[i]);

                //UPDATE MEMBER AND NEIGHBOR LISTS
                map<shared_ptr<string>, double, cmp_str_ptr> N = G->GetNeighborhood(to_check[i]);

                for (it_n = N.begin(); it_n != N.end(); it_n++) {
                    if ((it_d2 = members.find(it_n->first)) != members.end()) { //Update member
                        it_d2->second.first -= it_n->second;
                        it_d2->second.second += it_n->second;
                    } else if ((it_d2 = neighbors.find(it_n->first)) != neighbors.end()) { //Update current neighbor
                        it_d2->second.first -= it_n->second;
                        it_d2->second.second += it_n->second;
                    } //No new neighbors can be added to consider when removing members
                }
            }
        }

        //Get best component to move forward with
        //map < double, set < shared_ptr < string >, cmp_str_ptr > > comps = Components(seed, G, lambda);
        //seed = (comps.begin())->second;

        //Print ( seed );
    }
}

/**
 *void Print( const set < shared_ptr < string >, cmp_str_ptr >& seed )
 *
 * Simple helper function. Prints the members of a set on the same line (with following newline).
 *
 *@param seed Vertices to print
 */
void Print(const set<shared_ptr<string>, cmp_str_ptr> &seed, ofstream &fout, const string delim) {
    set<shared_ptr<string> >::const_iterator it_s = seed.begin();

    fout << **it_s;

    ++it_s;
    while (it_s != seed.end()) {
        fout << delim << **it_s;
        ++it_s;
    }
    fout << endl;
}


/**
 *Insertion point for CIS
 */
int main(int argc, char **argv) {
    //Get command line params
    parameters_helper P;
    P.Read(argc, argv);

    string inputfile, outputfile, delimiters, seed_file, seed_delim, output_delim;
    bool directed, given_seeds;
    double lambda;

    P.set<string>(&inputfile, "i", "network.dat");
    P.set<string>(&outputfile, "o", "clusters.dat");
    P.set<string>(&delimiters, "dl", "|");
    P.set<string>(&seed_file, "s", "seeds.dat");
    P.set<string>(&seed_delim, "sdl", "|");
    P.set<double>(&lambda, "l", 0);
    P.set<string>(&output_delim, "odl", "|");

    P.boolset(&given_seeds, "s");
    P.boolset(&directed, "dir");

    if (argc < 2) {
        cout << "Usage:\n\t./cis -i network -o output -dl delimiter -s seed file -l lambda value" << endl;
        return 0;
    }

    //Load the network
    temporal_network T;
    shared_ptr<network> G(T.AddNetwork(inputfile, delimiters, directed));
    set<set<shared_ptr<string>, cmp_str_ptr>, cmp_set_str> results;

    // Either read seeds from file or go through each vertex as seed.
    // Expand each seed and record the result
    if (given_seeds) {
        ifstream fin;
        openFileHarsh(&fin, seed_file);

        vector<string> fields;

        while (fline_tr(&fin, &fields, seed_delim)) {
            set<shared_ptr<string>, cmp_str_ptr> seed;
            for (unsigned int i = 0; i < fields.size(); i++) {
                seed.insert(shared_ptr<string>(new string(fields[i])));
            }

            ExpandSeed(seed, G, lambda);
            results.insert(seed);
        }
    } else {
        set<string>::const_iterator it_v = T.getFirstVertex();
        while (it_v != T.getLastVertex()) {
            set<shared_ptr<string>, cmp_str_ptr> seed;
            seed.insert(shared_ptr<string>(new string(*it_v)));

            ExpandSeed(seed, G, lambda);
            cout << "!!!" << seed.size() << "\t:";
            for (auto str:seed) {
                cout << *str << ",";
            }
            cout << endl;
            results.insert(seed);

            //cout << seed.size() << endl;
            ++it_v;
        }
    }

    for (auto iter_tmp = results.begin(); iter_tmp != results.end(); ++iter_tmp) {
        for (auto iter_tmp2 = (*iter_tmp).begin(); iter_tmp2 != (*iter_tmp).end(); ++iter_tmp2) {
            cout << *(*iter_tmp2) << ",";
        }
        cout << endl;
    }
    //Print resulting communities
    set<set<shared_ptr<string>, cmp_str_ptr>, cmp_set_str>::iterator it_ss;
    ofstream fout(outputfile.c_str());
    for (it_ss = results.begin(); it_ss != results.end(); it_ss++) {
        Print(*it_ss, fout, output_delim);
    }
    fout.close();

    return 0;
}
