package utils;

//import java.io.BufferedReader;
import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.HashSet;
import java.util.Set;  

import conf.MyConfiguration;

public class Eval {
	// new 
	public static String precision(String path1, String path2, String prePath, int K) throws IOException{
		// simRank Top20 与 xxxxWalk Top20相比较
		BufferedWriter out = new BufferedWriter(new FileWriter(prePath));
		BufferedReader input1 = new BufferedReader(new FileReader(path1));
		BufferedReader input2 = new BufferedReader(new FileReader(path2));
		String line1 = null, line2 = null;
		double sum = 0;
		int total = 0;
		double min = Double.MAX_VALUE;
		while ((line1 = input1.readLine()) != null){
//			System.out.println(line1);
			line2 = input2.readLine();
			String[] tokens1 = line1.split(MyConfiguration.SEPARATOR);
			String[] tokens2 = line2.split(MyConfiguration.SEPARATOR);
			if (!tokens1[0].equals(tokens2[0])){
				System.out.println("error !" + tokens1[0] +"\t" + tokens2[0]);
				continue;
			}
			
			Set<String> set1 = new HashSet<String>();
			Set<String> set2 = new HashSet<String>();
			for (int i = 1; i < tokens1.length; i++){
				String[] idsim1 = tokens1[i].split(MyConfiguration.SEPARATOR_KV);
				if (Double.valueOf(idsim1[1]) >= MyConfiguration.MIN ) 
					set1.add(idsim1[0]);
			}
			for (int i = 1; i < tokens2.length; i++){
				String[] idsim2 = tokens2[i].split(MyConfiguration.SEPARATOR_KV);
				if (Double.valueOf(idsim2[1]) >= MyConfiguration.MIN)
					set2.add(idsim2[0]);
			}
			
			int realK = Math.min(MyConfiguration.TOPK, set1.size());
			double pre = 1.0;
			if (realK != 0 ){
				set1.retainAll(set2);				// 重合Top K有多少
				pre = 1.0 * set1.size() / realK;
			}
			
			sum += pre;
			out.append(tokens1[0]+MyConfiguration.SEPARATOR + pre +"\r\n");
			
			total++;
			min = Math.min(min, pre);
		}
		out.close();
		System.out.println("total nodes:"+total+"\tavg precision: " + sum / total +"\tmin pre: "+ min );
		return sum / total + "";
	}
	
	/**
	 * use similarity files to get precision.
	 * filter the 0 sim pairs.
	 * @param path1 the simrank similarity file
	 * @param path2 the target similarity file
	 * @param prePath
	 * @return
	 * @throws IOException
	 */
	public static double precision_simFile(String path1, String path2, String prePath) throws IOException{
		BufferedWriter out = new BufferedWriter(new FileWriter(prePath));
		BufferedReader input1 = new BufferedReader(new FileReader(path1));
		BufferedReader input2 = new BufferedReader(new FileReader(path2));
		String line1 = null, line2 = null;
		double sum = 0;
		int total = 0;
		double min = Double.MAX_VALUE;
		while ((line1 = input1.readLine()) != null){
			line2 = input2.readLine();
			String[] tokens1 = line1.split(MyConfiguration.SEPARATOR);
			String[] tokens2 = line2.split(MyConfiguration.SEPARATOR);
			if (!tokens1[0].equals(tokens2[0])){
				System.out.println("error !" + tokens1[0] +"\t" + tokens2[0]);
				continue;
			}
			
			Set<String> set1 = new HashSet<String>();
			Set<String> set2 = new HashSet<String>();
			for (int i = 1; i < tokens1.length; i++){
				String[] idsim1 = tokens1[i].split(MyConfiguration.SEPARATOR_KV);
				if (Double.valueOf(idsim1[1]) >= MyConfiguration.MIN ) 
					set1.add(idsim1[0]);
			}
			for (int i = 1; i < tokens2.length; i++){
				String[] idsim2 = tokens2[i].split(MyConfiguration.SEPARATOR_KV);
				if (Double.valueOf(idsim2[1]) >= MyConfiguration.MIN)
					set2.add(idsim2[0]);
			}
			
			int realK = Math.min(MyConfiguration.TOPK, set1.size());
//			System.out.println(tokens1[0]+ " realK: "+ realK);
			double pre = 0;
			if (realK == 0 ){
				pre = 1.0;
			} else {
				set1.retainAll(set2);
				pre = 1.0 * set1.size() / realK;
			}
			
			sum += pre;
			out.append(tokens1[0]+MyConfiguration.SEPARATOR + pre +"\r\n");
			
			total++;
			min = Math.min(min, pre);
		}
		out.close();
		System.out.println("total nodes:"+total+"\tavg precision: " + sum / total +"\tmin pre: "+ min );
		return sum / total;
	}
	
