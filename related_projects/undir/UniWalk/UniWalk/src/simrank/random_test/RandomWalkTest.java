package simrank.random_test;

import java.io.IOException;
import java.util.HashSet;
import java.util.Set;

import lxctools.Log;
import lxctools.LxcArrays;

import conf.MyConfiguration;
import structures.DGraph;
import structures.Graph;

/**
 * this for test some random assumptions
 * @author luoxiongcai
 *
 */
public class RandomWalkTest {
	public static final String randTestPath = MyConfiguration.basePath +"/randTestResult.txt";
	private   Graph g;
	private  DGraph dg;
	
	public RandomWalkTest(Graph g){
		this.g = g;
	} 
	
	public RandomWalkTest(DGraph dg){
		this.dg = dg;
	}
	
	/**
	 * get a random path.
	 * assert that path[0] != path[pathLen - 1];
	 * @param src
	 * @param pathLen
	 * @return
	 */
	public int[] randomPath(int src, int pathLen){
		assert(pathLen % 2 == 0);
		int[] path = new int[pathLen + 1];
		path[0] = src;
		for (int i = 1; i <= pathLen; i++){
			path[i] = g.randNeighbor(path[i-1]);
		}
		assert(path[0] != path[pathLen - 1]);
		return path;
	}
	
	/**
	 * test the probability of a given path.
	 * the sample result meets the real value computed by hand.
	 */
	public void testPathPro(int[] path) {
		int len = path.length;
		// degree
		printDegree(path);
		int sample = 100000;
		// forward
		double realPro =  getPathPro(path);
		double samPro = samplePathPro(path, 100, sample);
		System.out.println("forward real pro: " + realPro);
		System.out.println("forward sample pro:" + samPro);
		System.out.println("forward single random suffer pro: " + samPro * g.degree(path[ len / 2]) / g.degree(path[len - 1]));
		
		System.out.println();
		
		//backward;
		LxcArrays.reverse(path);
		double b_realPro = getPathPro(path);
		double b_samPro = samplePathPro(path, 100,sample);
		System.out.println("backward real pro: " + b_realPro);
		System.out.println("backward sample pro:" + b_samPro);
		System.out.println("backward single random suffer pro: " + b_samPro * g.degree(path[ len / 2]) / g.degree(path[len - 1]));
		
		double df = 1.0;
		for (int i = 0; i < path.length / 2; i++){
			df *= 1.0 / (g.degree(path[i]) * g.degree(path[len - 1 - i]));
		}
		System.out.println("\r\nreal double random suffer pro: " + df);
		
		double double_samPro = samplePathProDoubleWalk(path, 100, sample *10);
		System.out.println("sample double random suffer pro: " + double_samPro);
		
	}
	
	private double getPathPro(int[] path){
		double realPro = 1.0;
		for (int i = 0; i < path.length - 1; i++){
			realPro *= 1.0 / g.degree(path[i]);
		}
		return realPro;
	}
	
	private void printDegree(){
		for (int v = 0; v < g.getVCount() ; v++)
			System.out.println(v+"\t: "+ g.degree(v));
	}
	
	private void printDegree(int [] path){
		for (int v :path){
			System.out.println(v+"\t: "+ g.degree(v));
		}
	}
	
	/**
	 * sample the probability of the given path from node path[0] to  node path[ path.length - 1];
	 * @param path
	 * @param times
	 * @param SAMPLE
	 * @return
	 */
	public double samplePathPro(int [] path, int times, int SAMPLE){
		double sum = 0;
		for (int t = 0; t < times; t++){
			int hit = 0;
			for (int i = 0; i < SAMPLE; i++){
				int cur = path[0];
				for (int j = 0; j < path.length; j++ ){
					if ( j == path.length - 1 && cur == path[j])
						hit++;
					else if (cur != path[j]) break;
					else {
						cur = g.randNeighbor(cur);
						if (cur == -1) break;
					}
				}
			}
			sum += hit *1.0/SAMPLE;
		}
		return sum / times;
	}
	
	/**
	 * sample the probability of the given path. Two random walkers  start to 
	 * walk simultaneously from path[0] and path[ path.length - 1], and meet in path[path.length / 2];
	 * @param path
	 * @param times
	 * @param SAMPLE
	 * @return
	 */
	public double samplePathProDoubleWalk(int [] path, int times, int SAMPLE){
		double sum = 0;
		int len = path.length;
		int mid = len / 2;
		for (int t = 0; t < times; t++){
			int hit = 0;
			for (int i = 0; i < SAMPLE; i++){
				int head = path[0];
				int tail = path[len - 1];
				for (int j = 0; j <= mid; j++ ){
					if ( j == mid && head == path[mid] && tail == path[mid] ){
//						System.out.println("hit!");
						hit++;
					}
						
					else if (head != path[j] || tail != path[ len - 1 - j]) break;
					else {
						head = g.randNeighbor(head);
						tail = g.randNeighbor(tail);
						if (head == -1 || tail == -1) break;
					}
				}
			}
			sum += hit *1.0/SAMPLE;
		}
		return sum / times;
	}
	
