"""
An agglomerative k-clique percolation algorithm.
This script is divided to parts as follows:
*networking framework: general datastructures for graphs
*extra functions for networks: also other helpful functions and classes
*percolation framework: general classes for edge percolation
*k-clique percolator: implementation of the sequential k-clique algorithm
*main program: parsing arguments and using above classes etc.

See http://arxiv.org/abs/0805.1449 for an explanation of the algorithm.

Author: Mikko Kivela (mtkivela at lce.hut.fi)
Jorkki Hyvonen is the author of the classes Net, Node and SymmNet
"""
import sys, array, math
from operator import mul


# ---- A networking framework -----

class Net(object):
    """A sparse matrix"""

    def __init__(self):
        self._nodes = {}

    def _legaledge(self, src, dst):
        # Override this for symmetrical/two-way linked ones
        return True

    def _intsetedge(self, src, dst, val):
        # Override this for symmetrical/two-way linked ones
        if val == 0:
            if src in self._nodes:
                del self._nodes[src][dst]
                if len(self._nodes[src]) == 0:
                    # delete isolated node
                    del self._nodes[src]
        else:
            if not src in self._nodes:
                self._nodes[src] = {dst: val}
            else:
                self._nodes[src][dst] = val

    def _setedge(self, src, dst, val):
        assert self._legaledge(src, dst)
        self._intsetedge(src, dst, val)
        assert self._legaledge(src, dst)

    def __getitem__(self, args):
        if isinstance(args, tuple):
            if len(args) != 2:
                raise KeyError, "Don't be silly. One or two indices"
            assert self._legaledge(args[0], args[1])
            try:
                retval = self._nodes[args[0]][args[1]]
            except KeyError:
                return 0
            return retval
        else:
            return Node(self, args)

    def __setitem__(self, key, val):
        if isinstance(key, tuple):
            if len(key) != 2:
                raise KeyError, "Don't be silly. One or two args"
            self._setedge(key[0], key[1], val)
            return val
        else:
            if isinstance(val, Node):
                if val.index in val.net._nodes:
                    val = val.net._nodes[val.index]
                else:
                    val = {}
            if not isinstance(val, dict):
                raise NotImplemented, \
                    "Setting nodes only implemented from maps"
            del self[key]  # calls __delitem__
            # We perform a deep copy
            for edge in val.iteritems():
                self._setedge(key, edge[0], edge[1])
            return Node(self, key)

    def __delitem__(self, args):
        if isinstance(args, tuple):
            if len(args) != 2:
                raise KeyError, "Don't be silly. One or two indices"
            self._setedge(args[0], args[1], 0)
        else:
            for dest in self[args]:
                # self[args] returns a node, for which .iter()
                self[args, dest] = 0

    def __iter__(self):
        return self._nodes.keys().__iter__()

    def isSymmetric(self):
        return False

    def __len__(self):
        return len(self._nodes)


class Node(object):
    def __init__(self, net, index):
        self.net = net;
        self.index = index;

    def __iter__(self):
        if not self.index in self.net._nodes:
            return iter([])
        return self.net._nodes[self.index].keys().__iter__()
        # return self.net._nodes[self.index].__iter__()

    def __getitem__(self, index):
        return self.net[self.index, index]

    def __setitem__(self, index, val):
        self.net[self.index, index] = val
        return val

    def deg(self):
        return len(self.net._nodes[self.index])


class SymmNet(Net):
    """A net with forced symmetry"""

    def _legaledge(self, src, dst):
        if src in self._nodes:
            if dst in self._nodes[src]:
                return dst in self._nodes \
                       and src in self._nodes[dst] \
                       and self._nodes[src][dst] == self._nodes[dst][src]
        # either no src or no edge src->dst
        return dst not in self._nodes or src not in self._nodes[dst]

    def _intsetedge(self, src, dst, val):
        Net._intsetedge(self, src, dst, val)
        Net._intsetedge(self, dst, src, val)
        return val

    def isSymmetric(self):
        return True


