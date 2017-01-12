slpa-py
=======

Speaker-listener Label Propagation Algorithm (SLPA). Xie et. al. 2011 

The program detects (overlapping) communities in the network. Experiment 
shows this algorithm converges after 20 iterations and the result is 
quite competitive. 

I implemented it in Python. Works for graph with nodes at scale 10,000. 
The program may not scale to real network with millions of code.

The input format is each line is an undirected edge with two nodes. 
The are separated by a tab. Currently the names of the node have to be
consecutive integers starting from 1. Example below:

1     20
2     1
233   22
...

I didn't fix the output. Currently the output nodes has 

index = original index - 1

The output shows for each node the communities it belongs to. 
