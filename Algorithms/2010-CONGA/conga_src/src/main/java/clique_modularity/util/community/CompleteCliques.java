package clique_modularity.util.community;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import clique_modularity.input_output.ReadEdges;

import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;

public class CompleteCliques {
    private static String filename = "tests/graphNew.txt";

    public CompleteCliques() {
    }

    public static ArrayList<HashSet<Integer>> completeCliques(ArrayList<HashSet<Integer>> var0, int var1, int var2) throws FileNotFoundException {
        ArrayList var10 = new ArrayList();

        int var8;
        for(var8 = 0; var8 < var0.size(); ++var8) {
            var10.add(new HashSet());
        }

        for(var8 = 0; var8 < var0.size(); ++var8) {
            Iterator var11 = ((HashSet)var0.get(var8)).iterator();

            while(var11.hasNext()) {
                int var9 = ((Integer)var11.next()).intValue();
                ((HashSet)var10.get(var8)).add(Integer.valueOf(var9));
            }
        }

        int var3 = var10.size();
        ReadEdges var15 = new ReadEdges();
        ArrayList var12 = var15.readGraph(filename, var1);
        System.out.println("comCliques.size: " + var10.size());
        Iterator var13 = ((HashSet)var12.get(0)).iterator();
        byte var6;
        if(var13.hasNext()) {
            var6 = 0;
        } else {
            var6 = 1;
        }

        ArrayList var16 = new ArrayList();

        int var4;
        int var5;
        for(var4 = var6; var4 < var1; ++var4) {
            int var7 = 0;

            for(var5 = 0; var5 < var3; ++var5) {
                if(((HashSet)var10.get(var5)).contains(Integer.valueOf(var4))) {
                    ++var7;
                }
            }

            if(var7 == 0) {
                var16.add(Integer.valueOf(var4));
            }
        }

        int var17 = var16.size();
        boolean var20 = false;

        for(var4 = 0; var4 < var17; ++var4) {
            int var18 = ((Integer)var16.get(var4)).intValue();
            if(var18 >= 0) {
                for(var5 = var4 + 1; var5 < var17; ++var5) {
                    int var19 = ((Integer)var16.get(var5)).intValue();
                    if(var19 >= 0 && ((HashSet)var12.get(var18)).contains(Integer.valueOf(var19))) {
                        var10.add(new HashSet());
                        ((HashSet)var10.get(var2)).add(Integer.valueOf(var18));
                        ((HashSet)var10.get(var2)).add(Integer.valueOf(var19));
                        var16.set(var4, Integer.valueOf(-1));
                        var16.set(var5, Integer.valueOf(-1));
                        ++var2;
                        var20 = true;
                        break;
                    }
                }

                if(!var20) {
                    var10.add(new HashSet());
                    ((HashSet)var10.get(var2)).add(Integer.valueOf(var18));
                    var16.set(var4, Integer.valueOf(-1));
                    ++var2;
                }
            }

            var20 = false;
        }

        return var10;
    }
}
