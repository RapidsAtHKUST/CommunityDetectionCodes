package clique_modularity.util.metric;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.BufferedReader;
import java.io.FileReader;
import java.lang.reflect.Array;
import java.util.HashMap;
import java.util.Vector;

public class Omega {
    public Omega() {
    }

    public static void main(String[] var0) {
        Vector var1 = new Vector();
        Vector var2 = new Vector();
        int var3 = Integer.parseInt(var0[0]);
        String var4 = var0[1];
        String var5 = var0[2];
        int var6 = readCount(var1, var4, var3);
        int var7 = readCount(var2, var5, var3);
        double var8 = evaluateOmega(var1, var2, (long) var3, var6, var7);
        System.out.println(var4 + ", " + var5 + ", size " + var3);
        System.out.println("clique_modularity.util.metric.Omega is " + var8);
    }

    public static int readCount(Vector<HashMap<Integer, Integer>> var0, String var1, int var2) {
        int var10 = 0;
        String[] var12 = null;

        int var3;
        for (var3 = 0; var3 < var2; ++var3) {
            var0.add(new HashMap());
        }

        try {
            BufferedReader var14 = new BufferedReader(new FileReader(var1));

            String var11;
            while ((var11 = var14.readLine()) != null && !var11.equals("")) {
                var12 = var11.split(" ");
                int var5 = Array.getLength(var12);
                byte var8;
                if (var12[0].endsWith(":")) {
                    var8 = 1;
                } else {
                    var8 = 0;
                }

                for (var3 = var8; var3 < var5; ++var3) {
                    int var6 = Integer.parseInt(var12[var3]);

                    for (int var4 = var3 + 1; var4 < var5; ++var4) {
                        if (var3 != var4) {
                            int var7 = Integer.parseInt(var12[var4]);
                            HashMap var13 = (HashMap) var0.get(var7);
                            if (!var13.containsKey(Integer.valueOf(var6))) {
                                var13.put(Integer.valueOf(var6), Integer.valueOf(0));
                            }

                            int var9 = ((Integer) var13.get(Integer.valueOf(var6))).intValue() + 1;
                            var13.put(Integer.valueOf(var6), Integer.valueOf(var9));
                            if (var9 > var10) {
                                var10 = var9;
                            }

                            var13 = (HashMap) var0.get(var6);
                            if (!var13.containsKey(Integer.valueOf(var7))) {
                                var13.put(Integer.valueOf(var7), Integer.valueOf(0));
                            }

                            var9 = ((Integer) var13.get(Integer.valueOf(var7))).intValue() + 1;
                            var13.put(Integer.valueOf(var7), Integer.valueOf(var9));
                            if (var9 > var10) {
                                var10 = var9;
                            }
                        }
                    }
                }
            }
        } catch (Exception var15) {
            System.out.println("Clusters/groups file error 1: " + var1 + ": " + var15.toString());
            System.exit(1);
        }

        return var10;
    }

    public static double evaluateOmega(Vector<HashMap<Integer, Integer>> var0, Vector<HashMap<Integer, Integer>> var1, long var2, int var4, int var5) {
        long var13 = 0L;
        long var15 = 0L;
        int var17 = Math.min(var4, var5);
        long[] var18 = new long[var17 + 1];
        long[] var19 = new long[var17 + 1];
        long[][] var20 = new long[var4 + 1][var5 + 1];

        int var9;
        int var10;
        for (var9 = 0; (long) var9 < var2; ++var9) {
            HashMap var6 = (HashMap) var0.get(var9);
            HashMap var7 = (HashMap) var1.get(var9);

            for (var10 = var9 + 1; (long) var10 < var2; ++var10) {
                int var11;
                if (var6.containsKey(Integer.valueOf(var10))) {
                    var11 = ((Integer) var6.get(Integer.valueOf(var10))).intValue();
                } else {
                    var11 = 0;
                }

                int var12;
                if (var7.containsKey(Integer.valueOf(var10))) {
                    var12 = ((Integer) var7.get(Integer.valueOf(var10))).intValue();
                } else {
                    var12 = 0;
                }

                ++var20[var11][var12];
            }
        }

        long var21 = var2 * (var2 - 1L) / 2L;

        int var8;
        for (var8 = 0; var8 <= var17; ++var8) {
            var18[var8] = 0L;
            var19[var8] = 0L;

            for (var9 = 0; var9 <= var4; ++var9) {
                var19[var8] += var20[var9][var8];
            }

            for (var10 = 0; var10 <= var5; ++var10) {
                var18[var8] += var20[var8][var10];
            }
        }

        for (var8 = 0; var8 <= var17; ++var8) {
            var13 += var20[var8][var8];
            var15 += var19[var8] * var18[var8];
        }

        return (double) (var21 * var13 - var15) / (double) (var21 * var21 - var15);
    }
}
