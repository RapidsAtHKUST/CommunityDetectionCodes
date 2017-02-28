package copra.util.metric;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.lang.reflect.Array;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Set;

public class ModOverlap {
    static int maxVertex = 0;

    public ModOverlap() {
    }

    public static void main(String[] var0) {
        String var1 = var0[0];
        String var2 = var0[1];
        double var3 = modOverlap(var1, var2);
        System.out.println(var1 + ", " + var2);
        System.out.println("Modularity is " + var3);
    }

    public static double modOverlap(String var0, String var1) {
        int var3 = 0;
        int var4 = 0;
        HashMap var5 = readClusters(var1);
        HashMap var6 = readGraphEdges(var0);

        String var2;
        for(Iterator var7 = var6.keySet().iterator(); var7.hasNext(); var3 += ((HashSet)var6.get(var2)).size()) {
            var2 = (String)var7.next();
            ++var4;
        }

        var3 /= 2;
        return modOverlap(var5, var6, (double)var4, (double)var3);
    }

    private static void showClusters(HashMap<Integer, HashMap<String, Double>> var0, int var1, int var2) {
        System.out.println("n = " + var1 + ", m = " + var2);
        System.out.println(var0);
        System.out.println(invertClusters(var0));
    }

    private static HashMap<String, HashMap<Integer, Double>> invertClusters(HashMap<Integer, HashMap<String, Double>> var0) {
        HashMap var6 = new HashMap();
        Iterator var7 = var0.keySet().iterator();

        while(var7.hasNext()) {
            int var1 = ((Integer)var7.next()).intValue();

            String var2;
            double var3;
            for(Iterator var5 = ((HashMap)var0.get(Integer.valueOf(var1))).keySet().iterator(); var5.hasNext(); ((HashMap)var6.get(var2)).put(Integer.valueOf(var1), Double.valueOf(var3))) {
                var2 = (String)var5.next();
                var3 = ((Double)((HashMap)var0.get(Integer.valueOf(var1))).get(var2)).doubleValue();
                if(!var6.containsKey(var2)) {
                    var6.put(var2, new HashMap());
                }
            }
        }

        return var6;
    }

    private static HashMap<Integer, HashMap<String, Double>> readClusters(String var0) {
        String[] var9 = null;
        String[] var10 = null;
        HashMap var12 = new HashMap();
        HashMap var13 = new HashMap();

        try {
            BufferedReader var14 = new BufferedReader(new FileReader(var0));

            String var7;
            for(int var2 = 0; (var7 = var14.readLine()) != null && !var7.equals(""); ++var2) {
                var9 = var7.split(" ");
                int var3 = Array.getLength(var9);
                byte var4;
                if(var9[0].endsWith(":")) {
                    var4 = 1;
                } else {
                    var4 = 0;
                }

                HashMap var11 = new HashMap();

                for(int var1 = var4; var1 < var3; ++var1) {
                    var10 = var9[var1].split(":");
                    String var8 = var10[0];
                    double var5;
                    if(Array.getLength(var10) == 1) {
                        var5 = 1.0D;
                    } else {
                        var5 = Double.parseDouble(var10[1]);
                    }

                    var11.put(var8, Double.valueOf(var5));
                    if(!var12.containsKey(var8)) {
                        var12.put(var8, Double.valueOf(0.0D));
                    }

                    var12.put(var8, Double.valueOf(((Double)var12.get(var8)).doubleValue() + var5));
                }

                var13.put(Integer.valueOf(var2), var11);
            }

            normalize(var13, var12);
        } catch (Exception var15) {
            System.out.println("Clusters/groups file error: " + var15.toString());
            System.exit(1);
        }

        return var13;
    }

    public static HashMap<String, HashSet<String>> readGraphEdges(String var0) {
        HashMap var6 = new HashMap();

        try {
            if((new File(var0)).exists()) {
                BufferedReader var1 = new BufferedReader(new FileReader(var0));
                int var7 = 0;

                String var2;
                while((var2 = var1.readLine()) != null) {
                    ++var7;
                    if(!var2.equals("") && !var2.startsWith("#")) {
                        String[] var5 = var2.split("[ \t]");
                        if(Array.getLength(var5) < 2) {
                            System.out.println("Missing vertices on line " + var7 + " of " + var0);
                            System.exit(1);
                        }

                        String var3 = var5[0];
                        String var4 = var5[1];
                        if(var3.equals(var4)) {
                            System.out.println("Ignoring self edge: " + var3 + "/" + var4);
                        } else {
                            if(var6.get(var3) == null) {
                                var6.put(var3, new HashSet());
                            }

                            if(!((HashSet)var6.get(var3)).add(var4)) {
                                ;
                            }

                            if(var6.get(var4) == null) {
                                var6.put(var4, new HashSet());
                            }

                            ((HashSet)var6.get(var4)).add(var3);
                        }
                    }
                }

                var1.close();
            } else {
                System.out.println("Graph file " + var0 + " not found");
                System.exit(1);
            }
        } catch (Exception var8) {
            System.out.println("readGraphEdges error: ");
            System.out.println(var8.toString());
            System.exit(1);
        }

        return var6;
    }

