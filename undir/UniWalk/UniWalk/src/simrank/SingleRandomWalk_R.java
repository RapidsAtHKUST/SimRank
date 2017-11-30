package simrank;

import java.io.IOException;
import java.util.LinkedList;
import java.util.Queue;

import lxctools.Log;
import lxctools.StopWatch;
import conf.MyConfiguration;
import structures.Graph;
import utils.Eval;
import utils.Print;

/**
 * @author luoxiongcai
 * 
 */
public class SingleRandomWalk_R extends SingleRandomWalk {
	private int[] sampleCount ;
	protected int STEP = 5;	
//	private int batchSize = 2500;
	private int batchSize = 40000;
	private int times = 3;			// path sharing
	public SingleRandomWalk_R(Graph g, int sample, int Step) {
		super(g,sample,Step);
		sampleCount = new int[g.getVCount()];
	}
	
	private int checkFinished(){
		int c = 0;
		for (int sc : sampleCount){
			if (sc >= SingleRandomWalk.SAMPLE)
				c++;
		}
		return c;
	}
	
	@Override
	public void compute(){
		StopWatch.start();
		
		for (int i = 0; i < COUNT; i++){
			if(i%100000==0) System.out.println("i: " + i);
			if ( i % batchSize == 0){
				System.out.println("stage "+(i / batchSize)+"\t"+checkFinished());
			}
			walk(i, times * STEP, sampleCount[i]);
			sampleCount[i] = SAMPLE;
			sim[i][i] = 0;
			
		}
		
		System.out.println("stage "+(g.getVCount() / batchSize)+"\t"+checkFinished());
		
		StopWatch.say("finish");
		
	}
	
	
	@Override
	public void computePathSim(int[] path, int pathLen){
//		System.out.println("Override...");
		if (pathLen == 0) return;
		for (int off = 0; off < (times - 2)*STEP; off++){
			int source = path[off];
			if (sampleCount[source] >= SAMPLE) continue;
			sampleCount[source]++;
			for (int i = 1 ; i <=  STEP && 2 * i <= pathLen; i++){
				int interNode = path[off + i];
				int target = path[off + 2*i];
				if (target == source) continue;
				if (isFirstMeet(path, off, off+2*i))
					sim[source][target] +=  cache[i]* g.degree(interNode)/ g.degree(target) / SAMPLE;
			}
//			break;
		}
	}
	
	
	
	public static void main(String[] args) throws IOException  {
		System.out.println("songjs");
		int fileNum = MyConfiguration.fileNum;
		fileNum = 5;
		Queue<String> pre_ans = new LinkedList<String>();
		for(int i=4;i<fileNum;i++){
			// input path
			String graphInPath = MyConfiguration.in_u_u_graphPath[i];
			String goldPath = MyConfiguration.out_u_u_graphPath_simrank[i] + "_simrank_navie_top" + MyConfiguration.TOPK +".txt";
			
			// output path
			String basePath = MyConfiguration.out_u_u_graphPath_single[i];
			String logPath = basePath + "_Single_R_Test.log";
			
//			int[] samples = {2500};
			int[] samples = {500,2500,5000,10000,20000,40000};
			int[] steps = {5};
			
			Log log = new Log(logPath);
			log.info("################## Test_u_u_Top" + MyConfiguration.TOPK + " ##################");
			Graph g = new Graph(graphInPath, MyConfiguration.u_u_count[i]);

			for(int step: steps){
				for (int sample : samples){	
					log.info("computation begin!");
					SingleRandomWalk_R srw = new SingleRandomWalk_R(g,sample,step);
					srw.compute();
					
					System.out.println("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" +MyConfiguration.TOPK);
					log.info("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" +MyConfiguration.TOPK);
					log.info("computation done!");
					
					String outPath = basePath + "_Single_R_top" + 20 + "_step" + step + "_sample" + sample + ".txt"; 
					log.info("u_u_graph singleRandomWalk output done!");
					String prePath = basePath + "_Single_R_top" + 20 + "_step" + step + "_sample" + sample + "precision.txt";
					// sim
//					Print.printByOrder(srw.getResult(), outPath, MyConfiguration.TOPK, 20);
//					log.info("printByOrder done!");
					// precision
//					String pre = Eval.precision(goldPath+".sim.txt", outPath+".sim.txt", prePath, 20);
//					pre_ans.add(pre);
//					System.out.println("pre" + pre);
//					log.info("Basic SingleRamdomWalk_R Top" + MyConfiguration.TOPK + " step" + step + " sample" + sample + " precision: " + pre);
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