# ----- Extra functions for general networking framework -----
class Net_edges:
    def __init__(self, net):
        self.net = net

    def __iter__(self):
        for node1Index in self.net:
            node1 = self.net[node1Index]
            for node2Index in node1:
                if (not self.net.isSymmetric()) or node1Index.__hash__() < node2Index.__hash__():
                    yield [node1Index, node2Index, self.net[node1Index, node2Index]]

    def __len__(self):
        lenght = 0
        for nodeIndex in self.net:
            lenght += len(self.net[nodeIndex].edges)
        if self.net.isSymmetric():
            return lenght / 2
        else:
            return lenght

    def __str__(self):
        return str(list(self))


Net.edges = property(Net_edges)


def getSubnet(net, nodes):
    newNet = Net()
    degsum = 0
    for node in nodes:
        degsum += net[node].deg()
    if degsum >= len(nodes) * (len(nodes) - 1) / 2:
        othernodes = set(nodes)
        for node in nodes:
            if net.isSymmetric():
                othernodes.remove(node)
            for othernode in othernodes:
                if net[node, othernode] != 0:
                    newNet[node, othernode] = net[node, othernode]
    else:
        for node in nodes:
            for neigh in net[node]:
                if neigh in nodes:
                    newNet[node, neigh] = net[node, neigh]
    return newNet


def loadNet_edg(input, mutualEdges=False, splitterChar=None, symmetricNet=True):
    """
    Reads a network data from input in edg format.

    If mutualEdges is set to True, an edge is added between nodes i
    and j only if both edges (i,j) and (j,i) are listed. The weight of
    the edge is the average of the weights of the original edges.
    """

    def isNumerical(input):
        try:
            for line in input:
                int(line.split(splitterChar)[0])
                int(line.split(splitterChar)[1])
        except ValueError:
            input.seek(0)
            return False
        input.seek(0)
        return True

    numerical = isNumerical(input)

    if symmetricNet:
        newNet = SymmNet()
    else:
        newNet = Net()

    nodeMap = {}  # Used only if mutualEdges = True.

    for line in input:
        fields = line.split(splitterChar)
        if len(fields) > 2:
            if numerical:
                fields[0] = int(fields[0])
                fields[1] = int(fields[1])
            if fields[0] != fields[1]:
                if mutualEdges:
                    if nodeMap.has_key((fields[1], fields[0])):
                        value = 0.5 * (nodeMap[(fields[1], fields[0])] + float(fields[2]))
                        newNet[fields[0]][fields[1]] = value
                    else:
                        nodeMap[(fields[0], fields[1])] = float(fields[2])
                else:
                    newNet[fields[0]][fields[1]] = float(fields[2])

    return newNet


class Enumerator:
    """
    Finds enumeration for hashable items. For new items a new number is
    made up and if the item already has a number it is returned instead
    of a new one.
    >>> e=Enumerator()
    >>> e['foo']
    0
    >>> e['bar']
    1
    >>> e['foo']
    0
    >>> list(e)
    ['foo', 'bar']
    """

    def __init__(self):
        self.number = {}
        self.item = []

    def _addItem(self, item):
        newNumber = len(self.number)
        self.number[item] = newNumber
        self.item.append(item)
        return newNumber

    def __getitem__(self, item):
        try:
            return self.number[item]
        except KeyError:
            return self._addItem(item)

    def getReverse(self, number):
        return self.item[number]

    def __iter__(self):
        return self.number.__iter__()

    def __len__(self):
        return len(self.number)


