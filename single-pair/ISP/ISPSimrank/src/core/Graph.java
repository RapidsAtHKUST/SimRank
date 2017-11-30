package core;



import java.util.*;

import util.Config;
import util.TextReader;

public class Graph {
	private final double h = Config.stopRea;
	private HashMap<Integer,HashMap<Integer,Double>> T;
	private HashMap<Integer,Integer> sum_i;
	public static void printGraph(HashMap<Integer,HashMap<Integer,Double>> g){
		int max_idx = 0;
		for (Map.Entry<Integer, HashMap<Integer,Double>> entry_i : g.entrySet()) {
		    int i = entry_i.getKey();
		    if (i > max_idx)
		    	max_idx = i;
		    HashMap<Integer,Double> Tij = entry_i.getValue();
		    for (Map.Entry<Integer,Double> entry_ij : Tij.entrySet()) {
		    	int j = entry_ij.getKey();
		    	if (j > max_idx)
		    		max_idx = j;
		    }
		}
		for (int i = 0; i <= max_idx; i++){
			for (int j = 0; j <= max_idx; j++){
				if (g.containsKey(i)&&(g.get(i).containsKey(j))){
					System.out.format("\t%.3f",g.get(i).get(j));
				}
				else System.out.print("\t0");
			}
			System.out.println();
		}
	}
	public Graph() {
		this.T = new HashMap<Integer,HashMap<Integer,Double>>();
		this.sum_i = new HashMap<Integer,Integer> ();
	}
	public boolean TiTj(double Qkij, int i, int j, HashMap<Integer,HashMap<Integer,Double>> Qk_next){
		if (T.containsKey(i) && T.containsKey(j)){
			HashMap<Integer,Double> TiT = T.get(i);
			HashMap<Integer,Double> Tj = T.get(j);
			for (Map.Entry<Integer,Double> TiT_k : TiT.entrySet()) {
				int row = TiT_k.getKey();
				double TiT_k_val = TiT_k.getValue();
		    	for (Map.Entry<Integer,Double> Tj_k : Tj.entrySet()) {
		    		int col = Tj_k.getKey();
		    		double Tj_k_val = Tj_k.getValue();
		    		HashMap<Integer,Double> Qk_next_row;
		    		if (Qk_next.containsKey(row)){
		    			Qk_next_row = Qk_next.get(row);
		    			if (Qk_next_row.containsKey(col)){
		    				double old_val = Qk_next_row.get(col);
		    				Qk_next_row.put(col, old_val+Qkij*TiT_k_val*Tj_k_val);
		    			}
		    			else{
		    				double new_val = Qkij*TiT_k_val*Tj_k_val;
		    				if (new_val > h)
		    					Qk_next_row.put(col, new_val);
		    			}
		    		}
		    		else{
		    			Qk_next_row = new HashMap<Integer,Double>();
		    			double new_val = Qkij*TiT_k_val*Tj_k_val;
	    				if (new_val > h)
	    					Qk_next_row.put(col, new_val);
		    			Qk_next.put(row, Qk_next_row);
		    		}
		    	}
		    }
			return true;				
		}
		else return false;
	}
	public Graph(String nodeFile, String edgeFile) throws Exception {
		// load nodes
		this.T = new HashMap<Integer,HashMap<Integer,Double>>();
		this.sum_i = new HashMap<Integer,Integer> ();
		System.out.println(nodeFile + "\nloading nodes...");
		TextReader in = new TextReader(nodeFile);
		
		String line;
		int count = 0;		
		// load edges
		System.out.println("\nloading edges...");
		in = new TextReader(edgeFile);
		count = 0;
		while ( (line = in.readln()) != null )
		{
			//System.out.println("Edge: "+ line);
			count ++;
			if (count % 1000000 == 0)
				System.out.print(count / 1000000 + "M ");
			
			String[] parts = line.split("\t");
			int startId = Integer.parseInt(parts[1]); // Transition matrix is based on the reverse graph
			int endId = Integer.parseInt(parts[0]);
			if (T.containsKey(startId)){
				HashMap<Integer,Double> Tj = T.get(startId);
				Tj.put(endId, 1.0);
				sum_i.put(startId, sum_i.get(startId)+1);
			}
			else{
				HashMap<Integer,Double> Tj = new HashMap<Integer,Double>();
				Tj.put(endId, 1.0);
				sum_i.put(startId, 1);
				T.put(startId, Tj);
			}
		}
		//test edges reading
		
		in.close();
		
		for (Map.Entry<Integer, HashMap<Integer,Double>> entry_i : T.entrySet()) {
		    Integer startId = entry_i.getKey();
		    Integer sum = sum_i.get(startId);
		    HashMap<Integer,Double> Tij = entry_i.getValue();
		    for (Map.Entry<Integer,Double> entry_ij : Tij.entrySet()) {
		    	entry_ij.setValue(1.0/sum);
		    }
		}
//		printGraph(T);
//		System.out.println();
		
	}
	
}
