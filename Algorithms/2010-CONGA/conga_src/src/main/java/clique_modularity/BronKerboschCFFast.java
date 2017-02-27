package clique_modularity;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

public class BronKerboschCFFast {
    private final ArrayList<HashSet<Integer>> graph;
    private Collection<Set<Integer>> cliques;

    public BronKerboschCFFast(ArrayList<HashSet<Integer>> var1) {
        this.graph = var1;
    }

    public Collection<Set<Integer>> getBiggestMaximalCliques() {
        this.getAllMaximalCliques();
        int var1 = 0;
        ArrayList var2 = new ArrayList();
        Iterator var3 = this.cliques.iterator();

        Set var4;
        while(var3.hasNext()) {
            var4 = (Set)var3.next();
            if(var1 < var4.size()) {
                var1 = var4.size();
            }
        }

        var3 = this.cliques.iterator();

        while(var3.hasNext()) {
            var4 = (Set)var3.next();
            if(var1 == var4.size()) {
                var2.add(var4);
            }
        }

        return var2;
    }

    public Collection<Set<Integer>> getAllMaximalCliques() {
        this.cliques = new ArrayList();
        ArrayList var1 = new ArrayList();
        ArrayList var2 = new ArrayList();
        ArrayList var3 = new ArrayList();

        for(int var4 = 0; var4 < this.graph.size(); ++var4) {
            if(((HashSet)this.graph.get(var4)).size() > 0) {
                var2.add(Integer.valueOf(var4));
            }
        }

        this.findCliques(var1, var2, var3);
        return this.cliques;
    }

    private Set<Integer> findCliques(List<Integer> var1, List<Integer> var2, List<Integer> var3) {
        HashSet var4 = new HashSet();
        new HashSet();
        ArrayList var7 = new ArrayList(var2);
        if(!this.end(var2, var3)) {
            Iterator var8 = var7.iterator();

            while(true) {
                Integer var9;
                do {
                    if(!var8.hasNext()) {
                        return var4;
                    }

                    var9 = (Integer)var8.next();
                } while(var4.contains(var9));

                ArrayList var10 = new ArrayList();
                ArrayList var11 = new ArrayList();
                var1.add(var9);
                var2.remove(var9);
                Iterator var12 = ((HashSet)this.graph.get(var9.intValue())).iterator();

                Integer var13;
                while(var12.hasNext()) {
                    var13 = (Integer)var12.next();
                    if(var2.contains(var13) && !var4.contains(var13)) {
                        var10.add(var13);
                    }
                }

                var12 = var3.iterator();

                while(var12.hasNext()) {
                    var13 = (Integer)var12.next();
                    if(((HashSet)this.graph.get(var9.intValue())).contains(var13)) {
                        var11.add(var13);
                    }
                }

                Object var5;
                if(var10.isEmpty() && var11.isEmpty()) {
                    if(var1.size() > 2) {
                        HashSet var6 = new HashSet(var1);
                        this.cliques.add(var6);
                        var5 = var6;
                    } else {
                        var5 = new HashSet();
                    }
                } else {
                    var5 = this.findCliques(var1, var10, var11);
                }

                var4.addAll((Collection)var5);
                var1.removeAll((Collection)var5);
                if(!((Set)var5).contains(var9)) {
                    var3.add(var9);
                }

                var1.remove(var9);
            }
        } else {
            return var4;
        }
    }

    private void check(String var1, Set<Integer> var2, List<Integer> var3) {
        for(int var4 = 0; var4 < var3.size(); ++var4) {
            if(var2.contains(var3.get(var4))) {
                System.out.println(var1 + " " + var2 + " " + var3);
                return;
            }
        }

    }

    private boolean end(List<Integer> var1, List<Integer> var2) {
        boolean var3 = false;
        Iterator var5 = var2.iterator();

        while(var5.hasNext()) {
            Integer var6 = (Integer)var5.next();
            int var4 = 0;
            Iterator var7 = var1.iterator();

            while(var7.hasNext()) {
                Integer var8 = (Integer)var7.next();
                if(((HashSet)this.graph.get(var6.intValue())).contains(var8)) {
                    ++var4;
                }
            }

            if(var4 == var1.size()) {
                var3 = true;
            }
        }

        return var3;
    }
}
