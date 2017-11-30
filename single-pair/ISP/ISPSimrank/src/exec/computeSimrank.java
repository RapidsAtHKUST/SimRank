package exec;


import java.util.*;

import util.*;
import core.*;




public class computeSimrank {

    public static void main(String[] args) throws Exception {
    	/*String nodeFile = args[0];
    	String edgeFile = args[1];
    	String queryFile = args[2];
    	String resultFile = args[3];
    	int topk = Integer.valueOf(args[4]);*/
    	Config.depth=Integer.valueOf(args[0]);
    	Config.stopRea = Double.valueOf(args[1]);
    	String nodeFile = Config.nodeFile;
    	String edgeFile = Config.edgeFile;
    	String queryFile = Config.queryFile;
    	String resultFile = Config.outputDir + "/" + 
        		"ISP-"  + "_D" +Config.depth+ " _h"+ Config.stopRea;
    	
//    	int matrixSize = Integer.valueOf(args[4]);
//    	int topk = Integer.valueOf(args[5]);
    	
    	double c = Config.alpha;//damping factor
        Graph graph = new Graph(nodeFile, edgeFile);	
        
       // graph.addSelfLoops(1.0);
	//	initWeights(graph);
		
	//	graph.printStats();
		
	
		SimRank sr = new SimRank(graph,c);
		

		System.out.println("\nLoading queries...");
        List<Integer[]> qids = new ArrayList<Integer[]>();
        TextReader in = new TextReader(queryFile);
        String line;
        while ( (line = in.readln()) != null) {
        	String[] parts = line.split("\t");
			int a = Integer.parseInt(parts[0]);
			int b = Integer.parseInt(parts[1]);
			qids.add(new Integer[]{a,b});
        }
        in.close();
 //       System.out.println("Size of queries: " + qids.size());
       
        System.out.println("Starting query processing...");
        
        TextWriter out = new TextWriter(resultFile);     
        int count =0;
        for (Integer[] qid : qids) {
        	//System.out.println("Processing Sim("+qid[0]+","+qid[1]+")");
        	//Map<Integer, Double> result = new HashMap<Integer, Double>();
        	count++;
            if (count % 10 == 0)
            	System.out.print("+");
        	long start = System.currentTimeMillis();
        	Double simScore = sr.getSim(qid[0],qid[1]);
        	long elapsed =  System.currentTimeMillis() - start;
        	out.writeln(elapsed+"ms "+simScore);
           
            
          /*  //test
            double sum =0.0;
            for (Map.Entry<Integer,Double> e: result.entrySet()){
            	sum += e.getValue();
            	System.out.print(" K: "+ e.getKey() + " V: "+e.getValue());
            }
            	
            System.out.println("\nSUm " + sum);
            //end test		
*/
//        	List<KeyValuePair> rankedResult = getTopResult(topk,result);
           
            
          /* //test
            for (int i = 0; i < rankedResult.size(); i++)
            	System.out.print("\nranked K: "+ rankedResult.get(i).key + " V: "+rankedResult.get(i).value);
            //end test	
             * 
             */
//            for (KeyValuePair e : rankedResult)
                //out.write("Sim("+qid[0]+","+qid[1]+")="+sr.getSim(qid[0],qid[1]));
          
           // out.writeln();
        }
        
        out.close();
        
      //  System.out.println();
    }
    
//    private static void initWeights(Graph graph) {
//		// TODO Auto-generated method stub
//    	
//    		for (int i = 0; i < graph.numNodes(); i++){
//    			Node n =graph.getNodeByIndex(i);
//    			for (Edge e: n.out){
//    				e.setWeight(1.0/n.outSize);
//    				//test
//    				System.out.print("weight of edge: " + e.from.id +" "+ e.to.id+ " "+ e.getWeight());
//    			}
//    			
//    		
//    		graph.updateDeg();
//    	}
//    }
//    	

	private static List<KeyValuePair> getTopResult(int k, final Map<Integer,Double> rawResult) {
		List<Integer> list = new ArrayList<Integer>(rawResult.size());
		for (int i : rawResult.keySet())
			list.add(i);
		
		Collections.sort(list, new Comparator<Integer>() {
			@Override
			public int compare(Integer arg0, Integer arg1) {
				return - Double.compare(rawResult.get(arg0), rawResult.get(arg1));
			}} );
		
		if (k > list.size()) 
			k = list.size();
		
		List<KeyValuePair> result = new ArrayList<KeyValuePair>(k);

		for (int i = 0; i < k; i++) {
			result.add(new KeyValuePair(list.get(i), rawResult.get(list.get(i))));
		}
		
		return result;
	}

}
