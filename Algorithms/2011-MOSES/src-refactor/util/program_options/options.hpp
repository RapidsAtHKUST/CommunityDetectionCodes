#ifndef _OPTIONS_HPP_
#define _OPTIONS_HPP_

extern int flag_assignOffsets;
extern int flag_planted; // run the iterative planted partition model
#ifdef USE_BOOST
extern char option_sizesOfComponents[1000];
#endif
extern char option_degreeDist[1000];
extern char option_localq[1000]; // filename to store some node-by-node and comm-by-node stats on modularity
extern char option_planted[1000];
extern char option_initialPartition[1000];
extern char option_subgraphNodes[1000]; // print out the edges in the subgraph. The nodes that we want are defined in this file.
extern char option_subgraphTo[1000]; // print out the edges in the subgraph. The nodes that we want are defined in this file.
#ifdef USE_BOOST
extern char option_waves[1000];
#endif
extern int flag_HALFMERGES /*= 0*/; // default is the fastest
extern int flag_MERGES_INTERLEAVED /*= 0*/; // default is the fastest
extern int flag_MERGES_ATEND /*= 1*/; // default is the fastest
extern int option_maxDegreeInComm; 
extern int flag_dumpEvery3Iters;
extern int option_ignoreNlines; 
extern char option_overlapping[1000];
extern char option_loadOverlapping[1000];
extern long double option_p_in; 
extern long double option_p_out; 
extern int option_maxDegree; 
extern int option_partition_as_partition;
extern char option_saveMOSESscores[1000];
extern long option_seed;
extern int flag_justCalcObjective;

#endif