	/**
	 * just compute the first "maxLineNum" lines.
	 * @param path1
	 * @param path2
	 * @param prePath
	 * @param maxLine
	 * @return
	 * @throws IOException
	 */
	public static double precision_simFile_truncate(String path1, String path2, String prePath, int maxLineNum) throws IOException{
		BufferedWriter out = new BufferedWriter(new FileWriter(prePath));
		BufferedReader input1 = new BufferedReader(new FileReader(path1));
		BufferedReader input2 = new BufferedReader(new FileReader(path2));
		String line1 = null, line2 = null;
		double sum = 0;
		int total = 0;
		int lineCount = 0;
		double min = Double.MAX_VALUE;
		while ((line1 = input1.readLine()) != null){
			if (lineCount >= maxLineNum) break;
			line2 = input2.readLine();
			String[] tokens1 = line1.split(MyConfiguration.SEPARATOR);
			String[] tokens2 = line2.split(MyConfiguration.SEPARATOR);
			if (!tokens1[0].equals(tokens2[0])){
				System.out.println("error !" + tokens1[0] +"\t" + tokens2[0]);
				continue;
			}
			
			Set<String> set1 = new HashSet<String>();
			Set<String> set2 = new HashSet<String>();
			for (int i = 1; i < tokens1.length; i++){
				String[] idsim1 = tokens1[i].split(MyConfiguration.SEPARATOR_KV);
				if (Double.valueOf(idsim1[1]) >= MyConfiguration.MIN ) 
					set1.add(idsim1[0]);
			}
			for (int i = 1; i < tokens2.length; i++){
				String[] idsim2 = tokens2[i].split(MyConfiguration.SEPARATOR_KV);
				if (Double.valueOf(idsim2[1]) >= MyConfiguration.MIN)
					set2.add(idsim2[0]);
			}
			
			int realK = Math.min(MyConfiguration.TOPK, set1.size());
//			System.out.println(tokens1[0]+ " realK: "+ realK);
			double pre = 0;
			if (realK == 0 ){
				pre = 1.0;
			} else {
				set1.retainAll(set2);
				pre = 1.0 * set1.size() / realK;
			}
			
			sum += pre;
			out.append(tokens1[0]+MyConfiguration.SEPARATOR + pre +"\r\n");
			
			total++;
			min = Math.min(min, pre);
			
			lineCount++;
		}
		out.close();
		System.out.println("total nodes:"+total+"\tavg precision: " + sum / total +"\tmin pre: "+ min );
		return sum / total;
	}
	
	
	public static double precision_ndcg(String exactSrkPath, String appSrkPath) throws IOException{
		BufferedReader input1 = new BufferedReader(new FileReader(exactSrkPath));
		BufferedReader input2 = new BufferedReader(new FileReader(appSrkPath));
		String line1 = null, line2 = null;
		double sum = 0;
		int total = 0;
		double min = Double.MAX_VALUE;
		double max = Double.MIN_VALUE;
		while ((line1 = input1.readLine()) != null){
			line2 = input2.readLine();
			String[] tokens1 = line1.split(MyConfiguration.SEPARATOR);
			String[] tokens2 = line2.split(MyConfiguration.SEPARATOR);
			int source = Integer.valueOf(tokens1[0]);
			double zk = 0;
			double appk = 0;
			System.out.println(total+"\t"+tokens1.length);
			for (int i = 1; i < tokens1.length; i++){
				String[]idSim1 = tokens1[i].split(MyConfiguration.SEPARATOR_KV);
				String[]idSim2 = tokens2[i].split(MyConfiguration.SEPARATOR_KV);
				zk += (Math.pow(2,Double.valueOf(idSim1[1])) * Math.log(2)/ Math.log(i+1));
				appk +=( Math.pow(2,Double.valueOf(idSim2[1]) )* Math.log(2)/ Math.log(i+1));
			}
			double ndcg = appk /zk;
			min = Math.min(min, ndcg);
			max = Math.max(max, ndcg);
			total++;
			sum += ndcg;
		}
		input1.close();
		input2.close();
		double avg = sum /total;
		System.out.println("avg NDCG@K: "+ avg +"\t max: "+max +"\tmin: "+min);
		return avg;
	}
	
	public static void main(String[] args) throws IOException {
		// bigraph
//		String path1 = MyConfiguration.basePath+"/movielens_tag_movie_simrank_topk.txt";
//		String path2 = MyConfiguration.basePath+"/movielens_tag_movie_singleRandomWalk_topk.txt";
		
		String path1 = MyConfiguration.basePath + "/simrank_topk_10k_5.txt.sim.txt";
		String path2 = MyConfiguration.basePath +"/singleWalk_topk_10k_5.txt.sim.txt";
//		String path2 = MyConfiguration.basePath +"/power_singleWalk_topk_10k_5.txt.sim.txt";
//		String path1 = MyConfiguration.basePath + "/Test_Real/simrank_moreno_crime_crime.txt.sim.txt";
//		String path2 = MyConfiguration.basePath +"/sigmod14/sigmod14_topk_5k_5.txt.sim.txt";
//		String path2 =  MyConfiguration.basePath +"/sigmod14/sigmod14_topk_moreno_crime_crime.txt.sim.txt";
//		String path2 = Configuration.basePath + "/singleWalk_topk_M.txt";

		// directed graph.
//		String path1 = Configuration.dataBasePath+"/d_simrank_topk.txt";
//		String path2 = Configuration.dataBasePath+"/d_singleWalk_topk.txt";
//		String path2 = Configuration.dataBasePath+"/d_doubleWalk_topk.txt";
		
		// undirected graph
//		String path1 = Configuration.dataBasePath+"/u_simrank_topk.txt";
//		String path2 = Configuration.dataBasePath+"/u_singleWalk_topk.txt";
//		String path2 = Configuration.dataBasePath+"/u_doubleWalk_topk.txt";
		
		// approximate 
//		String path2 = Configuration.dataBasePath + "/singleWalk_topk_approx.txt";
		
		String prePath = MyConfiguration.basePath + "/precision.txt";
		Eval.precision_simFile(path1, path2, prePath);
//		Eval.precision_simFile_truncate(path1, path2, prePath,100);
		
		
	}

}
