#Some Comments

```cpp
/*
		 (x2 * (log2l(x2)-log2l(N))  + (N-x2)  * (log2l(N-x2)-log2l(N))  + log2l(1+ging.groups.size()) - log2l(N))
		-(x  * (log2l(x) -log2l(N))  + (N-x )  * (log2l(N-x )-log2l(N))  + log2l(1+ging.groups.size()) - log2l(N))
	=  (x2 * (log2l(x2)-log2l(N))  + (N-x2)  * (log2l(N-x2)-log2l(N))  )
		-(x  * (log2l(x) -log2l(N))  + (N-x )  * (log2l(N-x )-log2l(N))  )
	=  (x2 * (log2l(x2)         )  + (N-x2)  * (log2l(N-x2)         )  )
		-(x  * (log2l(x)          )  + (N-x )  * (log2l(N-x )         )  )
	=   x2 *  log2l(x2)            + (N-x2)  *  log2l(N-x2)
		  -x *  log2l(x)             - (N-x )  *  log2l(N-x )
*/

void addEdge(V, int sharedCommunities) {
            // n is connected to _v // TODO: might be quicker to pass in the count of sharedCommunities too
            //assert(_ging._g.are_connected(_v, n)); // TODO: remove these assertions
            //assert(_grp.vs.count(n) == 1);
            //assert(_grp.vs.count(_v) == 0);
            count_edges_back_into_this_group++;
            _deltadeltaEdgeEntropy +=
                    log2l(1.0L - (1.0L - _ging._p_out) * powl(1.0L - _ging._p_in, 1 + sharedCommunities))
                    - log2l(1.0L - (1.0L - _ging._p_out) * powl(1.0L - _ging._p_in, sharedCommunities));
        }

```