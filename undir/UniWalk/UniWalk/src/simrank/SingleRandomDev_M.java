package simrank;
// s
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import lxctools.FixedCacheMap;
import lxctools.Log;
import lxctools.StopWatch;
import structures.Graph;
import utils.Eval;
import utils.Path;
import utils.Print;
import conf.MyConfiguration;
/**
 * use FixedHashTable<Integer>[] sim
 * do not store all similarities for each vertex.
 * @author luoxiongcai
 *
 */
public class SingleRandomDev_M {
	protected static final int topk = MyConfiguration.TOPK;
	protected int capacity;
	protected int STEP = 2;	
	protected int COUNT;
	protected Graph g;
	protected FixedCacheMap[] sim;
	protected int SAMPLE = 10000;
	public static double[] cache;  
	
	@SuppressWarnings("unchecked")
	public SingleRandomDev_M(Graph g, int M, int sample, int step) {
		this.STEP = step;
		this.g = g;
		this.COUNT = g.getVCount();
		sim = new FixedCacheMap[1000];
		capacity = topk * M;
		SAMPLE = sample;
		for (int i = 0; i < 1000; i++){
			sim[i] = new FixedCacheMap(capacity);
		}
		
		cache = new double[STEP+1];
		for (int i = 1; i <= STEP; i++)
			cache[i] = Math.pow(MyConfiguration.C, i);
	}

