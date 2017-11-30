package conf;

import java.io.IOException;

import utils.GraphGenerator;
import utils.Print;

public class MyConfiguration2 {
	//global setting
	
	public static  String basePath_in = "/Users/songjs/Desktop/workspace/Uniwalk";
	public static  String basePath_out = "/Users/songjs/Desktop/workspace/Uniwalk";
	// public static String realdata = basePath +"/realdata";
	public static  String SEPARATOR = " ";
//	public static  String SEPARATOR = "\t";
	public static final String SEPARATOR_KV = ":";
	public static  int TOPK = 10;
	public static  double MIN = 0.000000001;
	public static  double C = 0.8;
	public static int testTopK[] = {10,20,30,50,100};
	
	// configuration for undirected_unweighted graph
	public static  int u_u_count = 1000;
	public static  int u_u_avgD = 5;
	public static  String in_u_u_graphPath = basePath_in+"/input_u_u/Power_law_1000_";
	public static  String out_u_u_graphPath_simrank = basePath_out+"/output_u_u/SimRank/Power_law_1000_";
	public static  String out_u_u_graphPath_double = basePath_out+"/output_u_u/DoubleWalk/Power_law_1000_";
	public static  String out_u_u_graphPath_single = basePath_out+"/output_u_u/SingleWalk/Power_law_1000_";
	public static  String out_u_u_graphPath_single_dev = basePath_out+"/output_u_u/SingleWalk_dev/Power_law_1000_";
	public static  String out_u_u_graphPath_single_dev2 = basePath_out+"/output_u_u/SingleWalk_dev2/Power_law_1000_";
	
	//configuration for directed_unweighted graph;
	public static  int d_u_count = 1000;
	public static  int d_u_avgD = 50;
	public static  String d_u_graphPath = basePath_in+"/p2p-Gnutella08_Directed_unweighted_0_6300_20777.txt";
	
	
	//configuration for undirected_weighted graph;
	public static  int u_w_count = 1000;
	public static  int u_w_avgD = 5;
	public static  String u_w_graphPath = basePath_in+"/facebook_unDirected_weighted_0_4038_88234.txt";
	
	
	
	//configuration for directed_weighted graph;
	public static  int d_w_count = 1000;
	public static  int d_w_avgD = 50;
	public static  String d_w_graphPath = basePath_in+"/1000_100_directed_weighted.txt";

	
	//configuration for bigraph
	/*
	public static  int leftCount =  50000;
	public static  int rightCount = 50000;
	public static  int totalCount = 100000;//leftCount + rightCount;
	public static  int avgD = 5;
	public static  String biGraphPath =basePath+ "/realdata/movielens_tag_movie.txt";
	*/
	
	// configuration for database;
	public static  int d_count_database = 5242;
	public static  String d_edgeTable = "edges";
	public static  String d_simrankTable = "simrank";
	public static  String d_singleWalkTable = "singleWalk";
	
	
	// configuration for giraph
	public static final String countFlag = "biGraph_10k_5.txt";
	public static final int V =10000;
	public static final int stopV = 1000;
	public static final int machineCount = 10;
	public static final int BATCH_SIZE = 1000;
	

	public static final int SAMPLE = 10000;
	public static final int STEP = 5; // the max length of a double path.
	public static  int CYCLE = STEP * 2 + 2;
//	public static final byte pathLen = STEP ;   // for basic, shouldn't be larget than 127.
	public static  byte pathLen = STEP * 2 + 1 ;   // for reuse, shouldn't be larget than 127.
	public static final byte targetLen = STEP ;
	public static final int M = 3;
	

	
	public static void main(String[] args) throws IOException {
//		GraphGenerator.biGraph(MyConfiguration.leftCount, MyConfiguration.rightCount,(int)(MyConfiguration.totalCount*MyConfiguration.avgD / 2),MyConfiguration.biGraphPath);
		
//		GraphGenerator.dGraph(Configuration.d_count, Configuration.d_count * 
//				Configuration.d_avgD , Configuration.d_graphPath);
		
//		GraphGenerator.graph(Configuration.u_count, (int)(Configuration.u_avgD * Configuration.u_count / 2), Configuration.u_graphPath);
//		Print.edgeFileToDatabase(Configuration.u_graphPath, "edges_test");
//		System.out.println(" graph generation done");
	}
	
}
