package clique_modularity;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.ArrayList;
import java.util.Comparator;
import java.util.HashSet;
import java.util.Iterator;
import java.util.TreeSet;

class CommPairSet {
    ArrayList<HashSet<CommPair>> added = new ArrayList();
    TreeSet<CommPair> commPairs = new TreeSet(new Comparator() {
        public int compare(CommPair var1, CommPair var2) {
            return RebuildCommunities.detcomp(var1.modInc, var1.comm1, var1.comm2, var2.modInc, var2.comm1, var2.comm2);
        }
    });

    public CommPairSet(int var1) {
        for(int var2 = 0; var2 < var1; ++var2) {
            this.added.add(new HashSet());
        }

    }

    public boolean add(CommPair var1) {
        ((HashSet)this.added.get(var1.comm1)).add(var1);
        ((HashSet)this.added.get(var1.comm2)).add(var1);
        return this.commPairs.add(var1);
    }

    public void remove(int var1) {
        int var2;
        CommPair var3;
        for(Iterator var4 = ((HashSet)this.added.get(var1)).iterator(); var4.hasNext(); ((HashSet)this.added.get(var2)).remove(var3)) {
            var3 = (CommPair)var4.next();
            this.commPairs.remove(var3);
            if(var3.comm1 == var1) {
                var2 = var3.comm2;
            } else {
                var2 = var3.comm1;
            }
        }

        ((HashSet)this.added.get(var1)).clear();
    }

    public CommPair best() {
        return (CommPair)this.commPairs.last();
    }

    public String toString() {
        String var2 = "";

        CommPair var1;
        for(Iterator var3 = this.commPairs.iterator(); var3.hasNext(); var2 = var2 + var1.comm1 + "/" + var1.comm2 + "=" + var1.modInc + " ") {
            var1 = (CommPair)var3.next();
        }

        return var2;
    }
}
