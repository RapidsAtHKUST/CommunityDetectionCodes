package conga.util.graph;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

public class Vertex {
    public int vertex;
    public float betweenness;
    public Split split;
    public float vB;

    public Vertex(int var1) {
        this.vertex = var1;
        this.betweenness = Float.NEGATIVE_INFINITY;
        this.split = null;
        this.vB = 0.0F;
    }

    public Vertex(int var1, float var2, Split var3, float var4) {
        this.vertex = var1;
        this.betweenness = var2;
        this.split = var3;
        this.vB = var4;
    }

    public String toString() {
        return this.split == null ? this.vertex + " " + this.betweenness : this.vertex + " " + this.betweenness + " " + this.split.value1 + "/" + this.split.value2;
    }
}