class NodeFamily:
    """
    Defines a community structure of a network.
    """

    def __init__(self, cmap={}, inputFile=None):
        self.comm = []
        for community in cmap:
            self._addCommunity(cmap[community])
        if inputFile != None:
            self._parseStrings(inputFile)
        self._sortBySize()

    def _parseStrings(self, input):
        for line in input:
            fields = line.split()
            fields = map(int, fields)
            self._addCommunity(fields)

    def __str__(self):
        string = ""
        for community in self.comm:
            for node in community:
                string = string + str(node) + " "
            string = string[0:len(string) - 1] + "\n"
        return string

    def _sortBySize(self):
        self.comm.sort(lambda x, y: cmp(len(x), len(y)), reverse=True)

    def _addCommunity(self, newCommunity):
        self.comm.append(set(newCommunity))

    def __getitem__(self, index):
        return self.comm[index]

    def __len__(self):
        return len(self.comm)

    def getSizeDist(self):
        """
        Returns a map of size distribution. Keys are the sizes of the communities
        and their values are the number of communities of the size.
        """
        dist = {}
        for set in self.comm:
            if len(set) in dist:
                dist[len(set)] += 1
            else:
                dist[len(set)] = 1
        return dist

    def getGiant(self):
        """
        Returns the largest component as a set of nodes
        """
        maxsize = 0
        largest = None
        for community in self.comm:
            if len(community) > maxsize:
                maxsize = len(community)
                largest = community

        return largest

    def getGiantSize(self):
        """
        Returns the size of the largest component
        """
        giant = self.getGiant()
        if giant != None:
            return len(giant)
        else:
            return 0

    def getSusceptibility(self, size=None):
        """
        Returns the susceptibility defined as:
        (Sum_{s!=size(gc)} n_s * s * s) / (Sum_{s!=size(gc)} n_s * s)
        Size is the number of nodes in the network. If it is given, it is assumed
        that communities of size 1 are not included in this community structure.
        If there is only 0 or 1 community, zero is returned.
        """
        sd = self.getSizeDist()

        if len(sd) < 1:
            if size == None or size == 0:
                return 0.0
            else:
                return 1.0

        sizeSum = 0
        for key in sd.keys():
            sizeSum += key * sd[key]

        # If no size is given, assume that also communities of size 1 are included
        if size == None:
            sus = 0
            size = sizeSum
        else:
            sus = size - sizeSum  # s=1
            assert (sus >= 0)

        # Remove largest component
        gc = max(sd.keys())
        sd[gc] = 0

        # Calculate the susceptibility
        for key in sd.keys():
            sus += key * key * sd[key]
        if (size - gc) == 0:
            return 0.0
        else:
            return float(sus) / float(size - gc)

    def getCollapsed(self):
        """

        """
        newcs = NodeFamily({})
        for community in self.comm:
            newCommunity = set()
            for oldnode in community:
                for newnode in oldnode:
                    newCommunity.add(newnode)

            newCommunityArray = []
            for node in newCommunity:
                newCommunityArray.append(node)
            newcs._addCommunity(newCommunityArray)

        newcs._sortBySize()
        return newcs

    def getNew(self, newNodes):
        """
        Returns new community structure based on this one and
        new nodes denoted by indices on this one
        """
        newcs = NodeFamily({})
        for community in self.comm:
            newCommunity = set()
            for oldnode in community:
                for newnode in newNodes[oldnode]:
                    newCommunity.add(newnode)

            newCommunityArray = []
            for node in newCommunity:
                newCommunityArray.append(node)
            newcs._addCommunity(newCommunityArray)

        newcs._sortBySize()
        return newcs

    def getSetsForNodes(self):
        """
        Returns a map of nodes to the set it belongs.
        Sets are denoted by integers so that largest
        community has smallest number etc.
        """
        nodeCommunity = {}
        for i, c in enumerate(self.comm):
            for node in c:
                nodeCommunity[node] = i
        return nodeCommunity


# ---- General percolator related classes ----

