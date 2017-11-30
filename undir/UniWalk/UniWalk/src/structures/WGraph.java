package structures;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;

import lxctools.LxcArrays;

import conf.MyConfiguration;
/**
 * undirected and weighted graph
 * @author luoxiongcai
 *
 */
public class WGraph {
	private static final Random rand = new Random();
	
	private  int vCount;
	private  int eCount;
	private List<Integer>[] adjs; // adjacent list.
	private Map<Integer, Double>[] edges; // adjacent edges with weight.
	
	/**
	 * construction from an edge file.
	 * @param graphPath
	 * @throws IOException 
	 */ 
	public WGraph(String graphPath, int V) throws IOException{
		this.vCount = V;
		adjs = (List<Integer>[]) new ArrayList[this.vCount];
		edges = (Map<Integer, Double>[]) new HashMap[this.vCount];
		for (int i = 0; i < this.vCount; i++){
			adjs[i] = new ArrayList<Integer>();
			edges[i] = new HashMap<Integer, Double>();
		}
		BufferedReader input = new BufferedReader(new FileReader(graphPath));
		String line = null;
		while((line = input.readLine())!= null){
			String[] ids = line.split( MyConfiguration.SEPARATOR);
			if (ids.length == 2)
				addEdge(Integer.valueOf(ids[0]), Integer.valueOf(ids[1]));
			else if (ids.length == 3)
				addEdge(Integer.valueOf(ids[0]), Integer.valueOf(ids[1]), Double.valueOf(ids[2]));
			else throw new IOException("invalid edge file: should be [src, dst] or  [src, dst, w]");
		}
		input.close();
	}
	
	
	public void addEdge(int src, int dst){
		addEdge(src, dst,0.0);
	}
	
	public void addEdge(int src, int dst, double w){
		adjs[src].add(dst);
		adjs[dst].add(src);
		edges[src].put(dst, w);
		edges[dst].put(src, w);
		this.eCount++;	
	}
	
	public Map<Integer, Double> getAllEdgeWeight(int u){
		return edges[u];
	}
	
	public int degree(int v){
		return adjs[v].size();
	}
	
	/**
	 * random walk to a neighbor.
	 * -1: no neighbor.
	 * @param v
	 * @return
	 */
	public int randNeighbor(int v){
		int d = degree(v);
		if (d == 0) return -1;
		return adjs[v].get(rand.nextInt(d));
	}
	
	public int randNeighborByWeight(int v){
		int d = degree(v);
		if (d == 0) return -1;
		double[] weight = new double[d];
		double sum = 0;
		for (int i = 0; i < d; i++){
			weight[i] = adjs[v].get(i);
			sum += weight[i];
		}
		for (int i = 0; i < d; i++){
			weight[i] /= sum;
			weight[i] += ( i > 0 ? weight[i-1] : 0);
		}
		double r = Math.random();
		int index = LxcArrays.insertPoint(weight, r);
		return adjs[v].get(index);
	}
	
	public List<Integer> neighbors(int v){
		return adjs[v];
	}
	
	/**
	 * get the adjacent edges(with weight)
	 * @param v
	 * @return
	 */
	public Map<Integer, Double> edges(int v){
		return edges[v];
	}
	
	public int getVCount() {
		return vCount;
	}
	
	public int getECount() {
		return eCount;
	}
	
	/**
	 * modify the edge weight to the given w.
	 * @param src
	 * @param dst
	 * @param w
	 */
	public void updateWeight(int src, int dst, double w){
		edges[src].put(dst, w);
	}
	
	/**
	 * sum aggregator.
	 * @param src
	 * @param dst
	 * @param w
	 */
	public void updateMaxWeight(int src,int dst, double w){
		double old = edges[src].get(dst);
		edges[src].put(dst,Math.max(old, w));
	}
	
	/**
	 * add w to the old weight of edge [src, dst].
	 * @param src
	 * @param dst
	 * @param w
	 */
	public void accumulateWeight(int src, int dst, double w){
		edges[src].put(dst, edges[src].get(dst) + w);
	}
	

	public static void main(String[] args) throws IOException {
		WGraph g = new WGraph(MyConfiguration.biGraphPath,MyConfiguration.totalCount);
		System.out.println(g.vCount +"\t"+g.eCount);
		System.out.println(g.degree(1));
	}

}
