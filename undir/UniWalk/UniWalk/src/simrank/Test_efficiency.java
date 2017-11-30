package simrank;
// s
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

import lxctools.FixedCacheMap;
import lxctools.StopWatch;
import structures.Graph;
import utils.Path;
import utils.Print;
import conf.MyConfiguration;
/**
 * use FixedHashTable<Integer>[] sim
 * do not store all similarities for each vertex.
 * @author luoxiongcai
 *
 */
public class Test_efficiency {
	protected static final int topk = MyConfiguration.TOPK;
	protected int capacity;
	protected int STEP = 5;	
	protected int COUNT;
	protected Graph g;
	protected FixedCacheMap[] sim;
	protected int SAMPLE = 10000;
	public static double[] cache;  
	
	
	@SuppressWarnings("unchecked")
	public Test_efficiency(Graph g, int M, int sample) {
		this.g = g;
		this.COUNT = g.getVCount();
		sim = new FixedCacheMap[COUNT];
		capacity = topk * M;
		SAMPLE = sample;
		for (int i = 0; i < COUNT; i++){
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
		for (int i = 0; i < COUNT; i++){
			if(i%100==0) System.out.println("i: " + i);
			walk(i, 2*STEP,0);
		}
			
	}
	
	/**
	 * 
	 * @param v : the start node of the random walk
	 * @param len : the length of the random walk
	 * @param initSample : already sampled count.for reuse other paths.
	 */
	protected void walk(int v, int len, int initSample){
		int maxStep =Math.max( 2 * STEP , len);
		// 声明一个Path List
		
		StopWatch.start();
		LinkedList<Path[]> queue = new LinkedList<Path[]>();
		Path[] path = new Path[maxStep + 1];		// 数组
		path[0] = new Path();
		
		
		
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
				// 队列中有sum个是上一轮的，之后要删一个，加入x个
//				path = new Path[queue.get(i).length + 1];
				cur = queue.get(0)[pathLen];		// 删除过后，每次都是从最顶元素0开始
				
//				cur = path[pathLen];
//				cur = new Path();
//				cur.cur = path[pathLen].cur;  cur.sample = path[pathLen].sample;	// 手动复制
//				cur.prob = path[pathLen].prob;
				
				int degree = g.degree(cur.cur);	//获得顶点cur的度数
				if(degree==0){
					// 独立顶点
//					System.out.println("!!!当前点无连边..." + cur.cur + " " + degree);
				}
				if(degree !=0 && cur.sample >= degree){
					
					// 最起码每条都要送去一条边(newSample条)
					List<Integer> edges;
					edges = g.neighbors(cur.cur);	//顶点cur，边个数
					double newSample = ((double)cur.prob/(double)degree);
					
					int som = (int) (cur.sample % degree);
					for(int j=0;j<som;j++){
						Path nextCur = new Path();
						nextCur.sample = (int) (cur.sample / degree)+1;
						nextCur.prob = newSample;
						nextCur.cur = edges.get(j);	//edges内保存顶点的id
						
						path = queue.getFirst().clone();
						path[pathLen+1] = nextCur;
						queue.add(path);
					}
					for(int j=som;j<degree;j++){
						// 对每一条边，进行sample
						Path nextCur = new Path();
						nextCur.sample = (int) (cur.sample / degree);
						nextCur.prob = newSample;
						nextCur.cur = edges.get(j);	//edges内保存顶点的id
						
						path = queue.getFirst().clone();
						path[pathLen+1] = nextCur;
						queue.add(path);
						
//						System.out.print("路径：");
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
//					System.out.println("sample剩余：" + cur.sample);
					int number;
					if((int)cur.sample == cur.sample){
						number = (int)cur.sample;
					}else{
						number = (int)cur.sample + 1;
					}
//					number = Math.min(number, degree);
					for(int j=0;j<number;j++){	// 采样number个，随即找到路径走下去（但是sample会是一个小数）
						Path nextCur = new Path();
//						nextCur.sample = (double)cur.sample/(double)number;		
						nextCur.prob = newSample;
						nextCur.sample = 1.0;		
						int num = g.randNeighbor(cur.cur);	
						if (num == -1) break;
						nextCur.cur = num;
						
						path = queue.getFirst().clone();
						path[pathLen+1] = nextCur;
						queue.add(path);
					}
				}
				// 之后处理多出来的边，如：sample=120，degree=100。 那么还有20条需要按照2/10发出去20条
				queue.remove();
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
	protected void computePathSim(LinkedList<Path[]> queue, int pathLen, int start){
//		Path[] path = new Path[pathLen+1];
		
		
		for(int location=0; location<queue.size(); location++){
			Path[] path = queue.get(location);		// 地址传递？
			
			if (pathLen == 0) return;
			int source = path[0].cur;
			for (int i = start ; i <= STEP && 2 * i <= pathLen; i++){
				int interNode = path[i].cur;
				int target = path[2*i].cur;
				if (target == source) continue;
				if(target==-1) continue;
				if (isFirstMeet(path,0, 2*i)){
//					System.out.println(2*i + " " + path.length + " " + path[2*i].sample + " " + cache.length);
//					System.out.println(source + " " + target + " " + path[2*i].sample + " " + cache[i] + " " + g.degree(interNode)/ g.degree(target) );
					double incre =  path[2*i].prob * cache[i]* g.degree(interNode)/ g.degree(target) / SAMPLE;
					sim[source].put(target, (float)incre);	
				}
			}
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
		int number = 1000000;
		Path[] path_i = new Path[10];		// 数组
		StopWatch.start();
		Queue<Path[]> queue2 = new LinkedList<Path[]>();
		for(int i=0;i<number;i++){
			Path[] path = new Path[10];		// 数组
			for(int j=0;j<10;j++)
				path[j] = new Path();
			queue2.add(path);
		}
		StopWatch.say("time: ");
		for(int i=0;i<number;i++){
			Path[] path = new Path[11];
			path = queue2.element().clone();
			queue2.remove();
		}
		StopWatch.say("time: ");
		
		
		StopWatch.start();
		LinkedList<Path[]> queue = new LinkedList<Path[]>();
		for(int i=0;i<number;i++){
			Path[] path = new Path[10];		// 数组
			for(int j=0;j<10;j++)
				path[j] = new Path();
			queue.add(path);
			
		}
		StopWatch.say("time: ");
		path_i = queue.getFirst().clone();
		for(int i=0;i<number;i++){
			Path[] path = new Path[11];
//			path = queue.getFirst().clone();
			path = path_i.clone();		// 这个费时；但是queue中的clone很快，不知道为什么。。
			queue.remove();
		}
		StopWatch.say("time: ");
		
		
		
		
		
		
		
	}

}
