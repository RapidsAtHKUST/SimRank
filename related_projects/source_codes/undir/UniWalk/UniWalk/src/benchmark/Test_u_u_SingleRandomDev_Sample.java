package benchmark;

import java.io.IOException;

import lxctools.Log;
import simrank.SingleRandomDev;
import structures.Graph;
import utils.Eval;
import utils.Print;
import conf.MyConfiguration;

/**
 * 按照UniWalk进行向TopSim改进的方法，我们要基于这个方法来进行处理
 * test samples nums for topSim precision.
 * input data: 1K.
 * gold standard: 30 iterations.
 * @author luoxiongcai, Alan
 *
 */

public class Test_u_u_SingleRandomDev_Sample {
	
	public static void main(String[] args) throws IOException {
		int fileNum = MyConfiguration.fileNum;
		fileNum = 1;
		for(int i=0;i<fileNum;i++){
			String graphInPath = MyConfiguration.in_u_u_graphPath[i];
			String goldPath = MyConfiguration.out_u_u_graphPath_simrank[i] + "_simrank_navie_top" + MyConfiguration.TOPK +".txt";
			
			// 基本的输出路径
			String basePath = MyConfiguration.out_u_u_graphPath_singledev[i];
			String logPath = basePath + "_singledev_M_Test.log";
			
			int[] samples = {5000};
//			int[] samples = {2500,5000,10000,20000,40000,80000,160000,640000};
			int[] steps = {2};
			
			Log log = new Log(logPath);
			log.info("################## Test_u_u_Top" + MyConfiguration.TOPK);
			Graph g = new Graph(graphInPath, MyConfiguration.u_u_count[i]);

			for(int step: steps){
				for (int sample : samples){	// 这里step加上之后，路径还要进一步调整
					log.info("computation begin!");
					SingleRandomDev srw = new SingleRandomDev(g,sample,step);
					srw.compute();
					System.out.println("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" +20);
					log.info("第" + i + "个文件  Step:" + step + " Sample:"+sample + "TopK:" + 20);
					log.info("computation done!");
					String outPath = basePath + "_singledev_top" + 20 + "_step" + step + "_sample" + sample + ".txt"; 
					log.info("u_u_graph singleRandomWalk output done!");
					String prePath = basePath + "_singledev_top" + 20 + "_step" + step + "_sample" + sample + "precision.txt";
					// 计算精度，传入sim[][]数组,计算前k个相似的点输出到文件
					Print.printByOrder(srw.getResult(), outPath, MyConfiguration.TOPK, 20);
					log.info("printByOrder done!");
					// 下面精度计算的也一样，和DoubleWalk用的是同一个
					
					String pre = Eval.precision(goldPath+".sim.txt", outPath+".sim.txt", prePath, 20);
					log.info("Basic singledev Top" + MyConfiguration.TOPK + " step" + step + " sample" + sample + " precision: " + pre);
				}
			}
			log.close();
		}
		
	}

}



