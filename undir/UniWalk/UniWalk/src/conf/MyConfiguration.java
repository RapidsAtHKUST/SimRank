package conf;

import java.io.IOException;


public class MyConfiguration {
	// tmp setting
	public static String biGraphPath = "";
	public static int totalCount = -1;
	
	//global setting
	
//	public static  String basePath = "/home/junshuai.sjs/workspace/";
	public static  String basePath = "../";
	public static  String basePath_in = basePath + "data/";
	
	public static String realdata = basePath_in; // tmp
	
	// simrank
	public static  String basePath_out_simrank = basePath + "output/simrank/";
	// single
	public static  String basePath_out_single = basePath + "output/single/";
	public static  String basePath_out_singledev = basePath + "output/singledev/";
	// M
//	public static  String basePath_out_single_M = basePath + "output/single_M/";
//	public static  String basePath_out_singledev_M = basePath + "output/singledev_M/";
	// R:reuse
//	public static  String basePath_out_single_R = basePath + "output/single_R/";
//	public static  String basePath_out_singledev_R = basePath + "output/singledev_R/";

	public static  String basePath_out_sigmod14 = basePath + "output/sigmod14/";
	
	public static  String SEPARATOR = "\t";
	public static final String SEPARATOR_KV = ":";
	public static  int TOPK = 20;
	public static  double MIN = 0.000000001;
	public static  double C = 0.6;
	public static int testTopK[] = {20};	// may be removed later.
	
	// configuration for undirected_unweighted graph
	public static  int fileNum = 8 + 12;
//	public static  int u_u_count[] = {1380,38741,124325,511463,972120,5425963,10690276,38289740, 300000000, 1000000000};	
	public static  int u_u_count[] = {1380,38741,124325,511463,972120,5425963,10690276,38289740,
		100000,300000,1000000,3000000,10000000,30000000,
		100000,300000,1000000,3000000,10000000,30000000};	// 8,14    15,21
											//	100000000
		
	// 淘宝图: 3208894849条边
	public static  String in_u_u_graphPath[] = {
		basePath_in+"real/crime.txt",
		basePath_in+"real/arxiv.txt",
		basePath_in+"real/youtube.txt",
		basePath_in+"real/actors.txt",
		basePath_in+"real/bibs.txt",
		basePath_in+"real/dblp.txt",
		basePath_in+"real/livej.txt",
		basePath_in+"real/delicious.txt",
		basePath_in+"power/powerlaw_100K.txt",
		basePath_in+"power/powerlaw_300K.txt",
		basePath_in+"power/powerlaw_1M.txt",
		basePath_in+"power/powerlaw_3M.txt",
		basePath_in+"power/powerlaw_10M.txt",
		basePath_in+"power/powerlaw_30M.txt",
//		basePath_in+"power/powerlaw_100M.txt",
//		basePath_in+"power/powerlaw_300M.txt",
		basePath_in+"random/random_100K.txt",
		basePath_in+"random/random_300K.txt",
		basePath_in+"random/random_1M.txt",
		basePath_in+"random/random_3M.txt",
		basePath_in+"random/random_10M.txt",
		basePath_in+"random/random_30M.txt",
		basePath_in+"random/random_100M.txt"
//		basePath_in+"random/random_300M.txt"
	};
	
	// file names.
	public static  String out_u_u_graphPath_simrank[] = {
		basePath_out_simrank+"crime.txt",
		basePath_out_simrank+"arxiv.txt",
		basePath_out_simrank+"youtube.txt",
		basePath_out_simrank+"actors.txt",
		basePath_out_simrank+"bibs.txt",
		basePath_out_simrank+"dblp.txt",
		basePath_out_simrank+"livej.txt",
		basePath_out_simrank+"delicious.txt",
		basePath_out_simrank+"powerlaw_300M.txt",
		basePath_out_simrank+"powerlaw_1000M.txt"
	};
	
