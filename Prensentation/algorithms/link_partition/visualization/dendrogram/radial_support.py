"""All supporting functionality is collected here."""
from numpy import r_, arctan2, cos, sin
from numpy import atleast_2d as a2d

# ToDo: create proper documentation strings
def _a(a0, a1):
    return r_[a2d(a0), a2d(a1)]

def from_polar(p):
    """(theta, radius) to (x, y)."""
    return _a(cos(p[0])* p[1], sin(p[0])* p[1])

def to_polar(c):
    """(x, y) to (theta, radius)."""
    return _a(arctan2(c[1], c[0]), (c** 2).sum(0)** .5)

def d_to_polar(D):
    """Distance matrix to (theta, radius)."""
    # this functionality is to adopt for more general situations
    # intended functionality:
    # - embedd distance matrix to 2D
    # - return that embedding in polar coordinates
    pass

if __name__ == '__main__':
    from numpy import allclose
    from numpy.random import randn
    c= randn(2, 5)
    assert(allclose(c, from_polar(to_polar(c))))

    # ToDO: implement more tests
