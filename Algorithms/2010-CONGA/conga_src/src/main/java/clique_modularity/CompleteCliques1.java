package clique_modularity;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.io.FileNotFoundException;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;

public class CompleteCliques1 {
    public CompleteCliques1() {
    }

    public static ArrayList<HashSet<Integer>> completeCliques1(ArrayList<HashSet<Integer>> var0, int var1, int var2, String var3) throws FileNotFoundException {
        ArrayList var12 = new ArrayList();

        int var9;
        for(var9 = 0; var9 < var0.size(); ++var9) {
            var12.add(new HashSet());
        }

        for(var9 = 0; var9 < var0.size(); ++var9) {
            Iterator var13 = ((HashSet)var0.get(var9)).iterator();

            while(var13.hasNext()) {
                int var10 = ((Integer)var13.next()).intValue();
                ((HashSet)var12.get(var9)).add(Integer.valueOf(var10));
            }
        }

        int var4 = var12.size();
        ReadEdges var16 = new ReadEdges();
        ArrayList var11 = var16.readGraph(var3, var1);
        Iterator var14 = ((HashSet)var11.get(0)).iterator();
        byte var7;
        if(var14.hasNext()) {
            var7 = 0;
        } else {
            var7 = 1;
        }

        ArrayList var17 = new ArrayList();

        int var5;
        for(var5 = var7; var5 < var1; ++var5) {
            int var8 = 0;

            for(int var6 = 0; var6 < var4; ++var6) {
                if(((HashSet)var12.get(var6)).contains(Integer.valueOf(var5))) {
                    ++var8;
                }
            }

            if(var8 == 0) {
                var17.add(Integer.valueOf(var5));
            }
        }

        int var18 = var17.size();

        for(var5 = 0; var5 < var18; ++var5) {
            int var19 = ((Integer)var17.get(var5)).intValue();
            var12.add(new HashSet());
            ((HashSet)var12.get(var2)).add(Integer.valueOf(var19));
            ++var2;
        }

        return var12;
    }
}
