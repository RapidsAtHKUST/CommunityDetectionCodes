package conga.util.community;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import conga.util.graph.Split;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class Stats {
    public boolean clustering = false;
    public int initialGraphSize = 0;
    public int finalGraphSize = 0;
    public int nClusters = 0;
    public int clustGraphSize = 0;
    public int nEdges = 0;
    public int nSame = 0;
    public int nDiff = 0;
    public int pSame = 0;
    public int pDiff = 0;
    public int nEdgesRemoved = 0;
    public int nVerticesSplit = 0;
    public int nSingletonSplits = 0;
    public int nBetweenPhase = 0;
    public HashMap<String, Integer> verticesSplit = new HashMap();
    public ArrayList<Integer> compSizes = new ArrayList();
    public long time = 0L;
    public long time1 = 0L;
    public long time2 = 0L;
    public long time3 = 0L;
    public long time4 = 0L;
    public long time5 = 0L;
    public long time6 = 0L;
    public List<Double> mod = new ArrayList();
    public List<Double> ov = new ArrayList();
    public double resultMod = 0.0D;
    public double resultNewMod = 0.0D;
    public double resultVad = 0.0D;
    public double overlap = 0.0D;

    public Stats() {
    }

    public void showStats(String var1) throws Exception {
        if(this.clustering) {
            System.out.println("Initial graph: " + this.initialGraphSize + " vertices, " + this.nEdges + " edges");
            System.out.println("Initial graph: " + this.compSizes.size() + " components, with sizes: " + this.compSizes);
            System.out.println("Clustering: Final graph size: " + this.finalGraphSize);
            System.out.println("Clustering: Vertices split: " + this.nVerticesSplit + " total, " + this.verticesSplit.size() + " distinct");
            System.out.println("Clustering: Vertices split: " + this.verticesSplit);
            System.out.println("Clustering: Betweenness phases: " + this.nBetweenPhase);
            System.out.println("Clustering: Total time: " + this.time + "ms");
        }

    }

    public void splitVertex(String var1, Split var2) {
        if(this.verticesSplit.containsKey(var1)) {
            this.verticesSplit.put(var1, Integer.valueOf(((Integer)this.verticesSplit.get(var1)).intValue() + 1));
        } else {
            this.verticesSplit.put(var1, Integer.valueOf(1));
        }

        ++this.nVerticesSplit;
        if(var2.value1.size() == 1 || var2.value2.size() == 1) {
            ++this.nSingletonSplits;
        }

    }
}
