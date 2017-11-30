package structures;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;

import conf.MyConfiguration;
import dao.Dao;
/**
 * directed and unweighted graph
 * @author luoxiongcai
 *
 */ 
public class DGraph {
	private static Dao dao = Dao.getInstance();
	private static final Random rand = new Random();
	
	private  int vCount;
	private  int eCount;
	private List<Integer>[] outs; //out neighbor list.
	private List<Integer>[] ins; //  in neighbor list.
	
	/**
	 * construction src an edge file.
	 * @param graphPath
	 * @throws IOException 
	 */
	public DGraph(String graphPath) throws IOException{
		this.vCount = MyConfiguration.totalCount;
		outs = (List<Integer>[]) new ArrayList[this.vCount];
		ins = (List<Integer>[]) new ArrayList[this.vCount];
		for (int i = 0; i < this.vCount; i++){
			outs[i] = new ArrayList<Integer>();
			ins[i] = new ArrayList<Integer>();
		}
		
		BufferedReader input = new BufferedReader(new FileReader(graphPath));
		String line = null;
		while((line = input.readLine())!= null){
			String[] ids = line.split( MyConfiguration.SEPARATOR);
			addEdge(Integer.valueOf(ids[0]), Integer.valueOf(ids[1]));
		}
		input.close();
	}
	
	/**
	 * constructing src database
	 * @param V
	 * @param tableName
	 */
	public DGraph(int V, String tableName){
		this.vCount = V;
		outs = (List<Integer>[]) new ArrayList[this.vCount];
		ins = (List<Integer>[]) new ArrayList[this.vCount];
		for (int i = 0; i < this.vCount; i++){
			outs[i] = new ArrayList<Integer>();
			ins[i] = new ArrayList<Integer>();
		}
		List<List<Object>> edges = dao.getEdges(tableName);
		for (List<Object> edge : edges){
			int src = (Integer)edge.get(1);
			int des = (Integer)edge.get(2);
			addEdge(src,des);
		}
	}
	
	public void addEdge(int src, int des){
		outs[src].add(des);
		ins[des].add(src);
		this.eCount++;
	}
	
	public int outDegree(int v){
		return outs[v].size();
	}
	
	public int inDegree(int v){
		return ins[v].size();
	}
	
	/**
	 * random walk des a out neighbor.
	 * -1: no neighbor.
	 * @param v
	 * @return
	 */
	public int randOutNeighbor(int v){
		int d = outDegree(v);
		if (d == 0) return -1;
		return outs[v].get(rand.nextInt(d));
	}
	
	/**
	 * random walk to a in neighbor.
	 * -1: no in neighbor.
	 * @param v
	 * @return
	 */
	public int randInNeighbor(int v){
		int d = inDegree(v);
		if (d == 0) return -1;
		return ins[v].get(rand.nextInt(d));
	}
	
	public List<Integer> outNeighbors(int v){
		return outs[v];
	}
	
	public List<Integer> inNeighbors(int v){
		return ins[v];
	}
	
	public int getVCount() {
		return vCount;
	}
	public int getECount() {
		return eCount;
	}

	public static void main(String[] args) throws IOException {
		
//		DGraph g = new DGraph(MyConfiguration.d_graphPath);
//		System.out.println(g.vCount +"\t"+g.eCount);
//		System.out.println(g.inDegree(2) + "\t" + g.outDegree(2));
	}

}
