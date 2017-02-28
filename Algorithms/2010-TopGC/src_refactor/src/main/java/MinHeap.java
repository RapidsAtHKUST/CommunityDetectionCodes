//
// Source code recreated from a .class file by IntelliJ IDEA
// (powered by Fernflower decompiler)
//

public class MinHeap {
    private IntAndDouble[] Heap;
    private int maxsize;
    private int size;

    public static void main(String[] args) {
        MinHeap m = new MinHeap();
        System.out.println(m.size);
        m.insert(new IntAndDouble(2, 2.0D));
        System.out.println(m.size);
        m.insert(new IntAndDouble(1, 3.0D));
        System.out.println(m.size);
        m.insert(new IntAndDouble(0, 5.0D));
        m.print();
        System.out.println(m.size());
        System.out.println("Removed: " + m.removeMin());
        System.out.println(m.size());
        System.out.println("Removed: " + m.removeMin());
        System.out.println(m.size());
        System.out.println("Removed: " + m.removeMin());
        System.out.println(m.size());
    }

    public MinHeap() {
        this(100);
    }

    public MinHeap(int max) {
        this.maxsize = max;
        this.Heap = new IntAndDouble[this.maxsize];
        this.size = 0;
        this.Heap[0] = new IntAndDouble(0, 4.9E-324D);
    }

    private int leftchild(int pos) {
        return 2 * pos;
    }

    private int rightchild(int pos) {
        return 2 * pos + 1;
    }

    private int parent(int pos) {
        return pos / 2;
    }

    private boolean isleaf(int pos) {
        return pos > this.size / 2 && pos <= this.size;
    }

    private void swap(int pos1, int pos2) {
        IntAndDouble tmp = this.Heap[pos1];
        this.Heap[pos1] = this.Heap[pos2];
        this.Heap[pos2] = tmp;
    }

    public void insert(IntAndDouble elem) {
        ++this.size;
        if(this.size >= this.maxsize - 1) {
            this.doubleSize();
        }

        this.Heap[this.size] = elem;

        for(int current = this.size; this.Heap[current].compareTo(this.Heap[this.parent(current)]) > 0; current = this.parent(current)) {
            this.swap(current, this.parent(current));
        }

    }

    public void doubleSize() {
        this.maxsize *= 2;
        IntAndDouble[] newHeap = new IntAndDouble[this.maxsize];

        for(int i = 0; i <= this.size; ++i) {
            newHeap[i] = this.Heap[i];
        }

        this.Heap = newHeap;
    }

    public void print() {
        for(int i = 1; i <= this.size; ++i) {
            System.out.print(this.Heap[i] + " ");
        }

        System.out.println();
    }

    public IntAndDouble peek() {
        return this.Heap[1];
    }

    public IntAndDouble removeMin() {
        this.swap(1, this.size);
        --this.size;
        if(this.size != 0) {
            this.pushdown(1);
        }

        return this.Heap[this.size + 1];
    }

    private void pushdown(int position) {
        while(!this.isleaf(position)) {
            int largestchild = this.leftchild(position);
            if(largestchild < this.size && this.Heap[largestchild].compareTo(this.Heap[largestchild + 1]) <= 0) {
                ++largestchild;
            }

            if(this.Heap[position].compareTo(this.Heap[largestchild]) >= 1) {
                return;
            }

            this.swap(position, largestchild);
            position = largestchild;
        }

    }

    public int size() {
        return this.size;
    }
}
