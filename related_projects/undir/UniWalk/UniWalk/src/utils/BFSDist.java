package utils;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.LinkedList;
import java.util.Queue;

import conf.MyConfiguration;

import structures.Graph;

/**
 * query the bfs distance of a given source vertex.
 * @author luoxiongcai
 *
 */
public class BFSDist {

	Graph g;
	int topk = 20;
	byte[][] dist;
	int maxStat;
	public BFSDist(Graph g, int maxStat){
		this.g = g;
		System.out.println(g.getVCount());
		this.maxStat = maxStat;
		this.dist = new byte[maxStat][g.getVCount()];
		bfs();
	} 
	
	private void bfs(){
		for (int u = 0; u < maxStat; u++){
			Queue<Integer> queue = new LinkedList<Integer>();
			queue.offer(u);
			byte hop = 0;
			boolean[] flag = new boolean[g.getVCount()];
			flag[u] = true;
			while(!queue.isEmpty()){
				int size = queue.size();
				for (int i = 0; i< size;i++){
					int v = queue.poll();
					dist[u][v] = hop;
					for (int nei : g.neighbors(v)){
						if (flag[nei]) continue;
						queue.add(nei);
						flag[nei] = true;
					}
					
				}
				hop += 1;
			}
			System.out.println(u+" bfs done");
		}
	}
	
	/**
	 * 
	 * @param topkFile
	 * @param maxStat
	 * @return
	 * @throws IOException 
	 */
	public void distCount(String topkFile) throws IOException{
		int[][] querys = process(topkFile);
		double [] stat = new double[17]; 
		for (int[] query : querys){
			int src = query[0];
			for (int i = 1; i < query.length; i++){
				int d = dist[src][query[i]];
				if (d < stat.length)
					stat[d] += 1.0;
			}
		}
		for (int i = 0; i < stat.length; i++){
			stat[i] = stat[i] / (topk*maxStat);
			System.out.println(i+" hop\t:"+stat[i]);
		}
	}
	
	private int[][] process(String topkFile) throws IOException{
		BufferedReader buff = new BufferedReader(new FileReader(topkFile));
		String line ="";
		int[][] result = new int[maxStat][topk+1];
		int count = 0;
		while((line = buff.readLine())!=null && count< maxStat ){
			String[] tokens = line.split(MyConfiguration.SEPARATOR);
			for (int i = 0; i < tokens.length; i++)
				result[count][i] = Integer.valueOf(tokens[i]);
			count++;
		}
		buff.close();
		return result;
	}
	
	public static void main(String[] args) throws IOException {
		Graph g = new Graph(MyConfiguration.biGraphPath,MyConfiguration.totalCount);
		int maxStat = 500;
		BFSDist bd = new BFSDist(g, maxStat);
		String topkFile = MyConfiguration.basePath+"/Test_Real/reuse_movielens_tag_movie.txt";;	
		bd.distCount(topkFile);
		System.out.println("done");
	}

}
