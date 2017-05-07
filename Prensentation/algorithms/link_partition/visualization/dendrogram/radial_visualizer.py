"""All visualization functionality is collected here."""
from pylab import plot
    
# ToDo: create proper documentation
def simple_link(t, ndx, level):
    """Simple_link is just a minimal example to demonstrate what can be
    achieved when it's called from _grouper.tree.traverse for each link.
    - t, tree instance
    - ndx, a pair of (from, to) indicies
    - level, of from, i.e. root is in level 0
    """
    plot(t.points[0, ndx], t.points[1, ndx])
    if 0== level:
        plot(t.points[0, ndx[0]], t.points[1, ndx[0]], 's')
    if t.is_leaf(ndx[1]):
        plot(t.points[0, ndx[1]], t.points[1, ndx[1]], 'o')

# ToDO: implement more suitable link visualizers
# No doubt, this will the part to burn most of the dev. resources

if __name__ == '__main__':
    # ToDO: implement tests
    pass
