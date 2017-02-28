package clique_modularity.algorithm.KJ;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import clique_modularity.algorithm.MaxClique;
import clique_modularity.input_output.ReadEdges;
import clique_modularity.input_output.WriteCliques;
import clique_modularity.util.ARRAY2;
import clique_modularity.util.community.CompleteCliques1;
import clique_modularity.util.community.RebuildCommunities;
import clique_modularity.util.community.RemoveCliques;

import java.io.FileNotFoundException;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TreeSet;

public class KJ {
    private static ArrayList<HashSet<Integer>> resultCliques = new ArrayList();
    public static int cliquesIndex = 0;
    public static long start_time;
    public static long end_time;
    public static long addedges_time;
    public static long cliquesfinding_time;
    public static long rebuildcommunities_time;

    public KJ() {
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
        boolean var4 = true;
        start_time = (new Date()).getTime();
        ArrayList var5 = null;
        Object var6 = null;
        RemoveCliques var8 = null;
        int var11 = 2147483647;
        ARRAY2 var12 = null;
        boolean[][] var13 = (boolean[][])null;
        boolean var14 = true;
        ReadEdges var7 = new ReadEdges();
        String var16 = "cliquesNew.txt";
        var5 = var7.readGraph(var0, var2);
        var12 = MaxClique.convert_V(var2);

        ArrayList var9;
        for(var13 = MaxClique.convert_e(var5); var4; var5 = RemoveCliques.removeCliques(var5, var9, var13, (TreeSet)null)) {
            HashSet var10 = MaxClique.mcq(var5, var12, var13, var2, var11, (TreeSet)null);
            var11 = var10.size();
            if(var10.size() < 3) {
                break;
            }

            var9 = converseTolist(var10);
            saveCliques(var9);
            var8 = new RemoveCliques();
        }

        cliquesfinding_time = (new Date()).getTime();
        ArrayList var17 = CompleteCliques1.completeCliques1(resultCliques, var2, cliquesIndex, var0);
        long var18 = (new Date()).getTime();
        String var20 = "";
        String var21 = "";
        var20 = "beforeMerge_KJ.txt";
        var21 = "mergedCommunities.txt";
        WriteCliques.saveGraphfile(var17, var20);
        ArrayList var22 = RebuildCommunities.rebuildCommunities(var3, var2, var17, var0);
        rebuildcommunities_time = (new Date()).getTime();
        WriteCliques.saveGraphfile(var22, var21);
        resultCliques.clear();
        var17.clear();
        var22.clear();
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

    private static ArrayList converseTolist(HashSet<Integer> var0) {
        ArrayList var1 = new ArrayList();
        Iterator var4 = var0.iterator();

        while(var4.hasNext()) {
            int var3 = ((Integer)var4.next()).intValue();
            var1.add(Integer.valueOf(var3));
        }

        return var1;
    }

    private static void printGraph(ArrayList<HashSet<Integer>> var0) {
        System.out.println("-----Test Experiment1-------");
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

        System.out.println("-----------------------------------");
    }
}