	/**
	 * test the single direction random walk for simrank.
	 * @param src
	 * @param des
	 * @param C : the decay factor;
	 * @param L : the maximum step;
	 */
	public void testPairSimRank(int src, int des, double C, int L){
		if (src == des) {
			System.out.println("same nodes!");
			return;
		}
		
		double times = 30;
		double sim = 0;
		for (int j = 0; j < times ; j++){
			int SAMPLE = 40000;
			int[] path = new int[2*L + 1];
			path[0] = src;
			double sum = 0;
			for (int t = 0 ; t < SAMPLE; t++){
				int cur = src;
				for (int step = 1; step <= 2*L; step++){
					cur = g.randNeighbor(cur);
					if (cur == -1) System.out.println("cur:" + cur);
					path[step] = cur;
					if ((cur == des) && (step % 2 == 0) ){
						if (isFirstMeet(path, step))
							sum += Math.pow(C, step / 2) * g.degree(path[step / 2]) / g.degree(path[step]);
					}
				}
			}
			sim +=  sum / SAMPLE;
		}
		System.out.println(src + "\t" + des + "\tsim : " + sim / times);
	}
	
	/**
	 * 
	 * @param path
	 * @param targetIndex
	 * @return
	 */
	private boolean isFirstMeet(int[] path, int targetIndex){
		int internal = targetIndex / 2;
		for (int i = 0; i < internal; i++){
			if (path[i] == path[targetIndex - i]) return false;
		}
		return true;
	}
	
	/**
	 * 
	 * @param src
	 * @param des
	 * @param C
	 * @param L
	 */
	public void testPairSimRankDoubleWalk(int src, int des, double C, int L){
		if (src == des) {
			System.out.println("same nodes!");
			return;
		}
		
		double times = 20;
		double sim = 0;
		for (int j = 0; j < times ; j++){
			int SAMPLE = 30000;
			int[] path = new int[2*L + 1];
			path[0] = src;
			double sum = 0;
			for (int t = 0 ; t < SAMPLE; t++){
				int cur = src;
				for (int step = 1; step <= 2*L; step++){
					cur = g.randNeighbor(cur);
					path[step] = cur;
					if ((cur == des) && (step % 2 == 0) ){
						sum += Math.pow(C, step / 2) * g.degree(path[step / 2]) / g.degree(path[step]);
						break;
					}
				}
			}
			sim +=  sum / SAMPLE;
		}
		System.out.println(src + "\t" + des + "\tsim : " + sim / times);
	}
	
	
	/**
	 * 
	 * @param src
	 * @param des
	 * @param C
	 * @param L
	 */
	public void testPairSimRankDoubleWalkApprox(int src, int des, int SAMPLE, int STEP){
		double result = 0;
		int times = 30;
		for (int i = 0 ; i < times ; i++){
			int[][] pathv = sample(src,SAMPLE ,STEP);
			int[][] pathw = sample(des, SAMPLE,STEP);
			result += getSim(pathv, pathw, SAMPLE, STEP);
		}
		System.out.println(src + "\t"+ des +" sim = "+ (result / times ));
		
	}
	
	private  double getSim(int[][] pathv, int[][] pathw, int SAMPLE, int STEP){
		double result = 0;
		for (int i = 0; i < SAMPLE; i++){
			for (int j = 0; j < SAMPLE; j++){
				for (int step = 0; step < STEP && pathv[i][step] != -1 &&
						pathw[j][step] != -1; step++){
					if (pathv[i][step] == pathw[j][step]){
						result += Math.pow(MyConfiguration.C, step + 1);
						break;
					}
				}
			}
		}
		return result / (SAMPLE * SAMPLE);
	}
	
	private  int[][] sample(int src, int SAMPLE, int STEP){
		int [][]paths = new int[SAMPLE][STEP];
		for (int i = 0; i < SAMPLE; i++){
			int cur = src;
			for (int step = 0; step < STEP; step++){
				cur = g.randNeighbor(cur);
				paths[i][step] = cur;
				if (cur == -1) break;
			}
		}
		return paths;
	}

	public static void main(String[] args) throws IOException {
//		Log log = new Log(randTestPath);
		Graph g = new Graph(MyConfiguration.biGraphPath, MyConfiguration.totalCount);
//		Graph g = new Graph(Configuration.dataBasePath+"/TEST.txt",7);

		RandomWalkTest rwt = new RandomWalkTest(g);

//		int [] path = rwt.randomPath(1, 4);
//		rwt.testPathPro(path);
		
		System.out.println();
		int src = 12;
		System.out.println("src = " + src + "\t deg = " + g.degree(src) );
		int[] target = {241};
		for(int t = 0; t <target.length; t++){
			System.out.println("des = " + target[t] + "\t deg = " + g.degree(target[t]) );
			for (int i = 0; i < 5; i++){
				rwt.testPairSimRank(src,target[t], MyConfiguration.C, 10);
			}
		}

//		System.out.println("########double random walk");
//		for (int t = 0; t < target.length; t++){
//			System.out.println("des = " + target[t] + "\t deg = " + g.degree(target[t]) );			
//			for (int j = 0; j < 5; j++){
//				rwt.testPairSimRankDoubleWalkApprox(src, target[t], 1000, 10);
//			}
//		}
		
	}

}
