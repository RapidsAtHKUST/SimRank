package utils;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;

import conf.MyConfiguration;
import dao.Dao;

/**
 * graph generator;
 * @author luoxiongcai
 *
 */
public class GraphGenerator {
	private static Dao dao = Dao.getInstance();
	
	/**
	 * unweighted and undirected graph.
	 * @param V
	 * @param E
	 * @param outPath
	 * @throws IOException
	 */ 
	public static void graph(int V, int E, String outPath) throws IOException{
		BufferedWriter out = new BufferedWriter(new FileWriter(outPath));
		assert(V >0); assert(E > 0); assert (V * (V - 1) / 2 >= E);
		Set<Edge> edges  = new HashSet<Edge>(E);
		Random rand = new Random();
		while (edges.size() < E){
			int src = rand.nextInt(V);
			int des = rand.nextInt(V);
			Edge e = new Edge(Math.min(src,des), Math.max(src, des));
			if (!edges.contains(e)){
				edges.add(e);
				out.append(src + MyConfiguration.SEPARATOR + des+"\r\n");
			}
		}
		out.close();
		System.out.println("edge count: "+ edges.size());
	}

	
	public static void biGraph(int V1, int V2, int E,String outPath) throws IOException{
		BufferedWriter out = new BufferedWriter(new FileWriter(outPath));
		assert(V1 >0); assert(V2 > 0); assert(E > 0); assert (V1 * V2 >= E);
		Set<Edge> edges  = new HashSet<Edge>(E);
		Random rand = new Random();
		while (edges.size() < E){
			int src = rand.nextInt(V1);
			int des = rand.nextInt(V2) + V1;
			Edge e = new Edge(src, des);
			if (!edges.contains(e)){
				edges.add(e);
				out.append(src + MyConfiguration.SEPARATOR + des+"\r\n");
			}
		}
		out.close();
		System.out.println("edge count: "+ edges.size());
	}
	

	
	/**
	 * simple directed and unweighted graph.
	 * @param V
	 * @param E
	 * @param outPath
	 * @throws IOException
	 */
	public static void dGraph(int V, int E, String outPath) throws IOException{
		BufferedWriter out = new BufferedWriter(new FileWriter(outPath));
		assert(V >0);  assert(E > 0); assert (V*V/2 >= E);
		Set<Edge> edges  = new HashSet<Edge>(E);
		Random rand = new Random();
		while (edges.size() < E){
			int src = rand.nextInt(V);
			int des = rand.nextInt(V);
			if (src == des ) continue;
			Edge e = new Edge(src, des);
			if (!edges.contains(e)){
				edges.add(e);
				out.append(src + MyConfiguration.SEPARATOR + des+"\r\n");
			}
		}
		out.close();
		System.out.println("edge count: "+ edges.size());
	}
	
	public static void dGraphToDatabase(String tableName, int V, int E){
		assert(V >0);  assert(E > 0); assert (V*V/2 >= E);
		Set<Edge> edges  = new HashSet<Edge>(E);
		Random rand = new Random();
		while (edges.size() < E){
			int src = rand.nextInt(V);
			int des = rand.nextInt(V);
			if (src == des ) continue;
			Edge e = new Edge(src, des);
			if (!edges.contains(e)){
				edges.add(e);
				dao.insertOneEdge(tableName, src, des);
			}
		}
		System.out.println("edge count: "+ edges.size());
	}
	
	
	
	/**
	 * des indicate a directedEdge;
	 * @author luoxiongcai
	 *
	 */
	private static class Edge{
		private int src;
		private int des ;
		public Edge(int src, int des){
			this.src = Math.min(src, des);
			this.des = Math.max(src, des);
		}
		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + src;
			result = prime * result + des;
			return result;
		}
		@Override
		public boolean equals(Object obj) {
			if (this == obj)
				return true;
			if (obj == null)
				return false;
			if (getClass() != obj.getClass())
				return false;
			Edge other = (Edge) obj;
			if (src != other.src)
				return false;
			if (des != other.des)
				return false;
			return true;
		}
		
		
	}
	public static void main(String[] args) throws IOException {
		//bigraph
//		GraphGenerator.biGraph(Configuration.leftCount, Configuration.rightCount,
//				(int)(Configuration.totalCount*Configuration.avgD / 2),
//				Configuration.biGraphPath);
		
		//sample directed graph
//		GraphGenerator.dGraph(Configuration.d_count, Configuration.d_count * 
//				Configuration.d_avgD , Configuration.d_graphPath);
		
//		GraphGenerator.graph(Configuration.u_count, (int)(Configuration.u_avgD * Configuration.u_count / 2), Configuration.u_graphPath);
	
		int leftCount = 150000;
		int rightCount = 150000;
		int totalCount = leftCount + rightCount;
		int avgD = 5;
		String biGraphPath = MyConfiguration.basePath+"/biGraph_300k_5.txt";
		int totalEdge = totalCount * avgD / 2;
		GraphGenerator.biGraph(leftCount, rightCount,totalEdge,biGraphPath);
		System.out.println("done");
	}

}
