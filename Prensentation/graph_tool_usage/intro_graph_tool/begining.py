from graph_tool.all import *
import graph_tool.all as gt
import matplotlib.pyplot as plt


def first_example():
    g = Graph()
    v1 = g.add_vertex()
    v2 = g.add_vertex()
    e = g.add_edge(v1, v2)
    graph_draw(g, vertex_text=g.vertex_index, vertex_font_size=18, output_size=(200, 200), output="two-nodes.png")


def draw_football():
    g = gt.collection.data["football"]
    print g.list_properties()

    state = gt.minimize_blockmodel_dl(g, deg_corr=False)
    state.draw(pos=g.vp.pos, output="football-sbm-fit.png")

    b = state.get_blocks()
    r = b[10]  # group membership of vertex 10
    print(r)
    e = state.get_matrix()

    plt.matshow(e.todense())
    plt.savefig("football-edge-counts.png")


pv = None


def draw_lesmis():
    g = gt.collection.data["lesmis"]

    state = gt.BlockState(g, B=20)  # This automatically initializes the state
    # with a random partition into B=20
    # nonempty groups; The user could
    # also pass an arbitrary initial
    # partition using the 'b' parameter.

    # If we work with the above state object, we will be restricted to
    # partitions into at most B=20 groups. But since we want to consider
    # an arbitrary number of groups in the range [1, N], we transform it
    # into a state with B=N groups (where N-20 will be empty).

    state = state.copy(B=g.num_vertices())

    # Now we run 1,000 sweeps of the MCMC

    dS, nmoves = state.mcmc_sweep(niter=1000)

    print("Change in description length:", dS)
    print("Number of accepted vertex moves:", nmoves)

    gt.mcmc_equilibrate(state, wait=1000, mcmc_args=dict(niter=10))

    def collect_marginals(s):
        global pv
        pv = s.collect_vertex_marginals(pv)

    # Now we collect the marginals for exactly 100,000 sweeps
    gt.mcmc_equilibrate(state, force_niter=10000, mcmc_args=dict(niter=10),
                        callback=collect_marginals)

    # Now the node marginals are stored in property map pv. We can
    # visualize them as pie charts on the nodes:
    state.draw(pos=g.vp.pos, vertex_shape="pie", vertex_pie_fractions=pv,
               edge_gradient=None, output="lesmis-sbm-marginals.png")


if __name__ == '__main__':
    first_example()
    draw_football()
    draw_lesmis()
