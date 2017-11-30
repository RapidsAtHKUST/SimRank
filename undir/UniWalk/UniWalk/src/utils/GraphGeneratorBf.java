package utils;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Random;


import com.google.common.hash.BloomFilter;
import com.google.common.hash.Funnel;
import com.google.common.hash.PrimitiveSink;

import conf.MyConfiguration;

/**
 * use bloom filter and  multithreads to speed up for extremely huge graphs.
 * @author luoxiongcai
 *
 */
public class GraphGeneratorBf implements Runnable{
	public static final int V = 700000000;  // should be times of 100000.
	public static final int D = 5;
	public   BloomFilter<Edge> edges_bf ;
	

	private int begin;//include.
	private int end; //exclude.
	private int E;
	
	private Random rand = new Random();
	private String dir = "K:/lxcdata/lshrank_data/biGraph_700m_5";
	private String outPath ;
	private BufferedWriter outWriter;
	public GraphGeneratorBf(int begin, int end, int E, int flag){
		this.begin = begin;
		this.end = end; 
		this.E = E;
		edges_bf = BloomFilter.create(new EdgeFunnel(),2*E,0.00000001);
		outPath = dir +"/"+flag+".txt";
	}
	@Override
	public void run() {
		try {
			outWriter = new BufferedWriter(new FileWriter(outPath));
		} catch (IOException e1) {
			// TODO Auto-generated catch block
			e1.printStackTrace();
		}
		int count = 0 ;
		while (count < E){
			int src = rand.nextInt(end - begin) + begin;
			int des = rand.nextInt(V/2) + V/2;
			Edge e = new Edge(src, des);

			if (!edges_bf.mightContain(e)){
				edges_bf.put(e);
				count++;
				try {
					outWriter.append(src + MyConfiguration.SEPARATOR + des+"\r\n");
				} catch (IOException e1) {
					// TODO Auto-generated catch block
					e1.printStackTrace();
				}
			}
		}
		try {
			outWriter.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	
	// 
	private static class Edge{
		int src;
		int dst;
		public Edge(int src, int dst){
			this.src = src < dst ? src : dst;
			this.dst = src >= dst ? src : dst;
		}
		
		public int getSrc() {
			return src;
		}

		public void setSrc(int src) {
			this.src = src;
		}

		public int getDst() {
			return dst;
		}

		public void setDst(int dst) {
			this.dst = dst;
		}

		@Override
		public int hashCode() {
			final int prime = 31;
			int result = 1;
			result = prime * result + dst;
			result = prime * result + src;
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
			if (dst != other.dst)
				return false;
			if (src != other.src)
				return false;
			return true;
		}
		
	}
	
	private static class EdgeFunnel implements Funnel<Edge>{

		@Override
		public void funnel(Edge e, PrimitiveSink into) {
			into.putInt(e.getSrc());
			into.putInt(e.getDst());
		}
		
	}

	
	public static void main(String[] args){
		int thr = 16;
		long edges = GraphGeneratorBf.V;
			edges = edges* D / 2;
		System.out.println("V: "+GraphGeneratorBf.V);
		System.out.println("edges: " + edges);
		long batch = edges / thr;
		int batchV = GraphGeneratorBf.V / 2 / 16;
		System.out.println("batchs: "+ batch);
		for (int i = 0; i < thr; i++){
			Thread thread = new Thread(new GraphGeneratorBf(i * batchV,(i+1)*batchV,(int)batch, i));
			thread.start();
			System.out.println("thread start: "+i);
		}
	}
}
