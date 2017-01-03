#include <zconf.h>
#include <fcntl.h>
#include <sys/mman.h> // for mmap(2)
#include <sys/stat.h> // for fstat64(2)

#include <string.h>
#include <cerrno>

#include <bitset> // to help find the distinct vertices
#include <algorithm>
#include <set>

#include "graph_loading.hpp"

namespace graph_loading {
    struct readEdgeInvalidLineInDataException : public exception {
    }; // TODO: Proper error message, to be throw by readEdge()
    const char *readEdge(const char *cur, int &l, int &r) throw(readEdgeInvalidLineInDataException);

    template<class Name>
    class RangeOfEdges {
    private:
        const char *p;
        const char *updating_p;
        const char *const fileEnd;
        Name l, r;

        void read_a_row() { updating_p = readEdge(p, l, r); }

    public:
        RangeOfEdges(const char *b, const char *e) : p(b), fileEnd(e) { if (p != fileEnd) read_a_row(); }

        pair<Name, Name> front() { return make_pair(l, r); }

        bool empty() { return p == fileEnd; }

        void popFront() {
            p = updating_p;
            if (p != fileEnd) read_a_row();
        }
    };

    pair<const char *, const char *> mmapFile(const char *fileName);

    void loadBloomGraphMMAPFastButFussy(SimpleIntGraph &bg, const char *fileName);

    void findDistinctVertices(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg);

    void countDegrees(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg);

    void convertDegreesToOffsets(SimpleIntGraph &bg);

    void populateNameToIDHash(
            SimpleIntGraph &bg); // A hash used temporarily during graph loading to test if a given edge has new nodes in it.
    static void
    addNeighbour(VertexIDType edge_source, VertexIDType edge_target, VertexIDType _defaultVertexID, long &_edges_added,
                 SimpleIntGraph &_bg);

    void loadEdges(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg);

    void loadSimpleIntGraphFromFile(SimpleIntGraph &bg, const char *fileName) {
        loadBloomGraphMMAP(bg, fileName);
    }

    void loadBloomGraphMMAP(SimpleIntGraph &bg, const char *fileName) {
        // loadBloomGraphMMAPFastButFussy(bg, fileName); return;
        const char *fileBegin, *fileEnd;
        make_refpair(fileBegin, fileEnd) = mmapFile(fileName);
        const RangeOfEdges<SimpleIntGraph::Name> roe(fileBegin,
                                                     fileEnd); // Never un-const this. Take a copy of it if necessary.

        typedef SimpleIntGraph::Name N;
        typedef pair<N, N> Pair;

        set<pair<N, N> > edges;
        set<N> *names = new set<N>;

        { // populate the list of distinct vertices
            RangeOfEdges<SimpleIntGraph::Name> roe2(roe);
            pair<SimpleIntGraph::Name, SimpleIntGraph::Name> edge;
            Foreach(edge, roe2) {
                        names->insert(edge.first);
                        names->insert(edge.second);
                    }
            ForeachContainer(N n, *names) {
                        bg.vertex_mappings.push_back(n);
                    }
            free(names);
            names = NULL; // free this asap to save a few scraps of RAM. All its data is in bg.vertex_mappings now.
            bg.vertex_count = bg.vertex_mappings.size();
        }
        {
            RangeOfEdges<SimpleIntGraph::Name> roe2(roe);
            Pair edge;
            Foreach(edge, roe2) {
                        if (edge.first != edge.second) {
                            edges.insert(edge);
                            swap(edge.first, edge.second);
                            edges.insert(
                                    edge); // we have to add it both ways here. Let's not worry about the efficiency of this yet!
                        }
                    }
            bg.edge_count = edges.size();
        }
        populateNameToIDHash(bg);
        {
            set<pair<N, N> >::const_iterator edge = edges.begin();
            size_t offset = 0;
            for (V v = 0; v < (long) bg.vcount(); v++) {
                N n = bg.name_of_one_node(v);
                int degree = 0;
                while (edge != edges.end() && edge->first == n) {
                    degree++;
                    bg.edge_targets.push_back(bg.key_for_vertexName(edge->second));
                    edge++;
                }
                bg.degrees.emplace_back(degree);
                bg.offsets.emplace_back(offset);
                offset += degree;
            }
            assert(offset == bg.edge_targets.size());
            bg.offsets.emplace_back(offset);
        }

        PP(bg.ecount());
        PP(bg.vcount());
        PP(bg.edge_targets.size());
        PP(bg.degrees.size());
        PP(bg.offsets.size());

    }

