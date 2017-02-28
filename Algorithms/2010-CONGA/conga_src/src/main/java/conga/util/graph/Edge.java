package conga.util.graph;//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

public class Edge {
    public int vertex1;
    public int vertex2;
    public float betweenness;

    public Edge(int var1, int var2) {
        this.vertex1 = var1;
        this.vertex2 = var2;
        this.betweenness = 0.0F;
    }

    public Edge(int var1, int var2, float var3) {
        this.vertex1 = var1;
        this.vertex2 = var2;
        this.betweenness = var3;
    }

    public String toString() {
        return this.vertex1 + "/" + this.vertex2 + " " + this.betweenness;
    }
}
