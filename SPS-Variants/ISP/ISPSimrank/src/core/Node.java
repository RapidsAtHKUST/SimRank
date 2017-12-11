package core;

import java.util.*;

public class Node {
    int id;
    private List<Edge> out;
    private List<Edge> in;

    public double outDeg;
    public double inDeg;
    public int inSize;
    public int outSize;

    public Node(int id) {
        this.id = id;
        this.out = new ArrayList<>();
        this.in = new ArrayList<>();
        this.inDeg = 0;
        this.outDeg = 0;
        this.inSize = 0;
        this.outSize = 0;
    }

    public void updateDeg() {
        inDeg = 0;
        for (Edge e : in)
            inDeg += e.getWeight();
        inSize = in.size();

        outDeg = 0;
        for (Edge e : out)
            outDeg += e.getWeight();
        outSize = out.size();

    }

    @Override
    public int hashCode() {
        return id;
    }

    @Override
    public boolean equals(Object o) {
        Node n = (Node) o;
        return n.id == this.id;
    }
}
