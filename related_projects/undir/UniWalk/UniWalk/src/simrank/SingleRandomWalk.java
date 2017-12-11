package simrank;

import java.io.IOException;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;

import lxctools.Log;
import lxctools.StopWatch;
import structures.Graph;
import utils.Eval;
import utils.Print;
import conf.MyConfiguration;
/**
 * @author luoxiongcai
 *
 */
public class SingleRandomWalk {
	protected final int topk = MyConfiguration.TOPK;
	protected int STEP = 5;	
	protected int COUNT;
	protected Graph g;
	protected double[][]sim;
	public static  int SAMPLE = 10000;		// sample number
	public static double[] cache;  
	
	public SingleRandomWalk(Graph g, int sample, int step) {
		this.STEP = step;
		this.SAMPLE = sample;
		this.g = g;
		this.COUNT = g.getVCount();
		sim = new double[COUNT][COUNT];
		cache = new double[STEP+1];
		for (int i = 1; i <= STEP; i++)
			cache[i] = Math.pow(MyConfiguration.C, i);
	}

	public void compute(){
		StopWatch.start();
		
		for (int i = 0; i < COUNT; i++){
			if(i%100000==0) System.out.println("i: " + i);
			walk(i, 2*STEP,0);
			sim[i][i] = 0;
		}
		
		StopWatch.say("finish");
	}
	
	/**
	 * 
	 * @param v : the start node of the random walk
	 * @param len : the length of the random walk
	 * @param initSample : already sampled count.for reuse other paths.
	 */
	protected void walk(int v, int len, int initSample){
		int maxStep = Math.max( 2 * STEP , len);
		for (int i = initSample; i < SAMPLE; i++) {
//			if(i%1000000==0){
//				System.out.println(i);
//			}
			int pathLen = 0;
			int[] path = new int[maxStep + 1];
			Arrays.fill(path, -1);
			path[0] = v;
			int cur = v;
			while (pathLen < maxStep){
				cur = g.randNeighbor(cur);
				if (cur == -1) break;
				path[++pathLen] = cur;
			}
			// compute the sim to v;
			computePathSim(path, pathLen);			
		}
	}
	

	
	/**
	 * compute the similarity of one path that starts from path[0]
	 * @param path : a path from path[0]
	 * @param pathLen : the length of the path. 
	 */
	protected void computePathSim(int[] path, int pathLen){
		if (pathLen == 0) return;
		int source = path[0];
		for (int i = 1 ; i <= STEP && 2 * i <= pathLen; i++){
			int interNode = path[i];
			int target = path[2*i];
			if (target == source) continue;
			if (isFirstMeet(path,0, 2*i)){
				sim[source][target] +=  cache[i]* g.degree(interNode)/ g.degree(target) / SAMPLE;
			}
		}
	}
	
	/**
	 * srcIndex < dstIndex
	 * @param path
	 * @param targetIndex
	 * @return
	 */
	public boolean isFirstMeet(int[] path, int srcIndex, int dstIndex){
		int internal = (dstIndex - srcIndex) / 2 + srcIndex;
		for (int i = srcIndex; i < internal; i++){
			if (path[i] == path[dstIndex - i + srcIndex]) return false;
		}
		return true;
	}
	
	/**
	 * srcIndex > dstIndex.
	 * @param path
	 * @param srcIndex
	 * @param dstIndex
	 * @return
	 */
	public boolean isFirstMeetReverse(int[] path, int srcIndex, int dstIndex){
		int internal = (srcIndex - dstIndex) / 2 + dstIndex; 
		for (int i = dstIndex; i < internal; i++){
			if (path[i] == path[srcIndex - i + dstIndex]) return false;
		}
		return true;
	}
	
	public double[][] getResult(){
		return sim;
	}


	public static void main(String[] args) throws IOException {
		int fileNum = MyConfiguration.fileNum;
		fileNum = 5;
		Queue<String> pre_ans = new LinkedList<String>();
		for(int i=4;i<fileNum;i++){
			String graphInPath = MyConfiguration.in_u_u_graphPath[i];
			String goldPath = MyConfiguration.out_u_u_graphPath_simrank[i] + "_simrank_navie_top" + MyConfiguration.TOPK +".txt";
			
			String basePath = MyConfiguration.out_u_u_graphPath_single[i];
			String logPath = basePath + "_Single_Test.log";
			
			int[] samples = {500, 2500,5000,10000,20000,40000};
//			int[] samples = {2500};
			int[] steps = {5};
			
			Log log = new Log(logPath);
			log.info("################## Test_u_u_Top" + MyConfiguration.TOPK + " ##################");
			Graph g = new Graph(graphInPath, MyConfiguration.u_u_count[i]);

			for(int step: steps){
				for (int sample : samples){	
					log.info("computation begin!");
					SingleRandomWalk srw = new SingleRandomWalk(g,sample,step);
					srw.compute();
					
					System.out.println("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" +MyConfiguration.TOPK);
					log.info("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" +MyConfiguration.TOPK);
					log.info("computation done!");
					
					String outPath = basePath + "_Single_top" + 20 + "_step" + step + "_sample" + sample + ".txt"; 
					log.info("u_u_graph singleRandomWalk output done!");
					String prePath = basePath + "_Single_top" + 20 + "_step" + step + "_sample" + sample + "precision.txt";
					// sim
//					Print.printByOrder(srw.getResult(), outPath, MyConfiguration.TOPK, 20);
//					log.info("printByOrder done!");
//					// precision
//					String pre = Eval.precision(goldPath+".sim.txt", outPath+".sim.txt", prePath, 20);
//					pre_ans.add(pre);
//					log.info("Basic SingleRamdomWalk Top" + MyConfiguration.TOPK + " step" + step + " sample" + sample + " precision: " + pre);
				}
			}
			log.close();
		}
		System.out.println("precision:");
		while(!pre_ans.isEmpty()){
			String pre = pre_ans.remove();
			System.out.println(pre);
		}
	}

}
