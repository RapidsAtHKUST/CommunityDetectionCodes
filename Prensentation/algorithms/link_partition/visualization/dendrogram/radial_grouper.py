"""All grouping functionality is collected here."""
from collections import namedtuple
from numpy import r_, arange, argsort, array, ones, pi, where
from numpy import logical_and as land
from radial_support import from_polar

__all__= ['tree', 'pre_order', 'post_order']

Node= namedtuple('Node', 'ndx lnk')

# ToDo: enhance documentation
def _groub_by(p, tol, r):
    g, gm, gp= [], [], p- p[0]
    while True:
        if gp[-1]< 0: break
        ndx= where(land(0.<= gp, gp< tol))[0]
        if 0< len(ndx):
            g.append(ndx)
            gm.append(p[ndx].mean())
        gp-= tol
    return g, array([gm, [r]* len(gm)])

def _leafs(p):
    return argsort(p[0])

def _create_leaf_nodes(ndx):
    nodes= []
    for k in xrange(len(ndx)):
        nodes.append(Node(ndx[k], []))
    return nodes

def _link_and_create_nodes(_n, n_, cn, groups):
    nodes, n0= [], 0
    for k in xrange(len(groups)):
        nodes.append(Node(n_+ n0, [cn[m] for m in groups[k]]))
        n0+= 1
    return n_, n_+ n0, nodes

def _process_level(nodes, polar, p, tol, scale, _n, n_):
    groups, p= _groub_by(p, tol, scale* polar[1, _n])
    _n, n_, nodes= _link_and_create_nodes(_n, n_, nodes, groups)
    polar[:, _n: n_]= p
    return nodes, polar, _n, n_

def _create_tree(p, r0, scale, tols):
    if None is tols:
        tols= .3* pi/ 2** arange(5)[::-1]
    _n, n_= 0, p.shape[1]
    polar= ones((2, (len(tols)+ 2)* n_))
    polar[0, :n_], polar[1, :n_]= p[0], r0
    # leafs
    nodes= _create_leaf_nodes(_leafs(p))
    nodes, polar, _n, n_= _process_level(
    nodes, polar, polar[0, _leafs(p)], tols[0], scale, _n, n_)
    # links
    for tol in tols[1:]:
        nodes, polar, _n, n_= _process_level(
        nodes, polar, polar[0, _n: n_], tol, scale, _n, n_)
    # root
    polar[:, n_]= [0., 0.]
    return Node(n_, nodes), polar[:, :n_+ 1]

def _simplify(self):
    # ToDo: combine single linkages
    return self._root

def _call(self, node0, node1, f, level):
    f(self, [node0.ndx, node1.ndx], level)

def pre_order(self, node0, f, level= 0):
    for node1 in node0.lnk:
        _call(self, node0, node1, f, level)
        pre_order(self, node1, f, level+ 1)

def post_order(self, node0, f, level= 0):
    for node1 in node0.lnk:
        post_order(self, node1, f, level+ 1)
        _call(self, node0, node1, f, level)

class tree(object):
    def __init__(self, p, r0= pi, scale= .9, tols= None):
        self._n= p.shape[1]
        self._root, self._p= _create_tree(p, r0, scale, tols)

    def traverse(self, f, order= pre_order, cs= 'Cartesian'):
        self.points= self._p
        if cs is 'Cartesian':
            self.points= from_polar(self._p)
        order(self, self._root, f, 0)
        return self

    def simplify(self):
        self._root= _simplify(self)
        return self

    def is_root(self, ndx):
        return ndx== self._p.shape[1]- 1

    def is_leaf(self, ndx):
        return ndx< self._n

if __name__ == '__main__':
    # ToDO: add tests
    from numpy import r_, round
    from numpy.random import rand
    from pylab import plot, show

    def _l(t, n, l):
        # print round(a, 3), n, l, t.is_root(n[0]), t.is_leaf(n[1])
        plot(t.points[0, n], t.points[1, n])
        if 0== l:
            plot(t.points[0, n[0]], t.points[1, n[0]], 's')
        if t.is_leaf(n[1]):
            plot(t.points[0, n[1]], t.points[1, n[1]], 'o')

    n= 123
    p= r_[2* pi* rand(1, n)- pi, ones((1, n))]
    t= tree(p).simplify().traverse(_l)
    # t= tree(p).traverse(_l, cs= 'Polar')
    show()
    # print
    # t.traverse(_l, post_order, cs= 'Polar')