    void loadBloomGraphMMAPFastButFussy(SimpleIntGraph &bg, const char *fileName) {

        const char *fileBegin, *fileEnd;
        make_refpair(fileBegin, fileEnd) = mmapFile(fileName);
        const RangeOfEdges<SimpleIntGraph::Name> roe(fileBegin, fileEnd);

        findDistinctVertices(roe, bg);
        countDegrees(roe, bg);
        populateNameToIDHash(bg);
        convertDegreesToOffsets(bg);
        loadEdges(roe, bg);


        //Pn("Sleeping for 5 seconds"); sleep(5);
        // To load vertices in, no edges yet. 10m nodes 7.5m edges. // 7767637 distinct, I think
        //  151M     9s
        // To load vertices in, no edges yet. 20m nodes 75m edges. // 19988969 distinct, I think
        // 1303M   100s
        // To load vertices in, no edges yet. 40m nodes 75m edges. // 39058948 distinct, I think
        // 1264M   125s // lower mem! Not really as surprising as it seems though.
        // To load vertices in, no edges yet. 50m nodes 350m edges. //  distinct, I think
        //         618s

        // To load vertices in, counting degree. 40m nodes 75m edges. // 39058948 distinct, I think
        //         140s
        // To load vertices in, no edges yet. 50m nodes 350m edges. // 49999965 distinct, I think
        //         710

        // Edges loaded                        10m nodes   7.5m edges.
        //             17s
        // Edges loaded                        20m nodes  75m edges.
        // 100:120    262s
        // Edges loaded                        40m nodes  75m edges.
        // Edges loaded                        50m nodes 350m edges.

        // Hash                                20m nodes  75m edges.
        //            199s (+84 for edges)
        // Hash                                40m nodes  75m edges.
        //            236s (+93 for edges)
    }

    void findDistinctVertices(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg) {
        typedef unsigned int ID;
        vector<SimpleIntGraph::Name> &vertices = bg.vertex_mappings; // This is empty at the start. It'll be grown here.

#define BLOOM_SIZE 800000000 // 100MB should be useful, and not a waste of memory
        bitset<BLOOM_SIZE> *bloom = new bitset<BLOOM_SIZE>; // Just to help, quickly, remember if a vertex has already been found.


        /* TODO
         * Use a set, not extendin the vector, for the most recent additions.
         * Store as binary file.
         * Calculate a good value for k (the bloom filter) and use it.
         */
        struct nested {
            static void addNewOrIncreaseDegree(vector<SimpleIntGraph::Name> &_vertices, const long _numSorted,
                                               const SimpleIntGraph::Name &addThis, bitset<BLOOM_SIZE> *_bloom) {
                if (_vertices.size() >=
                    2) { // OPTIMIZE BASED on idea that, in many data files, consecutive edges will mention similar edges
                    decltype(_vertices.rbegin()) last = _vertices.rbegin();
                    if (*last == addThis) { return; }
                    last++;
                    if (*last == addThis) { return; }
                }

                unsigned int h = hash_integer<unsigned int>(addThis, BLOOM_SIZE);
                unless(h < BLOOM_SIZE) {
                    PP(BLOOM_SIZE);
                    PP(h);
                }
                assert(h < BLOOM_SIZE);
                assert(h < _bloom->size());
                //PP(h);
                if (!_bloom->test(h)) { // definitely haven't seen this vertex before. Add it and get outta here.
                    _bloom->set(h);
                    _vertices.push_back(addThis);
                    return;
                }

                decltype(_vertices.end()) splitHere = _vertices.begin() + _numSorted;
                if (!binary_search(_vertices.begin(), splitHere, addThis))
                    _vertices.push_back(addThis); // This is (probably) a new node. It's degree is 1.
            }

            static void mergeInTheRecentAdditions(vector<SimpleIntGraph::Name> &_vertices, unsigned long &_numSorted) {
                sort(_vertices.begin() + _numSorted, _vertices.end()); // Sort those that have recently been added.
                _vertices.erase(unique(_vertices.begin() + _numSorted, _vertices.end()),
                                _vertices.end()); // Remove the duplicates. among them
                inplace_merge(_vertices.begin(), _vertices.begin() + _numSorted,
                              _vertices.end()); // We know we're merging non-overlapping sets, so we can finish now.
                _numSorted = _vertices.size();
            }
        };


        unsigned long numSorted = 0; // This many elements of vertices are known to be sorted.
        pair<SimpleIntGraph::Name, SimpleIntGraph::Name> edge;
        Foreach(edge, roe) {
                    const SimpleIntGraph::Name &l = edge.first;
                    const SimpleIntGraph::Name &r = edge.second;

                    nested::addNewOrIncreaseDegree(vertices, numSorted, l, bloom);
                    nested::addNewOrIncreaseDegree(vertices, numSorted, r, bloom);

                    if (vertices.size() * 0.8 - numSorted >=
                        10000000) // This could be tweaked. It shouldn't be too aggressive until we think we're running low on memory.
                        nested::mergeInTheRecentAdditions(vertices, numSorted);
                }
        nested::mergeInTheRecentAdditions(vertices, numSorted);

        bg.vertex_count = vertices.size();

        delete bloom;
    }

