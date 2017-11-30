package simrank;

import java.io.IOException;

import lxctools.StopWatch;
import conf.MyConfiguration;
import structures.Graph;
import utils.Print;

/**
 * naive simrank
 * @author luoxiongcai
 *
 */
public class SimRank {
	private int STEP = 20;	
	private int COUNT;
	private Graph g;
	private double[][]sim, tempSim;
	
	public SimRank(Graph g){
		this.g = g;
		this.COUNT = g.getVCount();
		    
		sim = new double[COUNT][COUNT];
		tempSim = new double[COUNT][COUNT];
		for (int i = 0; i < COUNT; i++){
			sim[i][i] = 1.0;
			tempSim[i][i] = 1.0;
		}
	}
	
	/**
	 * the main logic 
	 */
	public void compute(){
		int r = 0;
		while (r++ < STEP){
			for (int i = 0; i < COUNT; i++){
				for (int j = i+1; j < COUNT; j++){
					tempSim[i][j] = sim(i, j);
					tempSim[j][i] = tempSim[i][j];
				}
			}
			// copy ;
			for (int i = 0; i < COUNT; i++){
				for (int j = 0; j < COUNT; j++){
					sim[i][j] = tempSim[i][j];
					sim[j][i] = tempSim[j][i];
				}
			}
		}
		postProcess();
	}
	
	/**
	 * set sim(i,i) = 0
	 */
	private void postProcess(){
		for (int i = 0; i < COUNT; i++)
			sim[i][i] = 0;
	}
	
	public double sim(int v, int w){
		if (v == w) return 1;
		double result = 0;
		if (g.degree(v) == 0 || g.degree(w) == 0) return 0;
		for (int vn : g.neighbors(v)){
			for (int wn : g.neighbors(w)){
				result += sim[vn][wn];
			}
		}
		return MyConfiguration.C * result / (g.degree(v) * g.degree(w));
	}
	
	public double[][] getResult(){
		return sim;
	}
	
	
	
	public static void main(String[] args) throws IOException {
		StopWatch.start();
		StopWatch.say("####### SimRank naive#########");
		
		int fileNum = MyConfiguration.fileNum;
		fileNum = 2;
		for(int i=0;i<fileNum;i++){
			String graphInPath = MyConfiguration.in_u_u_graphPath[i];
			Graph g = new Graph(graphInPath, MyConfiguration.u_u_count[i]);
			StopWatch.say("Graph construction done! v: " + g.getVCount()+" [0...V-1]\t E: "+g.getECount());
			
			String graphOutPath = MyConfiguration.out_u_u_graphPath_simrank[i];
			String outPath = graphOutPath + "_simrank_navie_top" + MyConfiguration.TOPK +".txt";
			SimRank sr = new SimRank(g);	// 全部初始化为1
			sr.compute();					// 迭代计算Naive SimRank值
	
			StopWatch.say("Graph simrank computation done!");
			Print.printByOrder(sr.getResult(), outPath, MyConfiguration.TOPK, 10 );
			StopWatch.say("Graph simrank result print done!");
		}
	}

}