    public static void normalize(HashMap<Integer, HashMap<String, Double>> var0, HashMap<String, Double> var1) {
        Iterator var4 = var0.keySet().iterator();

        while(var4.hasNext()) {
            HashMap var3 = (HashMap)var0.get(var4.next());
            Iterator var5 = var3.keySet().iterator();

            while(var5.hasNext()) {
                String var2 = (String)var5.next();
                var3.put(var2, Double.valueOf(((Double)var3.get(var2)).doubleValue() / ((Double)var1.get(var2)).doubleValue()));
            }
        }

    }

    public static double modOverlap(HashMap<Integer, HashMap<String, Double>> var0, HashMap<String, HashSet<String>> var1, double var2, double var4) {
        double var19 = 0.0D;
        Iterator var7 = var0.keySet().iterator();

        Iterator var8;
        Iterator var9;
        String var10;
        String var11;
        int var12;
        while(var7.hasNext()) {
            var12 = ((Integer)var7.next()).intValue();
            var8 = ((HashMap)var0.get(Integer.valueOf(var12))).keySet().iterator();

            while(var8.hasNext()) {
                var10 = (String)var8.next();
                if(var1.get(var10) == null) {
                    System.out.println("vertex " + var10 + " not found in network");
                }

                for(var9 = ((HashSet)var1.get(var10)).iterator(); var9.hasNext(); var19 += F(var10, var11, var12, var0)) {
                    var11 = (String)var9.next();
                }
            }
        }

        var19 /= var4 + var4;
        double var23 = 0.0D;
        var7 = var0.keySet().iterator();

        while(var7.hasNext()) {
            var12 = ((Integer)var7.next()).intValue();
            Set var6 = ((HashMap)var0.get(Integer.valueOf(var12))).keySet();
            HashMap var21 = new HashMap();
            HashMap var22 = new HashMap();
            var8 = var6.iterator();

            while(var8.hasNext()) {
                var10 = (String)var8.next();
                var21.put(var10, Double.valueOf(beta_out(var10, var12, var2, var1, var0)));
                var22.put(var10, Double.valueOf(beta_in(var10, var12, var2, var1, var0)));
            }

            var8 = var6.iterator();

            while(var8.hasNext()) {
                var10 = (String)var8.next();

                int var13;
                int var14;
                double var15;
                double var17;
                for(var9 = var6.iterator(); var9.hasNext(); var23 += var17 * var15 * (double)var13 * (double)var14) {
                    var11 = (String)var9.next();
                    var17 = ((Double)var21.get(var10)).doubleValue();
                    var15 = ((Double)var22.get(var11)).doubleValue();
                    var13 = ((HashSet)var1.get(var10)).size();
                    var14 = ((HashSet)var1.get(var11)).size();
                }
            }
        }

        var23 /= 4.0D * var4 * var4;
        return var19 - var23;
    }

    private static double beta_out(String var0, int var1, double var2, HashMap<String, HashSet<String>> var4, HashMap<Integer, HashMap<String, Double>> var5) {
        double var9 = 0.0D;
        Set var12 = ((HashMap)var5.get(Integer.valueOf(var1))).keySet();
        if(!var12.contains(var0)) {
            return 0.0D;
        } else {
            Iterator var11 = var12.iterator();

            while(var11.hasNext()) {
                String var6 = (String)var11.next();
                if(!var0.equals(var6)) {
                    var9 += F(var0, var6, var1, var5);
                }
            }

            double var7 = var9 / (var2 - 1.0D);
            return var7;
        }
    }

    private static double beta_in(String var0, int var1, double var2, HashMap<String, HashSet<String>> var4, HashMap<Integer, HashMap<String, Double>> var5) {
        double var9 = 0.0D;
        Set var12 = ((HashMap)var5.get(Integer.valueOf(var1))).keySet();
        if(!var12.contains(var0)) {
            return 0.0D;
        } else {
            Iterator var11 = var12.iterator();

            while(var11.hasNext()) {
                String var6 = (String)var11.next();
                if(!var0.equals(var6)) {
                    var9 += F(var6, var0, var1, var5);
                }
            }

            double var7 = var9 / (var2 - 1.0D);
            return var7;
        }
    }

    private static double F(String var0, String var1, int var2, HashMap<Integer, HashMap<String, Double>> var3) {
        double var6 = alpha(var0, var2, var3);
        double var8 = alpha(var1, var2, var3);
        double var4 = 1.0D / ((1.0D + Math.exp(-f(var6))) * (1.0D + Math.exp(-f(var8))));
        return var4;
    }

    private static double f(double var0) {
        return 60.0D * var0 - 30.0D;
    }

    private static double alpha(String var0, int var1, HashMap<Integer, HashMap<String, Double>> var2) {
        return ((HashMap)var2.get(Integer.valueOf(var1))).keySet().contains(var0)?((Double)((HashMap)var2.get(Integer.valueOf(var1))).get(var0)).doubleValue():0.0D;
    }
}
