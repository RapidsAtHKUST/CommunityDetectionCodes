package copra;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.FileOutputStream;
import java.io.PrintStream;
import java.lang.reflect.Array;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;

public class Project {
    public Project() {
    }

    public static void main(String[] var0) {
        boolean var1 = false;
        String var2 = "";
        ArrayList var3 = new ArrayList();
        ArrayList var4 = new ArrayList();
        new ArrayList();
        new ArrayList();
        new ArrayList();
        new ArrayList();
        ArrayList var9 = new ArrayList();
        ArrayList var10 = new ArrayList();
        ArrayList var11 = new ArrayList();
        int var12 = Array.getLength(var0);
        if(var12 < 1) {
            printUsageAndExit();
        }

        String var14 = var0[0];
        int var15 = 1;

        while(var15 < var12) {
            String var13 = var0[var15++];
            if(var13.equals("-w")) {
                var1 = true;
            }

            if(var13.equals("-s")) {
                var2 = var0[var15++];
            }
        }

        String var16 = var14.substring(0, var14.lastIndexOf(46));
        String var17 = var16 + "-1" + var2 + ".txt";
        String var18 = var16 + "-2" + var2 + ".txt";
        COPRA.readBiGraphEdges(var14, var3, var4, var9, var10, var11);
        if(var1) {
            List var7 = weightedProject(var3, var4, var11);
            List var8 = weightedProject(var4, var3, var11);
            writeWeightedEdges(var17, var7, var9);
            writeWeightedEdges(var18, var8, var10);
        } else {
            List var5 = project(var3, var4);
            List var6 = project(var4, var3);
            writeGraphEdges(var17, var5, var9);
            writeGraphEdges(var18, var6, var10);
        }

    }

    private static List<HashSet<Integer>> project(List<HashSet<Integer>> var0, List<HashSet<Integer>> var1) {
        ArrayList var2 = new ArrayList();

        for(int var4 = 0; var4 < var0.size(); ++var4) {
            HashSet var3 = new HashSet();
            Iterator var5 = ((HashSet)var0.get(var4)).iterator();

            while(var5.hasNext()) {
                var3.addAll((Collection)var1.get(((Integer)var5.next()).intValue()));
            }

            var3.remove(Integer.valueOf(var4));
            var2.add(var3);
        }

        return var2;
    }

    private static List<HashMap<Integer, Float>> weightedProject(List<HashSet<Integer>> var0, List<HashSet<Integer>> var1, List<HashMap<Integer, Float>> var2) {
        ArrayList var3 = new ArrayList();

        for(int var5 = 0; var5 < var0.size(); ++var5) {
            HashMap var4 = new HashMap();
            Iterator var9 = ((HashSet)var0.get(var5)).iterator();

            while(var9.hasNext()) {
                int var6 = ((Integer)var9.next()).intValue();
                Iterator var10 = ((HashSet)var1.get(var6)).iterator();

                while(var10.hasNext()) {
                    int var7 = ((Integer)var10.next()).intValue();
                    float var8 = ((Float)((HashMap)var2.get(var5)).get(Integer.valueOf(var6))).floatValue() * ((Float)((HashMap)var2.get(var7)).get(Integer.valueOf(var6))).floatValue();
                    if(var4.containsKey(Integer.valueOf(var7))) {
                        var4.put(Integer.valueOf(var7), Float.valueOf(((Float)var4.get(Integer.valueOf(var7))).floatValue() + var8));
                    } else {
                        var4.put(Integer.valueOf(var7), Float.valueOf(var8));
                    }
                }
            }

            var3.add(var4);
        }

        return var3;
    }

    private static void writeGraphEdges(String var0, List<HashSet<Integer>> var1, List<String> var2) {
        try {
            PrintStream var7 = new PrintStream(new FileOutputStream(var0));

            for(int var3 = 0; var3 < var1.size(); ++var3) {
                String var4 = (String)var2.get(var3);
                Iterator var6 = ((HashSet)var1.get(var3)).iterator();

                while(var6.hasNext()) {
                    String var5 = (String)var2.get(((Integer)var6.next()).intValue());
                    if(var4.compareTo(var5) < 0) {
                        var7.println(var4 + "\t" + var5);
                    }
                }
            }
        } catch (Exception var8) {
            var8.printStackTrace();
            System.exit(1);
        }

    }

    private static void writeWeightedEdges(String var0, List<HashMap<Integer, Float>> var1, List<String> var2) {
        try {
            PrintStream var9 = new PrintStream(new FileOutputStream(var0));

            for(int var3 = 0; var3 < var1.size(); ++var3) {
                String var5 = (String)var2.get(var3);
                HashMap var8 = (HashMap)var1.get(var3);
                Iterator var7 = var8.keySet().iterator();

                while(var7.hasNext()) {
                    int var4 = ((Integer)var7.next()).intValue();
                    String var6 = (String)var2.get(var4);
                    if(var5.compareTo(var6) < 0) {
                        var9.println(var5 + "\t" + var6 + "\t" + var8.get(Integer.valueOf(var4)));
                    }
                }
            }
        } catch (Exception var10) {
            var10.printStackTrace();
            System.exit(1);
        }

    }

    private static void printUsageAndExit() {
        System.err.println("Usage: java copra.Project <infile>");
        System.exit(1);
    }
}
