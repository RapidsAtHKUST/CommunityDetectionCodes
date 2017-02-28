package clique_modularity.util.community;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import clique_modularity.util.graph.ELEMENT;

import java.io.FileOutputStream;
import java.io.PrintStream;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TreeSet;

public class RemoveCliques {
    public RemoveCliques() {
    }

    public static ArrayList<HashSet<Integer>> removeCliques(ArrayList<HashSet<Integer>> var0, ArrayList var1, boolean[][] var2, TreeSet<ELEMENT> var3) {
        for (int var4 = 0; var4 < var1.size(); ++var4) {
            int var5 = ((Integer) var1.get(var4)).intValue();

            int var6;
            for (Iterator var7 = ((HashSet) var0.get(var5)).iterator(); var7.hasNext(); var2[var5][var6] = false) {
                var6 = ((Integer) var7.next()).intValue();
                var7.remove();
                ((HashSet) var0.get(var6)).remove(Integer.valueOf(var5));
                var2[var6][var5] = false;
            }
        }

        return var0;
    }

    public static void saveNewedges(ArrayList<HashSet<Integer>> var0, String var1) {
        int var5 = var0.size();

        try {
            PrintStream var6 = new PrintStream(new FileOutputStream(var1));

            for (int var3 = 0; var3 < var5; ++var3) {
                Iterator var2 = ((HashSet) var0.get(var3)).iterator();

                while (var2.hasNext()) {
                    int var4 = ((Integer) var2.next()).intValue();
                    var6.println(var3 + " " + var4);
                }
            }
        } catch (Exception var7) {
            System.out.println("Error " + var7.toString());
            var7.printStackTrace();
            System.exit(1);
        }

    }

    public static void saveUndirected(ArrayList<HashSet<Integer>> var0) {
        int var4 = var0.size() - 1;
        String var5 = "tests/cliquesNew1.txt";

        try {
            PrintStream var6 = new PrintStream(new FileOutputStream(var5));
            var6.println("p edge " + var4);

            for (int var2 = 0; var2 < var4; ++var2) {
                Iterator var1 = ((HashSet) var0.get(var2)).iterator();

                while (var1.hasNext()) {
                    int var3 = ((Integer) var1.next()).intValue();
                    if (var3 > var2) {
                        var6.println("e " + var2 + " " + var3);
                    }
                }
            }
        } catch (Exception var7) {
            System.out.println("Error " + var7.toString());
            var7.printStackTrace();
            System.exit(1);
        }

    }

    private static void printGraph(ArrayList<HashSet<Integer>> var0) {
        int var4 = var0.size();

        for (int var2 = 0; var2 < var4; ++var2) {
            Iterator var1 = ((HashSet) var0.get(var2)).iterator();

            while (var1.hasNext()) {
                int var3 = ((Integer) var1.next()).intValue();
                System.out.println(var2 + " " + var3);
            }
        }

    }
}