    void countDegrees(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg) {
        bg.degrees.resize(bg.vertex_count);

        pair<SimpleIntGraph::Name, SimpleIntGraph::Name> edge;
        Foreach(edge, roe) {
                    bg.degrees.at(bg.key_for_vertexName(edge.first))++;
                    bg.degrees.at(bg.key_for_vertexName(edge.second))++;
                }

    }

    void convertDegreesToOffsets(SimpleIntGraph &bg) {
        vector<SimpleIntGraph::Name> &vs = bg.vertex_mappings;
        bg.offsets.resize(1 + bg.vcount());

        long total = 0;

        typedef vector<SimpleIntGraph::Name>::iterator PairIterator;
        PairIterator endVertex = vs.begin() + bg.vcount();
        PairIterator i;
        unsigned int counter = 0;
        for (i = vs.begin(); i !=
                             endVertex; ++i) { // TODO: Move to my own range. clique.idiro.com doesn't appear to have up-to-date foreach.hpp. And I don't like the C++ Range; maybe should move to D after all.
            long degree = bg.degrees.at(counter);
            bg.offsets.at(counter) = total;
            total += degree;
            ++counter;
        }
        bg.offsets.at(counter) = total;

        bg.edge_count = total;
    }

    void populateNameToIDHash(SimpleIntGraph &bg) {
        /* See also key_for_vertexName */
        assert(bg.hash_offsets.empty());
        bg.hash_offsets.resize(size_t(bg.vertex_count * 1.50), 0);
        for (long int i = 0; i < bg.vertex_count; i++) {
            unsigned int h = hash_integer<unsigned int>(bg.name_of_one_node(i), bg.hash_offsets.size());
            for (unsigned int j = 0; j < 5 && h + j <
                                              bg.hash_offsets.size(); j++) { // TODO: put this code into Graph.hpp and have proper hashLookup and hashInsert functions.
                if (bg.hash_offsets.at(h + j) == 0) {
                    bg.hash_offsets.at(h + j) = i;
                    break;
                }
            }
        }
    }


