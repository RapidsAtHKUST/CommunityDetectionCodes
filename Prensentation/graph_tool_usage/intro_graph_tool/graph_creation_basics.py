# https://graph-tool.skewed.de/static/doc/quickstart.html#property-maps
from __future__ import division, absolute_import, print_function
from itertools import izip
from numpy.random import randint
from graph_tool.all import *
import sys

if sys.version_info < (3,):
    range = xrange
import os
from pylab import *  # for plotting
from numpy.random import *  # for random sampling


def create_graph():
    g = Graph()
    g.add_vertex(100)
    # insert some random links
    for s, t in izip(randint(0, 100, 100), randint(0, 100, 100)):
        g.add_edge(g.vertex(s), g.vertex(t))

    vprop_double = g.new_vertex_property("double")  # Double-precision floating point
    vprop_double[g.vertex(10)] = 3.1416

    vprop_vint = g.new_vertex_property("vector<int>")  # Vector of ints
    vprop_vint[g.vertex(40)] = [1, 3, 42, 54]

    eprop_dict = g.new_edge_property("object")  # Arbitrary python object.
    eprop_dict[g.edges().next()] = {"foo": "bar", "gnu": 42}  # In this case, a dict.

    gprop_bool = g.new_graph_property("bool")  # Boolean
    gprop_bool[g] = True
    return g


def iterate_graph():
    graph0 = create_graph()
    for v in graph0.vertices():
        print(v)
    for e in graph0.edges():
        print(e)
    for v in graph0.vertices():
        for e in v.out_edges():
            print(e),
        print()
        for w in v.out_neighbours():
            print(w)

    # efficient array-based operation, not use python loop
    edges = graph0.get_edges()
    print((edges[:, 0] * edges[:, 1]).sum())


def build_price_network():
    # We start with an empty, directed graph
    g = Graph()

    # We want also to keep the age information for each vertex and edge. For that
    # let's create some property maps
    v_age = g.new_vertex_property("int")
    e_age = g.new_edge_property("int")

    # The final size of the network
    N = 100000

    # We have to start with one vertex
    v = g.add_vertex()
    v_age[v] = 0

    # we will keep a list of the vertices. The number of times a vertex is in this
    # list will give the probability of it being selected.
    vlist = [v]

    # let's now add the new edges and vertices
    for i in range(1, N):
        # create our new vertex
        v = g.add_vertex()
        v_age[v] = i

        # we need to sample a new vertex to be the target, based on its in-degree +
        # 1. For that, we simply randomly sample it from vlist.
        i = randint(0, len(vlist))
        target = vlist[i]

        # add edge
        e = g.add_edge(v, target)
        e_age[e] = i

        # put v and target in the list
        vlist.append(target)
        vlist.append(v)

    # now we have a graph!

    # let's do a random walk on the graph and print the age of the vertices we find,
    # just for fun.

    v = g.vertex(randint(0, g.num_vertices()))
    while True:
        print("vertex:", int(v), "in-degree:", v.in_degree(), "out-degree:",
              v.out_degree(), "age:", v_age[v])

        if v.out_degree() == 0:
            print("Nowhere else to go... We found the main hub!")
            break

        n_list = []
        for w in v.out_neighbours():
            n_list.append(w)
        v = n_list[randint(0, len(n_list))]

    # let's save our graph for posterity. We want to save the age properties as
    # well... To do this, they must become "internal" properties:

    g.vertex_properties["age"] = v_age
    g.edge_properties["age"] = e_age

    # now we can save it
    g.save("price.xml.gz")

    # Let's plot its in-degree distribution
    in_hist = vertex_hist(g, "in")

    y = in_hist[0]
    err = sqrt(in_hist[0])
    err[err >= y] = y[err >= y] - 1e-2

    figure(figsize=(6, 4))
    errorbar(in_hist[1][:-1], in_hist[0], fmt="o", yerr=err,
             label="in")
    gca().set_yscale("log")
    gca().set_xscale("log")
    gca().set_ylim(1e-1, 1e5)
    gca().set_xlim(0.8, 1e3)
    subplots_adjust(left=0.2, bottom=0.2)
    xlabel("$k_{in}$")
    ylabel("$NP(k_{in})$")
    tight_layout()
    savefig("price-deg-dist.pdf")
    savefig("price-deg-dist.png")


if __name__ == '__main__':
    iterate_graph()
    build_price_network()
