package conga.algorithm;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import conga.CONGA;
import conga.util.community.BP;
import conga.util.graph.Split;
import conga.util.graph.Edge;
import conga.util.graph.Vertex;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.TreeSet;

public class Between {
    public List<HashMap<Integer, BP>> betweenV = new ArrayList();
    public List<PB> pbV = new ArrayList();
    List<HashMap<Integer, Float>> weightsI;
    TreeSet<Edge> edges = new TreeSet<Edge>(new Comparator<Edge>() {
        public int compare(Edge var1, Edge var2) {
            if (var1.vertex1 == var2.vertex1 && var1.vertex2 == var2.vertex2) {
                return 0;
            } else {
                float var3 = Between.this.wBetweenness(var1, Between.this.weightsI);
                float var4 = Between.this.wBetweenness(var2, Between.this.weightsI);
                return CONGA.detLess(var3, var4, var1.vertex1, var2.vertex1, var1.vertex2, var2.vertex2) ? -1 : 1;
            }
        }
    });
    TreeSet<Vertex> vertices = new TreeSet<Vertex>(new Comparator<Vertex>() {
        public int compare(Vertex var1, Vertex var2) {
            return var1.vertex == var2.vertex ? 0 : (CONGA.detLess(var1.betweenness, var2.betweenness, var1.vertex, var2.vertex) ? -1 : 1);
        }
    });

    public Between(List<HashSet<Integer>> var1, HashSet<String> var2, List<HashMap<Integer, Float>> var3) {
        int var7 = var1.size();
        this.weightsI = var3;
        if (var2.contains("off")) {
            this.pbV = null;
            this.vertices = null;
        }

        this.setSize(var7);

        for (int var5 = 0; var5 < var7; ++var5) {
            Iterator var4 = ((HashSet) var1.get(var5)).iterator();

            while (var4.hasNext()) {
                int var6 = ((Integer) var4.next()).intValue();
                if (var5 < var6) {
                    this.addEdge(var5, var6);
                }
            }

            this.addVertex(var5, (HashSet) var1.get(var5));
        }

    }

    public PB getPB(int var1) {
        return this.pbV == null ? null : (PB) this.pbV.get(var1);
    }

    public void setSize(int var1) {
        if (this.pbV != null) {
            while (this.pbV.size() < var1) {
                this.pbV.add(null);
            }
        }

        while (this.betweenV.size() < var1) {
            this.betweenV.add(null);
        }

    }

    public Edge bestEdge() {
        return this.edges.isEmpty() ? null : (Edge) this.edges.last();
    }

    public Vertex bestVertex() {
        return this.vertices != null && !this.vertices.isEmpty() ? (Vertex) this.vertices.last() : new Vertex(0);
    }

    public void initComp(TreeSet<Integer> var1, List<HashSet<Integer>> var2) {
        int var3;
        for (Iterator var4 = var1.iterator(); var4.hasNext(); this.addVertex(var3, (HashSet) var2.get(var3))) {
            var3 = ((Integer) var4.next()).intValue();
            this.replaceEdges(var3, (HashSet) var2.get(var3));
            if (this.getPB(var3) != null) {
                this.deleteVertex(var3);
            }
        }

    }

    public void removeEdge(int var1, int var2) {
        this.deleteEdge(var1, var2);
        this.reduceVertex(var1, var2);
        this.reduceVertex(var2, var1);
    }

    public void splitVertex(int var1, int var2, HashSet<Integer> var3, HashSet<Integer> var4) {
        PB var6 = (PB) this.pbV.get(var1);
        if (var6 != null) {
            var6.print("About to initialize conga.algorithm.PB (splitVertex) " + var1);
        }

        this.deleteVertex(var1);
        this.addVertex(var1, var3);
        this.addVertex(var2, var4);
        Iterator var7 = var4.iterator();

        while (var7.hasNext()) {
            int var5 = ((Integer) var7.next()).intValue();
            this.replaceEdge(var5, var1, var2);
            var6 = (PB) this.pbV.get(var5);
            if (var6 != null) {
                var6.replace(var1, var2);
            }
        }

    }

    public void putEdge(int var1, int var2) {
        Float var3 = Float.valueOf(((BP) ((HashMap) this.betweenV.get(var1)).get(Integer.valueOf(var2))).oldB);
        boolean var4 = this.edges.remove(new Edge(var1, var2, var3.floatValue()));
        if (!var4) {
            System.out.println(this.edges);
            System.out.println("(putEdge) remove failed: " + var1 + "/" + var2 + " " + var3);
        }

        var3 = Float.valueOf(((BP) ((HashMap) this.betweenV.get(var1)).get(Integer.valueOf(var2))).newB);
        this.edges.add(new Edge(var1, var2, var3.floatValue()));
        ((BP) ((HashMap) this.betweenV.get(var1)).get(Integer.valueOf(var2))).oldB = var3.floatValue();
    }

