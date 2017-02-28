package clique_modularity.algorithm.BK;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import clique_modularity.util.community.CompleteCliques1;
import clique_modularity.input_output.ReadEdges;
import clique_modularity.util.community.RebuildCommunities;
import clique_modularity.input_output.WriteCliques;

import java.io.FileNotFoundException;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashSet;
import java.util.Iterator;

public class BK {
    private static ArrayList<HashSet<Integer>> resultCliques = new ArrayList();
    public static int cliquesIndex = 0;
    public static long start_time;
    public static long end_time;
    public static long addedges_time;
    public static long cliquesfinding_time;
    public static long rebuildcommunities_time;

    public BK() {
    }

    public static void main(String[] var0) throws FileNotFoundException {
        boolean var1 = false;
        boolean var2 = false;
        int var3 = Array.getLength(var0);
        if(var3 == 4) {
            String var4 = var0[0];
            int var6 = Integer.parseInt(var0[1]);
            int var7 = Integer.parseInt(var0[2]);
            int var5 = Integer.parseInt(var0[3]);
        } else {
            System.out.println("Arguments should be filename, graph size, communities, N in order");
            System.out.println("filename means that the name of the graph file");
            System.out.println("size means that the number of nodes in the graph");
            System.out.println("communities means the number of communities are in graph");
            System.out.println("N means that the same number of neigbours two nodes should have");
            System.exit(1);
        }

    }

    public static long communityResults(String var0, String var1, int var2, int var3) throws FileNotFoundException {
        start_time = (new Date()).getTime();
        ArrayList var4 = null;
        Object var5 = null;
        boolean var7 = true;
        ReadEdges var6 = new ReadEdges();
        var4 = var6.readGraph(var0, var2);
        resultCliques = new ArrayList((new BronKerboschCFFast(var4)).getAllMaximalCliques());
        cliquesIndex = resultCliques.size();
        cliquesfinding_time = (new Date()).getTime();
        ArrayList var9 = CompleteCliques1.completeCliques1(resultCliques, var2, cliquesIndex, var0);
        String var10 = "";
        String var11 = "";
        var10 = "beforeMerge_BK.txt";
        var11 = "mergedCommunities.txt";
        WriteCliques.saveGraphfile(var9, var10);
        long var12 = (new Date()).getTime();
        ArrayList var14 = RebuildCommunities.rebuildCommunities(var3, var2, var9, var0);
        rebuildcommunities_time = (new Date()).getTime();
        WriteCliques.saveGraphfile(var14, var11);
        resultCliques.clear();
        var9.clear();
        var14.clear();
        cliquesIndex = 0;
        end_time = (new Date()).getTime();
        return end_time - start_time;
    }

    private static void saveCliques(ArrayList var0) {
        int var1 = var0.size();
        resultCliques.add(new HashSet());

        for(int var2 = 0; var2 < var1; ++var2) {
            int var5 = ((Integer)var0.get(var2)).intValue();
            ((HashSet)resultCliques.get(cliquesIndex)).add(Integer.valueOf(var5));
        }

        ++cliquesIndex;
    }

    private static void printGraph(ArrayList<HashSet<Integer>> var0) {
        int var4 = var0.size();
        System.out.println("ResultSize: " + var4);

        for(int var2 = 0; var2 < var4; ++var2) {
            Iterator var1 = ((HashSet)var0.get(var2)).iterator();

            while(var1.hasNext()) {
                int var3 = ((Integer)var1.next()).intValue();
                System.out.print(var3 + " ");
            }

            System.out.println();
        }

    }

    private static void calculate(ArrayList<HashSet<Integer>> var0) {
        int[] var3 = new int[30];
        int var1 = var0.size();

        int var2;
        for(var2 = 0; var2 < var1; ++var2) {
            ++var3[((HashSet)var0.get(var2)).size()];
        }

        for(var2 = 0; var2 < 30; ++var2) {
            System.out.println(var2 + " : " + var3[var2]);
        }

    }
}
