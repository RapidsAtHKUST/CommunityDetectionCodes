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

#endif