class KtreeInteger:
    """
    Implementation of the disjoint-set forest for integers
    """

    def __init__(self, size=0):
        self.ktree = []
        self.mappingOn = False
        if size != 0:
            for index in range(0, size + 1):
                self.ktree.append(index);

    def __getRealParent(self, node):
        """
        Private method. Reads elements directly from the tree.
        """
        try:
            return self.ktree[node]
        except IndexError:
            self.setSize(node)
            return node

    def __setRealParent(self, node, newParent):
        """
        Private.
        """
        try:
            self.ktree[node] = newParent
        except IndexError:
            self.setSize(node)
            self.ktree[node] = newParent

    def getParent(self, node):
        parent = self.__getRealParent(node)
        if node != parent:
            self.__setRealParent(node, self.getParent(parent))
        return self.__getRealParent(node)

    def setParent(self, node, newParent):
        self.__setRealParent(self.getParent(node), self.getParent(newParent))

    def __iter__(self):
        for i in self.ktree:
            yield i

    def getCommStruct(self, separateElements=True):
        communityMap = {}
        if self.mappingOn:
            nodes = self.ktree
        else:
            nodes = range(0, len(self.ktree))

        for node in nodes:
            communityKey = self.getParent(node)
            if separateElements or communityKey != node:
                if communityKey not in communityMap:
                    communityMap[communityKey] = [node]
                else:
                    communityMap[communityKey].append(node)

        return NodeFamily(communityMap)

    def __len__(self):
        return len(self.ktree)

    def mergeSetsWithElements(self, elements):
        first = elements[0]
        for i in range(1, len(elements)):
            self.setParent(elements[i], first)

    def addEdge(self, edge):
        self.setParent(edge[0], edge[1])

    def setSize(self, newSize):
        for index in range(len(self.ktree), newSize + 1):
            self.ktree.append(index);


class Ktree(KtreeInteger):
    """
    Disjoint-set forest with mapping frontend. This means that node names can be any
    hashable objects.
    """

    def __init__(self, size=0):
        self.ktree = KtreeInteger(size)
        self.nodeIndex = Enumerator()
        self.mappingOn = True

    def getParent(self, node):
        return self.nodeIndex.getReverse(self.ktree.getParent(self.nodeIndex[node]))

    def setParent(self, node, newParent):
        self.ktree.setParent(self.nodeIndex[node], self.nodeIndex[newParent])

    def __iter__(self):
        return self.nodeIndex.__iter__()

    def getCommStruct(self):
        cs = self.ktree.getCommStruct()
        newcs = NodeFamily()
        for c in cs:
            newc = []
            for node in c:
                newc.append(self.nodeIndex.getReverse(node))
            newcs.comm.append(newc)
        return newcs


class EvaluationList:
    """
    EvaluationList object is an iterable object that iterates through a list returning
    EvaluationEvent objects according to given rules.
    """

    def __init__(self, thelist, weightFunction=lambda x: x[2]):
        self.thelist = thelist
        self.weightFunction = weightFunction
        self.strengthEvaluations = False
        self.evaluationPoints = []
        self.lastEvaluation = False

    def setEvaluations(self, evaluationPoints):
        self.evaluationPoints = evaluationPoints

    def setLinearEvaluations(self, first, last, numberOfEvaluationPoints):
        self.strenghtEvaluations = False
        if last <= first:
            raise Exception("last<=first")
        if numberOfEvaluationPoints < 2:
            raise Exception("Need 2 or more evaluation points")
        last = last - 1
        self.setEvaluations(map(lambda x: int(first + (last - first) * x / float(numberOfEvaluationPoints - 1)),
                                range(0, numberOfEvaluationPoints)))
        self.lastEvaluation = False

    def setStrengthEvaluations(self):
        self.strengthEvaluations = True
        self.lastEvaluation = False

    def setLastEvaluation(self):
        self.lastEvaluation = True

    def __iter__(self):
        if not self.strengthEvaluations and not self.lastEvaluation:
            index = 0
            evalIter = self.evaluationPoints.__iter__()
            nextEvaluationPoint = evalIter.next()
            for element in self.thelist:
                yield element
                if index == nextEvaluationPoint:
                    yield EvaluationEvent(self.weightFunction(element), index + 1)
                    nextEvaluationPoint = evalIter.next()
                index += 1
        elif not self.lastEvaluation:
            last = None
            numberOfElements = 0
            for element in self.thelist:
                numberOfElements += 1
                if last != self.weightFunction(element) and last != None:
                    yield EvaluationEvent(last, numberOfElements - 1)
                last = self.weightFunction(element)
                yield element
            yield EvaluationEvent(last, numberOfElements)

        else:
            for element in self.thelist:
                yield element
            yield EvaluationEvent()


