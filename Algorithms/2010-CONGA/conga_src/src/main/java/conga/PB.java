package conga;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;

class PB {
    public HashMap<Integer, Integer> entryOf;
    public int[] vertexOf;
    public float[][] matrix;
    public int size;
    public Vertex vertex;
    public float oldB = 0.0F;

    public PB(HashSet<Integer> var1, Vertex var2) {
        int var4 = 0;
        Iterator var5 = var1.iterator();
        this.size = var1.size();
        this.vertexOf = new int[this.size];

        for(this.entryOf = new HashMap(); var5.hasNext(); ++var4) {
            int var3 = ((Integer)var5.next()).intValue();
            this.vertexOf[var4] = var3;
            this.entryOf.put(Integer.valueOf(var3), Integer.valueOf(var4));
        }

        this.matrix = new float[this.size][this.size];
        this.vertex = var2;
    }

    public void replace(int var1, int var2) {
        int var3 = ((Integer)this.entryOf.remove(Integer.valueOf(var1))).intValue();
        this.entryOf.put(Integer.valueOf(var2), Integer.valueOf(var3));
        this.vertexOf[var3] = var2;
    }

    public int remove(int var1) {
        int var5 = ((Integer)this.entryOf.remove(Integer.valueOf(var1))).intValue();
        int var2;
        if(var5 < this.size - 1) {
            int var4 = this.vertexOf[this.size - 1];
            this.vertexOf[var5] = var4;
            this.entryOf.put(Integer.valueOf(var4), Integer.valueOf(var5));
            this.matrix[var5] = this.matrix[this.size - 1];

            for(var2 = 0; var2 < this.size - 1; ++var2) {
                this.matrix[var2][var5] = this.matrix[var2][this.size - 1];
            }
        }

        --this.size;

        for(var2 = 1; var2 < this.size; ++var2) {
            for(int var3 = 0; var3 < var2; ++var3) {
                if(this.matrix[var2][var3] != 0.0F) {
                    return this.size;
                }
            }
        }

        return this.size;
    }

    public void print(String var1) {
        int[] var2 = Arrays.copyOf(this.vertexOf, this.size);
        Arrays.sort(var2);
        String var8 = var1 + ": [";
        boolean var9 = false;

        for(int var3 = 0; var3 < this.size - 1; ++var3) {
            for(int var4 = var3 + 1; var4 < this.size; ++var4) {
                int var5 = var2[var3];
                int var6 = var2[var4];
                float var7 = this.matrix[((Integer)this.entryOf.get(Integer.valueOf(var5))).intValue()][((Integer)this.entryOf.get(Integer.valueOf(var6))).intValue()];
                if((double)var7 < -0.005D || (double)var7 > 0.005D) {
                    System.out.format("%s%d/%d=%.2f", new Object[]{var8, Integer.valueOf(var5), Integer.valueOf(var6), Float.valueOf(var7)});
                    var8 = " ";
                    var9 = true;
                }
            }
        }

        if(var9) {
            System.out.println("]");
        }

    }
}
