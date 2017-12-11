package structures;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import conf.MyConfiguration;
/**
 * undirected and unweighted graph
 * @author luoxiongcai
 *
 */
public class Graph {
	private static final Random rand = new Random();
	
	private  int vCount;
	private  int eCount;
	private List<Integer>[] adjs; // adjacent list.
	
	/**
	 * construction from an edge file.
	 * @param graphPath
	 * @throws IOException 
	 */
	public Graph(String graphPath, int V) throws IOException{
		this.vCount = V;
		adjs = (List<Integer>[]) new ArrayList[this.vCount];
		System.out.println("vCount: " + this.vCount);
		for (int i = 0; i < this.vCount; i++){
			adjs[i] = new ArrayList<Integer>();
			if(i%10000000==0){
				System.out.println("new ArrayList i: " + i);
			}
		}
		
		BufferedReader input = new BufferedReader(new FileReader(graphPath));
		String line = null;
		long cnt = 0;
		while((line = input.readLine())!= null){
			String[] ids = line.split( MyConfiguration.SEPARATOR);
			addEdge(Integer.valueOf(ids[0]), Integer.valueOf(ids[1]));
			cnt += 1;
			if(cnt%20000000==0){
				System.out.println("input edges' cnt: " + cnt);
			}
		}
		input.close();
	}
	
	/**
	 * construction from program. 
	 * Add edge one by one by yourself!
	 * @param vCount
	 */
	public Graph(int vCount){
		
	}
	
	public void addEdge(int from, int to){
		adjs[from].add(to);
		adjs[to].add(from);
		this.eCount++;
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
	
	public List<Integer> neighbors(int v){
		return adjs[v];
	}
	
	public int getVCount() {
		return vCount;
	}
	public int getECount() {
		return eCount;
	}

	public static void main(String[] args) throws IOException {
//		Graph g = new Graph(MyConfiguration.biGraphPath,MyConfiguration.totalCount);
//		System.out.println(g.vCount +"\t"+g.eCount);
//		System.out.println(g.degree(1));
	}

}
