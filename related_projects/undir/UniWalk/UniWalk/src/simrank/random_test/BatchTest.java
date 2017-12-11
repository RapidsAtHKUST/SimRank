//package simrank.random_test;
//
//import java.io.IOException;
//
//import lxctools.Log;
//
//import simrank.DoubleRandomWalk;
//import simrank.SimRank;
//import simrank.SingleRandomWalk;
//import structures.Graph;
//import utils.Eval;
//import utils.GraphGenerator;
//import utils.Print;
//import conf.Configuration;
//
//public class BatchTest {
//	public static String testPath = "batchTestResult.txt";
//	public static String prePath = Configuration.basePath
//			+ "/precision.txt";
//	public static String simrankSimPath = Configuration.basePath
//			+ "/simrank_topk.txt";
//	public static String singleRandomPath = Configuration.basePath
//			+ "/singleWalk_topk.txt";
//	public static String doubleRandomPath = Configuration.basePath
//			+ "/singleWalk_topk.txt";
//
//	public static boolean isOnce = true;
//	/**
//	 * GIVEN : topk = 20, SAMPLE = 10000;
//	 */
//	static {
//		Configuration.leftCount = 1000;
//		Configuration.rightCount = Configuration.leftCount;
//		Configuration.totalCount = Configuration.leftCount
//				+ Configuration.rightCount;
//	}
//
//	public static void avgDTest() throws IOException {
//		Configuration.TOPK = 20;
//		SingleRandomWalk.SAMPLE = 10000;
//
//		int[] avgDs = { 5, 20, 50, 100 };
//		for (int a : avgDs) {
//			System.out.println("testing avgD = " + a);
//			Configuration.avgD = a;
//			GraphGenerator.biGraph(Configuration.leftCount,
//					Configuration.rightCount, (int) (Configuration.totalCount
//							* Configuration.avgD / 2),
//					Configuration.biGraphPath);
//			Graph g = new Graph(Configuration.biGraphPath,
//					Configuration.totalCount);
//			Log log = new Log(testPath);
//			log.info("avgDTest! avgD = " + a + " GIVEN: TOPK = "
//					+ Configuration.TOPK + "\t SAMPLE = "
//					+ SingleRandomWalk.SAMPLE);
//			run(log, g);
//		}
//	}
//	
//	public static void avgDTest_doubleWalk() throws IOException {
//		Configuration.TOPK = 20;
//		DoubleRandomWalk.SAMPLE = 100;
//
//		int[] avgDs = { 5, 20, 50, 100 };
//		for (int a : avgDs) {
//			System.out.println("testing DoubleRandowWalk avgD = " + a);
//			Configuration.avgD = a;
//			GraphGenerator.biGraph(Configuration.leftCount,
//					Configuration.rightCount, (int) (Configuration.totalCount
//							* Configuration.avgD / 2),
//					Configuration.biGraphPath);
//			Graph g = new Graph(Configuration.biGraphPath,
//					Configuration.totalCount);
//			Log log = new Log(testPath);
//			log.info("avgDTest! avgD = " + a + " GIVEN: TOPK = "
//					+ Configuration.TOPK + "\t SAMPLE = "
//					+ DoubleRandomWalk.SAMPLE);
//			run_doubleWalk(log, g);
//		}
//	}
//
//	public static void sampleTest() throws IOException {
//		Configuration.TOPK = 20;
//		Configuration.avgD = 100;
//
//		int[] samples = { 10000, 40000, 80000, 160000 };
//		GraphGenerator.biGraph(Configuration.leftCount,
//				Configuration.rightCount, (int) (Configuration.totalCount
//						* Configuration.avgD / 2), Configuration.biGraphPath);
//		Graph g = new Graph(Configuration.biGraphPath, Configuration.totalCount);
//		for (int s : samples) {
//			System.out.println("testing SAMPLE = " + s);
//			SingleRandomWalk.SAMPLE = s;
//			Log log = new Log(testPath);
//			log.info("sampeTest! sample = " + s + "GIVEN : TOPK = "
//					+ Configuration.TOPK + "\t avgD = " + Configuration.avgD);
//			run(log, g);
//		}
//	}
//
//	public static void topkTest() throws IOException {
//		Configuration.avgD = 20;
//		SingleRandomWalk.SAMPLE = 10000;
//
//		int[] topks = { 10, 40, 70, 100, 150, 200 };
//		GraphGenerator.biGraph(Configuration.leftCount,
//				Configuration.rightCount, (int) (Configuration.totalCount
//						* Configuration.avgD / 2), Configuration.biGraphPath);
//		Graph g = new Graph(Configuration.biGraphPath, Configuration.totalCount);
//		for (int t : topks) {
//			System.out.println("testing topk = " + t);
//			Configuration.TOPK = t;
//			Log log = new Log(testPath);
//			log.info("topkTest! topk = " + t + " GIVEN: SAMPLE = "
//					+ SingleRandomWalk.SAMPLE + "\tavgD = "
//					+ Configuration.avgD);
//			run(log, g);
//		}
//	}
//
//	private static void run(Log log, Graph g) throws IOException {
//		log.info("##### SimRank #####");
//		SimRank sr = new SimRank(g);
//		sr.compute();
//		log.info("simrank computation done!");
//		Print.printByOrder(sr.getResult(), simrankSimPath, Configuration.TOPK);
//		log.info("simrank output done!");
//
//		log.info("#### SingleRandomWalk ####");
//		SingleRandomWalk srw = new SingleRandomWalk(g);
//		srw.compute();
//		log.info("SingleRandomWalk computation done!");
//		Print.printByOrder(srw.getResult(), singleRandomPath,
//				Configuration.TOPK);
//		log.info("SingleRandomWalk output done!");
//
//		double precision = Eval.precision(simrankSimPath, singleRandomPath,
//				prePath);
//		log.info("precision: " + precision);
//		log.close();
//	}
//
//	private static void run_doubleWalk(Log log, Graph g) throws IOException {
//		log.info("##### SimRank #####");
//		SimRank sr = new SimRank(g);
//		sr.compute();
//		log.info("simrank computation done!");
//		Print.printByOrder(sr.getResult(), simrankSimPath, Configuration.TOPK);
//		log.info("simrank output done!");
//
//		log.info("#### DoubleRandomWalk ####");
//		System.out.println("double random walk begin:");
//		DoubleRandomWalk dsrw = new DoubleRandomWalk(g);
//		dsrw.compute();
//		log.info("DoubleRandomWalk computation done!");
//		Print.printByOrder(dsrw.getResult(), doubleRandomPath,
//				Configuration.TOPK);
//		log.info("DoubleRandomWalk output done!");
//
//		double precision = Eval.precision(simrankSimPath, doubleRandomPath,
//				prePath);
//		log.info("precision: " + precision);
//		log.close();
//	}
//	public static void main(String[] args) throws IOException {
////		 Log.info(testPath,	"############################ avgDTest! #########################");
////		 BatchTest.avgDTest();
////		Log.info(testPath,"############################ sampleTest! #########################");
////		BatchTest.sampleTest();
//		// Log.info(testPath,
//		// "############################ topkTest! #########################");
//		// BatchTest.topkTest();
//		 
//		 Log.info(testPath,	"############################ avgDTest! doublewalk ! #########################");
//		 BatchTest.avgDTest_doubleWalk();
//	}
//}