# ---- K-clique percolator ----

def getKCliqueComponents(net, k):
    """
    Returns community structure calculated with k-clique percolation.
    """

    def evaluateAtEnd(edges):
        for edge in edges:
            yield edge
        yield EvaluationEvent()

    edgesAndEvaluations = evaluateAtEnd(net.edges)

    kcliques = kcliquesByEdges(edgesAndEvaluations, k)  # unweighted clique percolation
    for community in communitiesByKCliques(kcliques):
        return community


class KClique(object):
    """
    A class for presenting cliques of size k. Realizations
    of this class just hold a sorted list of nodes in the clique.
    """

    def __init__(self, nodelist, notSorted=True):
        self.nodes = nodelist
        if notSorted:
            self.nodes.sort()
        self.hash = None

    def __hash__(self):
        if self.hash == None:
            self.hash = hash(reduce(mul, map(self.nodes[0].__class__.__hash__, self.nodes)))
        return self.hash

    def __iter__(self):
        for node in self.nodes:
            yield node

    def __cmp__(self, kclique):
        if self.nodes == kclique.nodes:
            return 0
        else:
            return 1

    def __add__(self, kclique):
        return KClique(self.nodes + kclique.nodes)

    def getSubcliques(self):
        for i in range(0, len(self.nodes)):
            yield KClique(self.nodes[:i] + self.nodes[(i + 1):], notSorted=False)

    def __str__(self):
        return str(self.nodes)

    def getEdges(self):
        for node in self.nodes:
            for othernode in self.nodes:
                if node != othernode:
                    yield (node, othernode)

    def getK(self):
        return len(self.nodes)


def getIntensity(kclique, net):
    intensity = 1
    for edge in kclique.getEdges():
        intensity *= net[edge[0], edge[1]]
    return pow(intensity, 1.0 / float(kclique.getK()))


class EvaluationEvent:
    def __init__(self, threshold=None, addedElements=None):
        self.threshold = threshold
        self.addedElements = addedElements


def kcliquesAtSubnet(nodes, net, k):
    """
    List all k-cliques at a given network. Any implementation is fine,
    but as this routine is a part of a clique percolator anyway we
    will use itself to find cliques larger than 2. Cliques of size 1 and
    2 are trivial.
    """
    if len(nodes) >= k:
        if k == 1:
            for node in nodes:
                yield KClique([node])
        elif k == 2:
            subnet = getSubnet(net, nodes)
            for edge in subnet.edges:
                yield KClique([edge[0], edge[1]])
        else:
            subnet = getSubnet(net, nodes)
            for kclique in kcliquesByEdges(subnet.edges, k):
                yield kclique


def kcliquesByEdges(edges, k):
    """
    Phase I in the SCP-algorithm.

    Generator function that generates a list of cliques of size k in the order they
    are formed when edges are added in the order defined by the 'edges' argument.
    If many cliques is formed by adding one edge, the order of the cliques is
    arbitrary.
    This generator will pass through any EvaluationEvent objects that are passed to
    it in the 'edges' generator.
    """
    newNet = SymmNet()  # Edges are added to a empty network one by one
    for edge in edges:
        if isinstance(edge, EvaluationEvent):
            yield edge
        else:
            # First we find all new triangles that are born when the new edge is added
            triangleEnds = set()  # We keep track of the tip nodes of the new triangles
            for adjacendNode in newNet[edge[0]]:  # Neighbor of one node of the edge ...
                if newNet[adjacendNode, edge[1]] != 0:  # ...is a neighbor of the other node of the edge...
                    triangleEnds.add(adjacendNode)  # ...then the neighbor is a tip of a new triangle

            # New k-cliques are now (k-2)-cliques at the triangle end points plus
            # the two nodes at the tips of the edge we are adding to the network
            for kclique in kcliquesAtSubnet(triangleEnds, newNet, k - 2):
                yield kclique + KClique([edge[0], edge[1]])

            newNet[edge[0], edge[1]] = edge[2]  # Finally we add the new edge to the network


