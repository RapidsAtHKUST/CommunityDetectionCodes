#Attention
##Current Status

Build successfully.

Fix interface-changing-bug due to change of igraph library. In [communities.cpp](communities.cpp) line 136.

- previous

```cpp
 err = igraph_neighborhood(graph, &p, *vs, 1, IGRAPH_ALL);
```

- current

```cpp
 err = igraph_neighborhood(graph, &p, *vs, 1, IGRAPH_ALL, 1);
```