package clique_modularity.util.metric;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.BufferedReader;
import java.io.FileReader;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

import clique_modularity.util.Pair;
import conga.CONGA;


public class Modularity {
    static int maxVertex = 0;

    public Modularity() {
    }

    public static void main(String[] var0) {
        String var1 = var0[0];
        String var2 = var0[1];
        double var3 = modularity(var1, var2);
        System.out.println(var1 + ", " + var2);
        System.out.println("clique_modularity.util.metric.Modularity is " + var3);
    }

    public static double modularity(String var0, String var1) {
        ArrayList var8 = new ArrayList();
        HashMap var9 = new HashMap();
        HashMap var10 = CONGA.readGraphEdges(var0, (String) null, false, (HashMap) null);
        Iterator var6 = var10.keySet().iterator();

        while (var6.hasNext()) {
            String var4 = (String) var6.next();
            int var2 = numVertex(var4, var9);
            Iterator var7 = ((HashSet) var10.get(var4)).iterator();

            while (var7.hasNext()) {
                String var5 = (String) var7.next();
                int var3 = numVertex(var5, var9);
                if (var2 < var3) {
                    var8.add(new Pair(var2, var3));
                }
            }
        }

        ArrayList var11 = new ArrayList();
        int var12 = readClusters(var11, var1, var9);
        return modularity(var12, var11, var8);
    }

    private static double modularity(int var0, List<Integer> var1, List<Pair> var2) {
        int var10 = 0;
        int[][] var12 = new int[var0][var0];

        int var3;
        int var4;
        for (var3 = 0; var3 < var0; ++var3) {
            for (var4 = 0; var4 < var0; ++var4) {
                var12[var3][var4] = 0;
            }
        }

        for (Iterator var13 = var2.iterator(); var13.hasNext(); ++var10) {
            Pair var11 = (Pair) var13.next();
            int var5 = ((Integer) var1.get(var11.value1)).intValue();
            int var6 = ((Integer) var1.get(var11.value2)).intValue();
            ++var12[var5][var6];
            ++var12[var6][var5];
        }

        double var14 = (double) (var10 + var10);
        double var16 = 0.0D;

        for (var3 = 0; var3 < var0; ++var3) {
            int var7 = 0;

            for (var4 = 0; var4 < var0; ++var4) {
                var7 += var12[var3][var4];
            }

            double var8 = (double) var7;
            var16 += ((double) var12[var3][var3] - var8 * var8 / var14) / var14;
        }

        return var16;
    }

    private static int readClusters(List<Integer> var0, String var1, HashMap<String, Integer> var2) {
        int var7 = 0;
        String[] var10 = null;

        String var8;
        try {
            for (BufferedReader var12 = new BufferedReader(new FileReader(var1)); (var8 = var12.readLine()) != null && !var8.equals(""); ++var7) {
                var10 = var8.split(" ");
                int var5 = Array.getLength(var10);
                byte var6;
                if (var10[0].endsWith(":")) {
                    var6 = 1;
                } else {
                    var6 = 0;
                }

                for (int var3 = var6; var3 < var5; ++var3) {
                    String var9 = var10[var3];
                    int var4 = numVertex(var9, var2);

                    while (var0.size() <= var4) {
                        var0.add(Integer.valueOf(-1));
                    }

                    var0.set(var4, Integer.valueOf(var7));
                }
            }
        } catch (Exception var13) {
            System.out.println("Clusters/groups file error: " + var13.toString());
            System.exit(1);
        }

        return var7;
    }

    private static int numVertex(String var0, HashMap<String, Integer> var1) {
        int var2;
        if (var1.containsKey(var0)) {
            var2 = ((Integer) var1.get(var0)).intValue();
        } else {
            var2 = maxVertex++;
            var1.put(var0, Integer.valueOf(var2));
        }

        return var2;
    }
}
