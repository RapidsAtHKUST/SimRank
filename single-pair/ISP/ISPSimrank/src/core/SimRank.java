package core;

import java.util.*;

import util.*;





public class SimRank {
	protected Graph g;
	private double Sim_ab;
	private double c;
	private final int MAX_ITER = Config.depth;
	private final int NUM_THREADS = 2;
	
	
	public double getSim(int a, int b) {
		if(a==b)
			return 1.0;
		Sim_ab = 0;
//		System.out.println("Line 30: g.numNodes()" + g.numNodes());
	
		HashMap<Integer,HashMap<Integer,Double>> Qk_prev = new HashMap<Integer,HashMap<Integer,Double>>();
		HashMap<Integer,Double> Q_0a = new HashMap<Integer,Double>();
		Q_0a.put(b, 1.0);
		Qk_prev.put(a, Q_0a);
		boolean stop_iteration = true;
		for (int k = 1; k < MAX_ITER; k++) {

			//System.out.println("\nIter: " + k);
			HashMap<Integer,HashMap<Integer,Double>> Qk = new HashMap<Integer,HashMap<Integer,Double>>();
			for (Map.Entry<Integer, HashMap<Integer,Double>> row : Qk_prev.entrySet()) {
			    int i = row.getKey();
			    HashMap<Integer,Double> Qij = row.getValue();
			    for (Map.Entry<Integer,Double> entry_ij : Qij.entrySet()) {
			    	int j = entry_ij.getKey();
			    	if (i!=j){
				    	boolean updated = g.TiTj(entry_ij.getValue(), i, j, Qk);
				    	if (updated)
				    		stop_iteration = false;
			    	}
			    }
			}
			double M = 0;
			for (Map.Entry<Integer, HashMap<Integer,Double>> row : Qk.entrySet()) {
			    Integer i = row.getKey();
			    HashMap<Integer,Double> Qij = row.getValue();
			    if (Qij.containsKey(i)){
			    	M += Qij.get(i);
			    }
			}
			Sim_ab += Math.pow(c, k)*M;
//			Graph.printGraph(Qk);
			Qk_prev = Qk;
//			System.out.println("k="+k+"Sim(a,b) = "+Sim_ab);
			if (stop_iteration)
				break;
		}
		return Sim_ab;
	}
	
	public SimRank(Graph g, double c) {
		// c: damping_factor=0.75
		this.c = c;
		this.g = g;
		
	}
	

	
	


//	private class SThread extends Thread {
//		
//		private double[][] output;
//		private int indexMod;
//		
//		public SThread(int indexMod, double[][] output) {
//			this.indexMod = indexMod;
//			this.output = output;
//		}
//		
//		@Override
//		public void run() {
//			//System.out.println("Line 118: graph "+ g.numNodes());
//			for (int i = 0; i < g.numNodes(); i ++) {
//			//	System.out.println("Line 121: " + i);
//				if (i % NUM_THREADS != indexMod)
//					continue;
//				
//				if (i % 1000 == 0)
//					System.out.print( i + " " );
//				
//				Node x = g.getNodeByIndex(i);
//	//			System.out.println("Line 129: Current node is: " + x.id);
//				double[] partial = new double[g.numNodes()];
//				Arrays.fill(partial, -1);
//				
//				for (int j = 0; j < i; j++) {
//					double sum = 0;					
//					Node y = g.getNodeByIndex(j);
//	//				System.out.println("Node y is: " + y.id);
//					for (Edge ey : y.in) {
//						int index = g.getIndexById(ey.from.id);
//	//					System.out.println("index of y's inneighbor " + ey.from.id + " is: " + index);
//						if (partial[index] < 0) {
//							partial[index] = 0;
//							for (Edge ex: x.in)
//								partial[index] += getSimRank(g.getIndexById(ex.from.id), g.getIndexById(ey.from.id)) * ex.getWeight();
//						}
//						sum += partial[index] * ey.getWeight();
//					}
//					output[i][j] = sum * c / x.inDeg / y.inDeg;
//	//				System.out.println("sum "+ sum + " node is: " + x.id+ " x.inDeg "+ x.inDeg+ " y.outDeg "+ y.outDeg+ " output " + output[i][j]);
//				}
//			}
//		}
//	}
	
	

}