    public void putVertex(int var1, float var2, Split var3) {
        this.putVertex(var1, var2, var3, 0.0F);
    }

    public void putVertex(int var1, float var2, Split var3, float var4) {
        PB var5 = (PB) this.pbV.get(var1);
        boolean var6 = this.vertices.remove(var5.vertex);
        if (!var6) {
            System.out.println("(putVertex) remove failed: " + var1);
        }

        var5.vertex = new Vertex(var1, var2, var3, var4);
        this.vertices.add(var5.vertex);
    }

    public Vertex getVertex(int var1) {
        PB var2 = (PB) this.pbV.get(var1);
        return var2 == null ? null : var2.vertex;
    }

    private float wBetweenness(Edge var1, List<HashMap<Integer, Float>> var2) {
        if (var2 == null) {
            return var1.betweenness;
        } else {
            Float var3;
            if (var1.vertex1 < var1.vertex2) {
                var3 = (Float) ((HashMap) var2.get(var1.vertex1)).get(Integer.valueOf(var1.vertex2));
            } else {
                var3 = (Float) ((HashMap) var2.get(var1.vertex2)).get(Integer.valueOf(var1.vertex1));
            }

            return var3 == null ? 0.0F : var1.betweenness / var3.floatValue();
        }
    }

    private void replaceEdges(int var1, HashSet<Integer> var2) {
        int var3;
        Iterator var4;
        if (var1 < this.betweenV.size() && this.betweenV.get(var1) != null) {
            HashSet var5 = new HashSet(((HashMap) this.betweenV.get(var1)).keySet());
            var4 = var5.iterator();

            while (var4.hasNext()) {
                var3 = ((Integer) var4.next()).intValue();
                this.deleteEdge(var1, var3);
            }
        }

        var4 = var2.iterator();

        while (var4.hasNext()) {
            var3 = ((Integer) var4.next()).intValue();
            if (var1 < var3) {
                this.addEdge(var1, var3);
            }
        }

    }

    private void replaceEdge(int var1, int var2, int var3) {
        if (var1 < var2) {
            this.deleteEdge(var1, var2);
        } else {
            this.deleteEdge(var2, var1);
        }

        if (var1 < var3) {
            this.addEdge(var1, var3);
        } else {
            this.addEdge(var3, var1);
        }

    }

    private void deleteEdge(int var1, int var2) {
        float var3 = ((BP) ((HashMap) this.betweenV.get(var1)).get(Integer.valueOf(var2))).oldB;
        ((HashMap) this.betweenV.get(var1)).remove(Integer.valueOf(var2));
        if (((HashMap) this.betweenV.get(var1)).isEmpty()) {
            this.betweenV.set(var1, null);
        }

        boolean var4 = this.edges.remove(new Edge(var1, var2, var3));
        if (!var4) {
            System.out.println("(deleteEdge) remove failed: " + var1 + "/" + var2 + " " + var3);
        }

    }

    private void addEdge(int var1, int var2) {
        Edge var3 = new Edge(var1, var2, 0.0F);
        this.edges.add(var3);
        if (this.betweenV.get(var1) == null) {
            this.betweenV.set(var1, new HashMap());
        }

        ((HashMap) this.betweenV.get(var1)).put(Integer.valueOf(var2), new BP(0.0F, 0.0F));
    }

    private void reduceVertex(int var1, int var2) {
        PB var3 = this.getPB(var1);
        if (var3 != null && var3.remove(var2) == 3) {
            this.deleteVertex(var1);
        }

    }

    private void deleteVertex(int var1) {
        Vertex var2 = ((PB) this.pbV.get(var1)).vertex;
        this.pbV.set(var1, null);
        boolean var3 = this.vertices.remove(var2);
        if (!var3) {
            System.out.println("(deleteVertex) remove failed: " + var1);
        }

    }

    private void addVertex(int var1, HashSet<Integer> var2) {
        if (var2.size() >= 4 && this.pbV != null) {
            Vertex var3 = new Vertex(var1);
            this.vertices.add(var3);
            this.pbV.set(var1, new PB(var2, var3));
        }

    }

    public void printBetweenness(int var1, List<String> var2) {
        Iterator var6 = this.edges.iterator();

        while (var6.hasNext()) {
            Edge var5 = (Edge) var6.next();
            double var3;
            if (var5.vertex1 < var5.vertex2) {
                var3 = (double) ((BP) ((HashMap) this.betweenV.get(var5.vertex1)).get(Integer.valueOf(var5.vertex2))).newB;
            } else {
                var3 = (double) ((BP) ((HashMap) this.betweenV.get(var5.vertex2)).get(Integer.valueOf(var5.vertex1))).newB;
            }

            System.out.format("%s/%s=%.2f", new Object[]{var2.get(var5.vertex1), var2.get(var5.vertex2), Double.valueOf(var3)});
            System.out.println();
        }

    }

