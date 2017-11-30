package simrank;

import java.io.IOException;
import java.util.Random;

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
public class SingleRandomWalk_M_R extends SingleRandomWalk_M {
	private int[] sampleCount ;
	protected int STEP = 5;	
//	private int batchSize = 2500;
	private int batchSize = 40000;
	private int times = 10;
	public SingleRandomWalk_M_R(Graph g, int M, int sample, int Step) {
		super(g, M, sample,Step);
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
			sampleCount[i] = 0;
		}
		for (int i = 0; i < COUNT; i++){
//			if(i%10000==0) System.out.println("i: " + i);
			if ( i % batchSize == 0){
//				System.out.println("stage "+(i / batchSize)+"\t"+checkFinished());
//				StopWatch.say("batchSize: " + batchSize + "  ");
//				System.out.println("msg_num: " + msg_num);
				msg_num = 0;
			}
			walk(i, times * STEP, sampleCount[i]);
			sampleCount[i] = SAMPLE;
		}
//		System.out.println("msg_num: " + msg_num);
//		StopWatch.say("total count: " + COUNT + "\n");
	}
	
	@Override
	public void computePathSim(int[] path, int pathLen){
//		System.out.println(pathLen);
		if (pathLen == 0) { return;}
		for (int off = 0; off <= (times - 2)*STEP; off++){
			int source = path[off];
//			if(source==1332){
//				System.out.println(source + " " + pathLen);
//				System.out.println(path[1] + " " + path[2]);
//			}
			if (sampleCount[source] >= SAMPLE) continue;
			sampleCount[source]++;
			for (int i = 1 ; i <=  STEP && off + 2 * i <= pathLen; i++){
				int interNode = path[off + i];
				int target = path[off + 2*i];
				if (isFirstMeet(path, off, off+2*i)){
					double incre = cache[i]* g.degree(interNode)/ g.degree(target) / SAMPLE;
					sim[source].put(target, (float)incre);
					msg_num += 1;
				}
			}
		}
		
	}
	
	
	
	
	public static void main(String[] args) throws IOException  {
		
    
		int fileNum = MyConfiguration.fileNum;
		fileNum = 1;
		for(int i=0;i<fileNum;i++){
			// input path
			String graphInPath = MyConfiguration.in_u_u_graphPath[i];
			String goldPath = MyConfiguration.out_u_u_graphPath_simrank[i] + "_simrank_navie_top" + MyConfiguration.TOPK +".txt";
			
			// output
			String basePath = MyConfiguration.out_u_u_graphPath_single[i];
			String logPath = basePath + "_Single_M_R_Test.log";
			
			int[] samples = {50};
//			int[] samples = {50,100, 250, 1000, 2500,5000,10000,20000,40000};
			int[] steps = {5};
			
			Log log = new Log(logPath);
			log.info("################## Test_u_u_Top" + MyConfiguration.TOPK + " ##################");
			Graph g = new Graph(graphInPath, MyConfiguration.u_u_count[i]);
			System.out.println("read done!");
			StopWatch.say("read done!");
			
			for(int step: steps){
				for (int sample : samples){	
					float acc = 0.0f;
					for(int a=0;a<10;a++){
//					log.info("computation begin!");
					SingleRandomWalk_M_R srw = new SingleRandomWalk_M_R(g, 5, sample,step);
					srw.compute();
					
//					System.out.println("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" +MyConfiguration.TOPK);
//					log.info("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" +MyConfiguration.TOPK);
//					log.info("computation done!");
					
					String outPath = basePath + "_Single_M_R_top" + 20 + "_step" + step + "_sample" + sample + ".txt"; 
//					log.info("u_u_graph singleRandomWalk output done!");
					String prePath = basePath + "_Single_M_R_top" + 20 + "_step" + step + "_sample" + sample + "precision.txt";
					// sim
					Print.printByOrder(srw.getResult(), outPath, MyConfiguration.TOPK);
//					log.info("printByOrder done!");
					// precision
					String pre = Eval.precision(goldPath+".sim.txt", outPath+".sim.txt", prePath, 20);
					Float p = new Float(pre);
					acc += p.floatValue();
//					log.info("Basic SingleRamdomWalk_M_R Top" + MyConfiguration.TOPK + " step" + step + " sample" + sample + " precision: " + pre);
					}
					System.out.println(sample + "acc: " + acc/10.0);
				}
			}
			log.close();
		}

	}

}

/*
Random rand = new Random();
		int []samples = {1,10,100,1000,10000};
		for(int ii=0;ii<5;ii++){
			int sample = samples[ii];
		    float cnt1 = 0.0f, cnt2 = 0.0f;
		    for(int k=0;k<1000;k++){
		    	
		        float sum1=0.0f, sum2 = 0.0f;
		        for(int i=0;i<sample;i++){
		            sum1 += rand.nextInt(1000);
		        }
		        cnt1 += (Math.abs((sum1 / sample) - 500) + Math.abs((sum1 / sample) - 500));
		        
        		for(int i=0;i<sample;i++){
		            sum2 += rand.nextInt(1000);
		        }
		        cnt2 += (Math.abs((sum1 / sample) - 500) + Math.abs((sum2 / sample) - 500));
		    }
		    System.out.print(cnt1/1000);
		    System.out.print(" ");
		    System.out.print(cnt2/1000);
		    System.out.print(" ");
		    System.out.println(cnt1/1000 - cnt2/1000);
		}
		return ;

*/ 
