package benchmark;

import java.io.IOException;

import lxctools.Log;
import simrank.SingleRandomWalk;
import structures.Graph;
import utils.Eval;
import utils.Print;
import conf.MyConfiguration;

/**
 * test samples nums for singleRandomWalk precision.
 * input data: 10k_5.
 * gold standard: 20 iterations.
 * @author luoxiongcai
 *
 */
public class Test_u_u_SingleRandomWalk_Sample {
	
	public static void main(String[] args) throws IOException {
		int fileNum = MyConfiguration.fileNum;
		fileNum = 1;
		for(int i=0;i<fileNum;i++){
			// 基本的输入路径，都一样
			String graphInPath = MyConfiguration.in_u_u_graphPath[i];
			String goldPath = MyConfiguration.out_u_u_graphPath_simrank[i] + "_simrank_navie_top" + MyConfiguration.TOPK +".txt";
			
			// 基本的输出路径
			String basePath = MyConfiguration.out_u_u_graphPath_single[i];
			String logPath = basePath + "_Single_Test.log";
			
			int[] samples = {2500};
//			int[] samples = {2500,5000,10000,20000,40000,80000,160000,640000};
			int[] steps = {5};
			
			Log log = new Log(logPath);
			log.info("################## Test_u_u_Top" + MyConfiguration.TOPK + " ##################");
			Graph g = new Graph(graphInPath, MyConfiguration.u_u_count[i]);

			for(int step: steps){
				for (int sample : samples){	// 这里step加上之后，路径还要进一步调整
					log.info("computation begin!");
					SingleRandomWalk srw = new SingleRandomWalk(g,sample,step);
					srw.compute();
					
					System.out.println("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" +MyConfiguration.TOPK);
					log.info("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" +MyConfiguration.TOPK);
					log.info("computation done!");
					
					String outPath = basePath + "_Single_top" + 20 + "_step" + step + "_sample" + sample + ".txt"; 
					log.info("u_u_graph singleRandomWalk output done!");
					String prePath = basePath + "_Single_top" + 20 + "_step" + step + "_sample" + sample + "precision.txt";
					// 计算精度，传入sim[][]数组,计算前k个相似的点输出到文件
					Print.printByOrder(srw.getResult(), outPath, MyConfiguration.TOPK, 20);
					log.info("printByOrder done!");
					// 下面精度计算的也一样，和DoubleWalk用的是同一个
					String pre = Eval.precision(goldPath+".sim.txt", outPath+".sim.txt", prePath, 20);
					log.info("Basic TopSimSingleRamdomWalk Top" + MyConfiguration.TOPK + " step" + step + " sample" + sample + " precision: " + pre);
				}
			}
			log.close();
		}
		
	}

}



