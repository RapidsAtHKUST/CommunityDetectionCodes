from numpy import r_, ones, pi, sort
from numpy.random import rand
from radial_grouper import tree, pre_order, post_order
from radial_visualizer import simple_link
from pylab import axis, figure, plot, subplot

# ToDo: create proper documentation
def _s(sp, t, o):
    subplot(sp)
    t.traverse(simple_link, order= o)
    axis('equal')
    
def demo1(n):
    p= r_[2* pi* rand(1, n)- pi, ones((1, n))]
    t= tree(p)
    f= figure()
    _s(221, t, pre_order)
    _s(222, t, post_order)
    t= tree(p, tols= sort(2e0* rand(9)))
    _s(223, t, pre_order)
    _s(224, t, post_order)
    f.show()
    # f.savefig('test.png')

# ToDO: implement more demos

if __name__ == '__main__':
    demo1(123)
