import igraph
import progressbar
import sys

if len(sys.argv) < 3:
  print "Synopsis: %s graph.edl output [gz|edl|gml] [verbose]" % sys.argv[0]
  sys.exit(-1)

format = "graphml"
if len(sys.argv) >= 4: 
  if (sys.argv[3] == "gz"): format = "graphmlz"
  elif (sys.argv[3] == "edl"): format = "edl"

if len(sys.argv) >= 5: verbose = sys.argv[4] == "verbose"
else:                  verbose = False;

# da asdot a intero
def asdot(AS):
  if "." in AS:
    a, b = AS.split(".")
    return str((2**16 * int(a)) + int(b))
  
  return AS

def select(x):
  return x[0]

class Converter:
  vertices = {}
  edges = set()
  namese = []
  
  """Return the ID of an AS node; the ID is created the first time."""
  def reg(self, v):
    try:
      return self.vertices[v][0]
    except KeyError:
      i = len(self.vertices)
      self.vertices[v] = i, v
      
      return self.vertices[v][0]
  
  def add(self, data, mv):
    sreg = self.reg
    edgesa = self.edges.add
    namesea = self.namese.append
    
    if verbose: 
      pbar = progressbar.ProgressBar(widgets = ["Parsing: ", progressbar.Percentage(), progressbar.Bar()], maxval = mv)
    else:
      pbar = progressbar.ProgressBar(widgets = [], maxval = mv)
    
    
    for y in pbar(data):
      I = asdot(y[0])
      J = asdot(y[1])
      
      if (int(I) > int(J)):
        auxval = I; I = J; J = auxval;
      
      a = sreg(I)
      b = sreg(J)
      
      edgesa((a, b, "%s\t%s" % (I, J)))
      #namesea(
  
  def toFileAs(self, path):
    f = open(path, "w")
    
    for s, t, u in self.edges:
      f.write("%d\t%d\n" % (s, t)) 
    
    f.close()
  
  def toGraph(self):
    graph = igraph.Graph()
    
    # come mai in alcuni casi funziona con -1 altre volte senza?
    maxv = len(self.vertices) # -1
    graph.add_vertices(maxv) 
    
    edgelist = []
    edgename = []
    # controllo
    for i in self.edges:
      a, b, c = i
      if a >= maxv: 
        print a
      if b >= maxv:
        print b
      edgelist.append((a, b))
      edgename.append(c)
    
    graph.add_edges(edgelist)
    
    labels = sorted(self.vertices.values(), key = select)
    graph.vs["label"] = [i[1] for i in labels]
    # graph.vs["label"] = self.vertices.keys()
    # graph.vs["ids"] = self.vertices.values()
    graph.es["names"] = edgename
    
    
    return graph

# leggi il contenuto del file
cv = Converter()

infile = file(sys.argv[1], "r")
d = infile.readlines()
data = (i.replace("\n","").split("\t") for i in d if i[0] != "#");
infile.close()

cv.add(data, len(d))

if format == "graphmlz":
  graph = cv.toGraph()
  graph.write_graphmlz(sys.argv[2])
elif format == "graphml":
  print "graphml"
  graph = cv.toGraph()
  graph.write_graphml(sys.argv[2])
else:
  cv.toFileAs(sys.argv[2])