	public static  String out_u_u_graphPath_singledev[] = {
		basePath_out_singledev+"crime.txt",
		basePath_out_singledev+"arxiv.txt",
		basePath_out_singledev+"youtube.txt",
		basePath_out_singledev+"actors.txt",
		basePath_out_singledev+"bibs.txt",
		basePath_out_singledev+"dblp.txt",
		basePath_out_singledev+"livej.txt",
		basePath_out_singledev+"delicious.txt",
		basePath_out_singledev+"powerlaw_100K.txt",
		basePath_out_singledev+"powerlaw_300K.txt",
		basePath_out_singledev+"powerlaw_1M.txt",
		basePath_out_singledev+"powerlaw_3M.txt",
		basePath_out_singledev+"powerlaw_10M.txt",
		basePath_out_singledev+"powerlaw_30M.txt",
		basePath_out_singledev+"powerlaw_100M.txt",
//		basePath_out_singledev+"powerlaw_300M.txt",
		basePath_out_singledev+"random_100K.txt",
		basePath_out_singledev+"random_300K.txt",
		basePath_out_singledev+"random_1M.txt",
		basePath_out_singledev+"random_3M.txt",
		basePath_out_singledev+"random_10M.txt",
		basePath_out_singledev+"random_30M.txt",
		basePath_out_singledev+"random_100M.txt"
//		basePath_out_singledev+"random_300M.txt"
		
	};
	
	public static  String out_u_u_graphPath_single[] = {
		basePath_out_single+"crime.txt",
		basePath_out_single+"arxiv.txt",
		basePath_out_single+"youtube.txt",
		basePath_out_single+"actors.txt",
		basePath_out_single+"bibs.txt",
		basePath_out_single+"dblp.txt",
		basePath_out_single+"livej.txt",
		basePath_out_single+"delicious.txt",
		basePath_out_single+"powerlaw_100K.txt",
		basePath_out_single+"powerlaw_300K.txt",
		basePath_out_single+"powerlaw_1M.txt",
		basePath_out_single+"powerlaw_3M.txt",
		basePath_out_single+"powerlaw_10M.txt",
		basePath_out_single+"powerlaw_30M.txt",
		basePath_out_single+"powerlaw_100M.txt",
//		basePath_out_single+"powerlaw_300M.txt",
		basePath_out_single+"random_100K.txt",
		basePath_out_single+"random_300K.txt",
		basePath_out_single+"random_1M.txt",
		basePath_out_single+"random_3M.txt",
		basePath_out_single+"random_10M.txt",
		basePath_out_single+"random_30M.txt",
		basePath_out_single+"random_100M.txt"
//		basePath_out_single+"random_300M.txt"
		
	};
	
	public static  String out_u_u_graphPath_sigmod14[] = {
		basePath_out_sigmod14+"crime.txt",
		basePath_out_sigmod14+"arxiv.txt",
		basePath_out_sigmod14+"youtube.txt",
		basePath_out_sigmod14+"actors.txt",
		basePath_out_sigmod14+"bibson.txt",
		basePath_out_sigmod14+"dblp.txt",
		basePath_out_sigmod14+"livej.txt",
		basePath_out_sigmod14+"delicious.txt",
		basePath_out_sigmod14+"powerlaw_100K.txt",
		basePath_out_sigmod14+"powerlaw_300K.txt",
		basePath_out_sigmod14+"powerlaw_1M.txt",
		basePath_out_sigmod14+"powerlaw_3M.txt",
		basePath_out_sigmod14+"powerlaw_10M.txt",
		basePath_out_sigmod14+"powerlaw_30M.txt",
		basePath_out_sigmod14+"random_100K.txt",
		basePath_out_sigmod14+"random_300K.txt",
		basePath_out_sigmod14+"random_1M.txt",
		basePath_out_sigmod14+"random_3M.txt",
		basePath_out_sigmod14+"random_10M.txt",
		basePath_out_sigmod14+"random_30M.txt"
	};

	
	public static void main(String[] args) throws IOException {

	
	}
	
}