    public void checkConsistency(int var1, List<HashSet<Integer>> var2) {
        int var7 = 0;
        int var8 = 0;

        int var5;
        int var6;
        Iterator var10;
        for (var5 = 0; var5 < var1 - 1; ++var5) {
            if (this.betweenV.get(var5) != null) {
                var8 += ((HashMap) this.betweenV.get(var5)).size();
                var10 = ((HashMap) this.betweenV.get(var5)).keySet().iterator();

                while (var10.hasNext()) {
                    var6 = ((Integer) var10.next()).intValue();
                    if (!((HashSet) var2.get(var5)).contains(Integer.valueOf(var6))) {
                        System.out.println("***conga.util.graph.Edge in between but not in graph: " + var5 + "/" + var6);
                    }

                    if (!this.edges.contains(new Edge(var5, var6, ((BP) ((HashMap) this.betweenV.get(var5)).get(Integer.valueOf(var6))).oldB))) {
                        System.out.println("***conga.util.graph.Edge in between but not in treeset: " + var5 + "/" + var6);
                    }
                }
            }

            if (this.pbV != null) {
                if (this.pbV.get(var5) != null && ((HashSet) var2.get(var5)).size() < 4) {
                    System.out.println("***conga.util.graph.Vertex in between but not in graph: " + var5);
                }

                if (this.pbV.get(var5) != null && !this.vertices.contains(((PB) this.pbV.get(var5)).vertex)) {
                    System.out.println("***conga.util.graph.Vertex in between but not in treeset: " + var5);
                }
            }
        }

        label110:
        for (var5 = 0; var5 < var2.size(); ++var5) {
            HashSet var9 = (HashSet) var2.get(var5);
            var7 += var9.size();
            if (var9.size() >= 4 && this.pbV != null && this.pbV.get(var5) == null) {
                System.out.println("***conga.util.graph.Vertex in graph but not in between: " + var5);
            }

            var10 = var9.iterator();

            while (true) {
                do {
                    do {
                        if (!var10.hasNext()) {
                            continue label110;
                        }

                        var6 = ((Integer) var10.next()).intValue();
                    } while (var5 >= var6);
                }
                while (this.betweenV.get(var5) != null && ((HashMap) this.betweenV.get(var5)).containsKey(Integer.valueOf(var6)));

                System.out.println("***conga.util.graph.Edge in graph but not in between: " + var5 + "/" + var6);
            }
        }

        Iterator var11 = this.edges.iterator();

        while (true) {
            do {
                do {
                    if (!var11.hasNext()) {
                        if (this.pbV != null) {
                            Iterator var12 = this.vertices.iterator();

                            while (var12.hasNext()) {
                                Vertex var4 = (Vertex) var12.next();
                                var5 = var4.vertex;
                                if (this.pbV.get(var5) == null) {
                                    System.out.println("***conga.util.graph.Vertex in treeset but not in between: " + var5);
                                }
                            }
                        }

                        if (var7 / 2 != var8 || var8 != this.edges.size() || var7 / 2 != this.edges.size()) {
                            System.out.println("***Sizes: " + var7 / 2 + " " + var8 + " " + this.edges.size());
                        }

                        return;
                    }

                    Edge var3 = (Edge) var11.next();
                    var5 = var3.vertex1;
                    var6 = var3.vertex2;
                } while (var5 >= var6);
            }
            while (this.betweenV.get(var5) != null && ((HashMap) this.betweenV.get(var5)).containsKey(Integer.valueOf(var6)));

            System.out.println("***conga.util.graph.Edge in treeset but not in between: " + var5 + "/" + var6);
        }
    }

    public ArrayList<Float> getSplitBetweenness(List<String> var1) {
        ArrayList var2 = new ArrayList();
        Iterator var5 = this.vertices.descendingIterator();

        while (true) {
            Vertex var3;
            int var4;
            do {
                if (!var5.hasNext()) {
                    return var2;
                }

                var3 = (Vertex) var5.next();
                var4 = var3.vertex;

                while (var2.size() <= var4) {
                    var2.add(Float.valueOf(0.0F));
                }

                var2.set(var4, Float.valueOf(var3.betweenness));
            } while (var4 != 3 && var4 != 26);

            System.out.println((String) var1.get(var4) + "\t" + var3.betweenness);
        }
    }

    public ArrayList<Float> getVertexBetweenness(TreeSet<Integer> var1, List<HashSet<Integer>> var2, List<String> var3) {
        ArrayList var4 = new ArrayList();
        Iterator var9 = var1.iterator();

        while (var9.hasNext()) {
            int var5 = ((Integer) var9.next()).intValue();
            Vertex var8 = this.getVertex(var5);
            float var7;
            if (var8 == null) {
                var7 = 0.0F;
            } else {
                var7 = var8.vB;
            }

            while (var4.size() <= var5) {
                var4.add(Float.valueOf(0.0F));
            }

            var4.set(var5, Float.valueOf(var7));
        }

        return var4;
    }
}