	public void compute(){
//		for (int i = 0; i < 1000; i++){
//			if(i%10==0) System.out.println("i: " + i);
//			walk(i, 2*STEP,0);
//		}
		StopWatch.start();
		for (int i = 0; i < Math.min(COUNT,1000); i++){
			if(i%100==0) System.out.println("i: " + i);
			walk(i, 2*STEP,0);
//			break;
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
//		StopWatch.start();
		int maxStep =Math.max( 2 * STEP , len);

		LinkedList<Path[]> queue = new LinkedList<Path[]>();
		Queue<Path[]> queue_i = new LinkedList<Path[]>();
		Path[] path = new Path[maxStep + 1];		
		path[0] = new Path();
		path[0].cur = v; path[0].sample = SAMPLE; path[0].prob = SAMPLE;
		queue.add(path);
		
		int pathLen = 0; 
		int TopSim=1; 
		while (pathLen < maxStep){
			if(pathLen/2 == TopSim){
				computePathSim(queue, pathLen,TopSim);
				TopSim++;
			}
			int sum = queue.size();
			Path cur = new Path(); 
//			System.out.println("pathLen = " + pathLen + "   sum = " + sum);
			for(int i=0;i<sum;i++){
//				cur = queue.get(0)[pathLen];		
				cur = queue.element()[pathLen];
				
				path = queue.getFirst().clone();
				queue_i.add(path);
//				cur = path[pathLen];
//				cur = new Path();
//				cur.cur = path[pathLen].cur;  cur.sample = path[pathLen].sample;	// 手动复制
//				cur.prob = path[pathLen].prob;
				
				int degree = g.degree(cur.cur);	
				if(degree==0){
//					System.out.println("!!!something wrong..." + cur.cur + " " + degree);
				}
				if(degree !=0 && cur.sample >= degree){
					
					// newSample
					List<Integer> edges;
					edges = g.neighbors(cur.cur);	// cur
					double newSample = ((double)cur.prob/(double)degree);
					int som = (int) (cur.sample % degree);
					for(int j=0;j<som;j++){
						Path nextCur = new Path();
						nextCur.sample = (int) (cur.sample / degree)+1;
						nextCur.prob = newSample;
						nextCur.cur = edges.get(j);	// id
						
						path = queue_i.element().clone();
						path[pathLen+1] = nextCur;
						queue.add(path);
					}
					for(int j=som;j<degree;j++){
						// sample
						Path nextCur = new Path();
						nextCur.sample = (int) (cur.sample / degree);
						nextCur.prob = newSample;
						nextCur.cur = edges.get(j);	// id
						
						path = queue_i.element().clone();
						path[pathLen+1] = nextCur;
						queue.add(path);
						
//						System.out.print("path:");
//						for(int k=0;k<=pathLen+1;k++){
//							 System.out.print( path[k].cur + " ");
//						}System.out.println();
//						for(int k=0;k<=pathLen+1;k++){
//							 System.out.print( path[k].sample + " ");
//						}System.out.println();
					}
//					cur.sample -= newSample*degree;
				}else{
					double newSample = ((double)cur.prob/(double)degree);
//					System.out.println("else...");
//					System.out.println("sample ：" + cur.sample);
					int number;
					if((int)cur.sample == cur.sample){
						number = (int)cur.sample;
					}else{
						number = (int)cur.sample + 1;
					}
//					number = Math.min(number, degree);
					for(int j=0;j<number;j++){	// number
						Path nextCur = new Path();
//						nextCur.sample = (double)cur.sample/(double)number;		
						nextCur.prob = newSample;
						nextCur.sample = 1.0;		
						int num = g.randNeighbor(cur.cur);	
						if (num == -1) break;
						nextCur.cur = num;
						
//						path = queue.getFirst().clone();
						path = queue_i.element().clone();
						path[pathLen+1] = nextCur;
						queue.add(path);
					}
				}
				queue.remove();
				queue_i.remove();
			}
			pathLen++;
		}
		// compute the sim to v;
		computePathSim(queue, pathLen, TopSim);
	}
	

	
	/**
	 * compute the similarity of one path that starts from path[0]
	 * @param path : a path from path[0]
	 * @param pathLen : the length of the path. 
	 */
	protected void computePathSim(Queue<Path[]> queue, int pathLen, int start){
		Queue<Path[]> queue2 = new LinkedList<Path[]>();
//		BlockingQueue<Path[]> queue;
		Path[] path = new Path[pathLen+1];
		while(!queue.isEmpty()){
			Path[] path2 = new Path[pathLen+1]; path2 = queue.element().clone();
			queue2.add(path2);
			
			path = queue.remove();
			
			if (pathLen == 0) return ;
			int source = path[0].cur;
			for (int i = start ; i <= STEP && 2 * i <= pathLen; i++){
				int interNode = path[i].cur;
				int target = path[2*i].cur;
				if (target == source) continue;
				if(target==-1) continue;
				if (isFirstMeet(path,0, 2*i)){
//					System.out.println(2*i + " " + path.length + " " + path[2*i].sample + " " + cache.length);
//					System.out.println(source + " " + target + " " + path[2*i].sample + " " + cache[i] + " " + g.degree(interNode)/ g.degree(target) );
//					sim[source][target] +=  path[2*i].prob * cache[i]* (double)g.degree(interNode)/ (double)g.degree(target) / SAMPLE ;
					double incre =  path[2*i].prob * cache[i]* g.degree(interNode)/ g.degree(target) / SAMPLE;
					sim[source].put(target, (float)incre);
				}
			}
		}
		Path[] path2 = new Path[pathLen+1];
		while(!queue2.isEmpty()){
			path2 = queue2.element().clone();
			queue.add(path2);
			queue2.remove();
		}
	}
	
	public FixedCacheMap[] getResult(){
		return sim;
	}
	
	/**
	 * srcIndex < dstIndex
	 * @param path
	 * @param targetIndex
	 * @return
	 */
	public boolean isFirstMeet(Path[] path, int srcIndex, int dstIndex){
		
		int internal = (dstIndex - srcIndex) / 2 + srcIndex;
		for (int i = srcIndex; i < internal; i++){
			if (path[i].cur == path[dstIndex - i + srcIndex].cur) return false;
		}
		return true;
	}

	public static void main(String[] args) throws IOException {
		int fileNum = MyConfiguration.fileNum;
		fileNum = 8;
		
		for(int i=7;i<fileNum;i++){
			// input
			String graphInPath = MyConfiguration.in_u_u_graphPath[i];
//			String goldPath = MyConfiguration.out_u_u_graphPath_simrank[i] + "_simrank_navie_top" + MyConfiguration.TOPK +".txt";
		
			// output
			String basePath = MyConfiguration.out_u_u_graphPath_singledev[i];
			String logPath = basePath + "_singledev_M_Test.log";
			
			int[] samples = {10000};
			int[] steps = {5};		
			
			Log log = new Log(logPath);
			log.info("################## Test_u_u_Top" + MyConfiguration.TOPK);
			System.out.println("read begin!");
			Graph g = new Graph(graphInPath, MyConfiguration.u_u_count[i]);
			System.out.println("read done!");
			StopWatch.say("read done!");
			for(int step: steps){
				for (int sample : samples){	
					StopWatch.start();
					log.info("computation begin!");
					StopWatch.say("computation begin!");
					SingleRandomDev_M srw = new SingleRandomDev_M(g, 5 * 1000, sample, step);
					srw.compute();
					log.info("computation done!");
					StopWatch.say("computation done!");
					System.out.println("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" + 20);
					log.info("Test Step:" + step + " Sample:"+sample + "TopK:" + 20);
					
					String outPath = basePath + "_singledev_M_top" + 20 + "_step" + step + "_sample" + sample + ".txt"; 
					log.info("u_u_graph singleRandomWalk output done!");

					String prePath = basePath + "_singledev_M_top" + 20 + "_step" + step + "_sample" + sample + "precision.txt";
					// sim
					Print.printByOrder(srw.getResult(), outPath, MyConfiguration.TOPK);
					// precision
//					log.info("printByOrder done!");
//					String pre = Eval.precision(goldPath+".sim.txt", outPath+".sim.txt", prePath, 20);
//					log.info("Basic SingleRamdomDev_M Top" + MyConfiguration.TOPK + " step" + step + " sample" + sample + " precision: " + pre);
					
				}
			}
			g = null;
			log.close();
		}
	}

}