    static void
    addNeighbour(VertexIDType edge_source, VertexIDType edge_target, VertexIDType _defaultVertexID, long &_edges_added,
                 SimpleIntGraph &_bg) {
        // TODO: throw if not-simple graph

        if ( /* if supposed to be simple */ edge_source == edge_target) {
            Die("Loop detected. Quitting. %d <> %d", _bg.name_of_one_node(edge_source),
                _bg.name_of_one_node(edge_target));
        }

        _edges_added++;
        /*
        if( flag_load_progress && ((_edges_added * 25) % _bg.ecount()) < 25) {
            Pn("edges %5.1f%% (%ld/%ld)" ,(_edges_added*50.0)/_bg.ecount() ,_edges_added, 2*_bg.ecount());
        }
        */
        VertexIDType *_from, *_to;
        make_refpair(_from, _to) = _bg.neighbours(edge_source);
        VertexIDType *firstNonEmptySlot = upper_bound(_from, _to, _defaultVertexID);
        // Note, the neighbours aren't fully sorted, just the empty slots are to the left of the full slots. Therefore, the upper_bound should be OK.
        if ( /* if supposed to be simple */ firstNonEmptySlot != _to) {
            if (*firstNonEmptySlot == edge_target)
                Die("That edge has already been entered. Only simple graphs are allowed in this version. Quitting. %d <> %d",
                    _bg.name_of_one_node(edge_source), _bg.name_of_one_node(edge_target));
        }
        firstNonEmptySlot--;
        if (*firstNonEmptySlot != _defaultVertexID)
            Die ("already entered all of these neighbours. This is an unexpected bug");
        *firstNonEmptySlot = edge_target;
        if (_from == firstNonEmptySlot) sort(_from, _to);
    }

    void loadEdges(RangeOfEdges<SimpleIntGraph::Name> roe, SimpleIntGraph &bg) {

        vector<VertexIDType> &edge_targets = bg.edge_targets;
        VertexIDType defaultVertexID = -1;
        edge_targets.resize(bg.edge_count, defaultVertexID);

        long edges_added = 0; // just so that we can keep counting for any progress bar we might like..
        pair<SimpleIntGraph::Name, SimpleIntGraph::Name> edge;
        Foreach(edge, roe) {
                    SimpleIntGraph::ID lkey = bg.key_for_vertexName(edge.first);
                    SimpleIntGraph::ID rkey = bg.key_for_vertexName(edge.second);
                    addNeighbour(lkey, rkey, defaultVertexID, edges_added, bg);
                    addNeighbour(rkey, lkey, defaultVertexID, edges_added,
                                 bg); // TODO: decide how to store loops ... Will I store the second entry? What'll that to do _edges_added?
                }
        // if(flag_load_progress) {
        // PP(edge_targets.size());
        // PP(edges_added);
        // }
    }

    pair<const char *, const char *> mmapFile(const char *fileName) {
        off_t fileSize;
        int graphFD = open(fileName, O_RDONLY);
        graphFD != -1 || Die("open(%s) failed", fileName);
        //PP(sizeof(char *)); // Must be 8 bytes (i.e. 64 bit) to load up the biggest files.
        {
            struct stat64 buf;
            fstat64(graphFD, &buf) == 0 || Die("fstat64 failed");
            fileSize = buf.st_size;
        }
        const void *map = mmap(NULL, fileSize, PROT_READ, MAP_SHARED
//#ifdef PLATFORM_MAC
                //|MAP_NOCACHE // flags
//#endif
                , graphFD, 0);
        map != (void *) -1 || Die("mmap failed");
        close(graphFD);
        const char *const fileBegin = (char *) map;
        const char *const fileEnd =
                fileBegin + fileSize; // I const the points also, so that I never accidentally change them.
        fileEnd[-1] == '\n' || Die("The file MUST be terminated by a newline");
        return make_pair(fileBegin, fileEnd);
    }


    const char *readEdge(const char *cur, int &l, int &r) throw(readEdgeInvalidLineInDataException) {
        while (*cur == '#' || *cur == '*') {
            cur = strchr(cur, '\n');
            ++cur;
        } // This'll fail is the final lines in the file are like this. Gotta reorganize the file reading

        char *endptr;

        errno = 0;
        l = strtol(cur, &endptr, 10);
        if (errno == EINVAL) { throw (readEdgeInvalidLineInDataException()); }

        while (!isdigit(*endptr)) endptr++; // advance to the start of the second name

        errno = 0;
        r = strtol(endptr, &endptr, 10);
        if (errno == EINVAL) { throw (readEdgeInvalidLineInDataException()); }

        endptr = strchr(endptr,
                        '\n'); // skip over to the start of the next line. This is why the file must be terminated by a newline ('\n')
        endptr++;
        return endptr;
    }


} // namespace graph_loading