def kcliquesWeight(net, k, weightFunction):
    kcliques = list(kcliquesByEdges(net.edges, k))
    kcliques.sort(lambda x, y: cmp(weightFunction(x, net), weightFunction(y, net)))
    for kclique in kcliques:
        yield kclique


def communitiesByKCliques(kcliques):
    """
    Phase II in the SCP algorithm. Finds communities in the order they
    appear as the cliques are added to the network.
    """
    # Calculate the neighboring relations
    krTree = Ktree()
    for kclique in kcliques:
        if isinstance(kclique, EvaluationEvent):
            communityStructure = krTree.getCommStruct().getCollapsed()
            communityStructure.threshold = kclique.threshold
            yield communityStructure
        else:
            # for fewer operations at ktree, names of new cliques should be saved
            # and given to ktree when merging the sets
            krcliques = list(kclique.getSubcliques())  # list all k-1 cliques that are subcliques
            krTree.mergeSetsWithElements(krcliques)  # merge the sets of k-1 cliques at the list


def kcliquePercolator(net, k, start, stop, evaluations, reverse=False, weightFunction=None):
    """
    K-clique percolator. This sorts the edges and combines the phases I-II. See
    helpstring below for explanation of the arguments.
    """
    if weightFunction == None:  # unweighted clique percolation with thresholding
        edges = list(net.edges)
        edges.sort(lambda x, y: cmp(x[2], y[2]), reverse=reverse)
        edgesAndEvaluations = EvaluationList(edges)
        edgesAndEvaluations.setLinearEvaluations(start, stop, evaluations)
        kcliques = kcliquesByEdges(edgesAndEvaluations, k)
    else:  # weighted clique percolation
        kcliques = EvaluationList(kcliquesWeight(net, k, weightFunction), weightFunction=lambda x: getIntensity(x, net))
        kcliques.setLinearEvaluations(start, stop, evaluations)

    for community in communitiesByKCliques(kcliques):
        yield community


# ---- Main program and parsing arguments ----

helpstring = "Incremental k-clique percolation algorithm.\n"
helpstring += "Usage: python kclique.py netname k [start end numberofevaluations] [weight]\n"
helpstring += "If only net name and k is specified, the components are returned. If start end and"
helpstring += " number of evaluation are specified the community structure will be evaluated many times."
helpstring += " The evaluations are made linearly between start and end. If no weigh is defined, the evaluations"
helpstring += " are made with respect to edge weights and if intensity is specified as the weight, weighted k-clique "
helpstring += "percolation is used and the evaluation are made with respect to cliques.\n"
helpstring += "Example: python kclique.py mynet.edg 5 1000 5000 5 intensty\n"
helpstring += "This example returns nodes in 5-clique communities when 1000, 2000, 3000, 4000 and 5000 first 5-cliques are"
helpstring += " added to the network after sorting them with respect to intensity.\n"
helpstring += "Output is given as a list of nodes separated by space and communities separated by line change."

if len(sys.argv) > 2:
    filename = sys.argv[1]
    k = int(sys.argv[2])
    f = open(filename, 'r')
    net = loadNet_edg(f)
if len(sys.argv) > 5:
    start, stop, evaluations = int(sys.argv[3]), int(sys.argv[4]), int(sys.argv[5])
    weightFunction = None
if len(sys.argv) == 7:
    if sys.argv[6] == "intensity":
        weightFunction = getIntensity

if len(sys.argv) == 3:
    cs = getKCliqueComponents(net, k)
    print cs
elif len(sys.argv) == 6 or len(sys.argv) == 7:
    for cs in kcliquePercolator(net, k, start, stop, evaluations, weightFunction=weightFunction):
        print "At threshold: " + str(cs.threshold)
        print cs
else:
    print helpstring
