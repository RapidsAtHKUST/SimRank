package core;


public class Edge {
	
	// instance members
	
	public Node from;
	public Node to;
	private double weight;
	
	public double getWeight() {
		return weight;
	}
	
	public void setWeight(double weight) {
		this.weight = weight;
	}
	
	public Edge(Node from, Node to) {
		this.from = from;
		this.to = to;
		this.weight = 1;
	}
	
	public Edge(Node from, Node to, double weight) {
		this.from = from;
		this.to = to;
		this.weight = weight;
	}

	@Override
	public int hashCode() {
		return from.id * 97 + to.id * 13;
	}
	
	@Override
	public boolean equals(Object o) {
		// assumption: no duplicate edges of the different weight
		Edge e = (Edge)o;
		return e.from.id == this.from.id && e.to.id == this.to.id;
	}
}
