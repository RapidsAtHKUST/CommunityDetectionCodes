# Graph Notations
## Property Map

e.g, usage as follows

```python
from itertools import izip
from numpy.random import randint
import graph_tool as gt

g = gt.Graph()
g.add_vertex(100)
# insert some random links
for s,t in izip(randint(0, 100, 100), randint(0, 100, 100)):
    g.add_edge(g.vertex(s), g.vertex(t))

vprop_double = g.new_vertex_property("double")            # Double-precision floating point
vprop_double[g.vertex(10)] = 3.1416

vprop_vint = g.new_vertex_property("vector<int>")         # Vector of ints
vprop_vint[g.vertex(40)] = [1, 3, 42, 54]

eprop_dict = g.new_edge_property("object")                # Arbitrary python object.
eprop_dict[g.edges().next()] = {"foo": "bar", "gnu": 42}  # In this case, a dict.

gprop_bool = g.new_graph_property("bool")                  # Boolean
gprop_bool[g] = True
```

internal property to store things inside the graph data structure

```python
eprop = g.new_edge_property("string")
g.edge_properties["some name"] = eprop
g.list_properties()
```

for convenience, the internal property maps can also be accessed via attributes:

```python
vprop = g.new_vertex_property("double")
g.vp.foo = vprop                        # equivalent to g.vertex_properties["foo"] = vprop
v = g.vertex(0)
g.vp.foo[v] = 3.14
```