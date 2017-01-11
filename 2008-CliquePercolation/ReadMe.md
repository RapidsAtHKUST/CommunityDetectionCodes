#Clique Percolation
##Description

The sequential clique percolation algorithm is method for detecting clique percolation communities. 
It is an alternative to CFinder: instead of finding maximal cliques in a graph and producing 
communities of all the possible clique sizes, 
the SCP algorithm finds all the cliques of single size and 
produces the community structure for all the possible thresholds. 
The SCP algorithm should work well even for large sparse networks, 
but might have trouble for dense networks and cliques of large size.



##Links
- [python implementation elaboration](http://www.lce.hut.fi/~mtkivela/kclique.html)
- [c++ implementation elaboration](http://www.lce.hut.fi/research/mm/complex/software/scp_0.1.tar.gz)
- [authors' paper, A sequential algorithm for fast clique percolation](https://arxiv.org/abs/0805.1449)