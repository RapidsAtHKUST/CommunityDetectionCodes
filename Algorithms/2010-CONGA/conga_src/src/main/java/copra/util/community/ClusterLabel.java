package copra.util.community;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.ArrayList;
import java.util.HashSet;
import java.util.Iterator;
import java.util.Map;
import java.util.Random;
import java.util.TreeMap;
import java.util.TreeSet;

public class ClusterLabel {
    private Map<Integer, Float> label;
    private float weight;
    private float overlap;
    private Random rand;

    public ClusterLabel(int var1, float var2) {
        this.initialize(var1, var2);
    }

    public ClusterLabel(int var1, float var2, int var3, boolean var4) {
        this.initialize(var1, var2);
        if(var4) {
            this.label.put(Integer.valueOf(var3), Float.valueOf(1.0F));
            this.weight = 1.0F;
        }

    }

    private void initialize(int var1, float var2) {
        this.label = new TreeMap();
        this.weight = 0.0F;
        if(var2 == 0.0F) {
            this.overlap = (float)var1;
        } else {
            this.overlap = var2;
        }

        this.rand = new Random();
    }

    public Map<Integer, Float> getLabel() {
        return this.label;
    }

    public float getWeight() {
        return this.weight;
    }

    public TreeSet<Integer> labelSet() {
        return new TreeSet(this.label.keySet());
    }

    public Map<Integer, Float> labelMap() {
        return this.label;
    }

    public void add(int var1) {
        if(this.label.containsKey(Integer.valueOf(var1))) {
            System.out.println("Trying to add " + var1 + " more than once");
        } else {
            this.label.put(Integer.valueOf(var1), Float.valueOf(1.0F));
            ++this.weight;
        }

    }

    public boolean sameAs(ClusterLabel var1) {
        Map var3 = var1.getLabel();
        Iterator var4 = this.label.keySet().iterator();

        int var2;
        do {
            if(!var4.hasNext()) {
                var4 = var3.keySet().iterator();

                do {
                    if(!var4.hasNext()) {
                        return true;
                    }

                    var2 = ((Integer)var4.next()).intValue();
                } while(this.label.containsKey(Integer.valueOf(var2)));

                return false;
            }

            var2 = ((Integer)var4.next()).intValue();
            if(!var3.containsKey(Integer.valueOf(var2))) {
                return false;
            }
        } while((double)((Float)this.label.get(Integer.valueOf(var2))).floatValue() >= (double)((Float)var3.get(Integer.valueOf(var2))).floatValue() - 0.001D && (double)((Float)this.label.get(Integer.valueOf(var2))).floatValue() <= (double)((Float)var3.get(Integer.valueOf(var2))).floatValue() + 0.001D);

        return false;
    }

    public void neighbour(ClusterLabel var1, float var2) {
        if(var1.getWeight() > 0.0F) {
            Map var4 = var1.getLabel();
            Iterator var5 = var4.keySet().iterator();

            while(var5.hasNext()) {
                int var3 = ((Integer)var5.next()).intValue();
                if(this.label.containsKey(Integer.valueOf(var3))) {
                    this.label.put(Integer.valueOf(var3), Float.valueOf(((Float)this.label.get(Integer.valueOf(var3))).floatValue() + ((Float)var4.get(Integer.valueOf(var3))).floatValue() * var2));
                } else {
                    this.label.put(Integer.valueOf(var3), Float.valueOf(((Float)var4.get(Integer.valueOf(var3))).floatValue() * var2));
                }
            }

            this.weight += var2;
        }

    }

    public void noMore() {
        this.reduce();
        this.normalize();
    }

    private void reduce() {
        HashSet var3 = new HashSet();
        float var4 = Float.NEGATIVE_INFINITY;
        float var5 = 0.0F;
        float var6 = 1.0F / this.overlap;
        Iterator var2 = this.label.keySet().iterator();

        int var1;
        while(var2.hasNext()) {
            var1 = ((Integer)var2.next()).intValue();
            if(((Float)this.label.get(Integer.valueOf(var1))).floatValue() / this.weight < var6) {
                if(((Float)this.label.get(Integer.valueOf(var1))).floatValue() > var4) {
                    var4 = ((Float)this.label.get(Integer.valueOf(var1))).floatValue();
                }

                var3.add(Integer.valueOf(var1));
            } else {
                var5 += ((Float)this.label.get(Integer.valueOf(var1))).floatValue();
            }
        }

        if(var5 > 0.0F) {
            var2 = var3.iterator();

            while(var2.hasNext()) {
                var1 = ((Integer)var2.next()).intValue();
                this.label.remove(Integer.valueOf(var1));
            }

            this.weight = var5;
        } else {
            ArrayList var7 = new ArrayList();
            var2 = var3.iterator();

            while(var2.hasNext()) {
                var1 = ((Integer)var2.next()).intValue();
                if(((Float)this.label.get(Integer.valueOf(var1))).floatValue() == var4) {
                    var7.add(Integer.valueOf(var1));
                }
            }

            this.label.clear();
            int var8 = var7.size();
            var1 = ((Integer)var7.get(this.rand.nextInt(var8))).intValue();
            this.label.put(Integer.valueOf(var1), Float.valueOf(1.0F));
            this.weight = 1.0F;
        }

    }

    public void normalize() {
        if(this.weight > 0.0F) {
            Iterator var2 = this.label.keySet().iterator();

            while(var2.hasNext()) {
                int var1 = ((Integer)var2.next()).intValue();
                this.label.put(Integer.valueOf(var1), Float.valueOf(((Float)this.label.get(Integer.valueOf(var1))).floatValue() / this.weight));
            }

            this.weight = 1.0F;
        }

    }

    public String toString() {
        String var3 = "";

        int var1;
        float var2;
        for(Iterator var4 = this.label.keySet().iterator(); var4.hasNext(); var3 = var3 + String.format("%d:%.3f", new Object[]{Integer.valueOf(var1), Float.valueOf(var2)})) {
            var1 = ((Integer)var4.next()).intValue();
            var2 = ((Float)this.label.get(Integer.valueOf(var1))).floatValue();
            if(!"".equals(var3)) {
                var3 = var3 + "/";
            }
        }

        return var3;
    }
}
