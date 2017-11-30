package utils;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import conf.MyConfiguration;

public class LogProcess {

	/**
	 * @param args
	 * @throws IOException 
	 */
	public static void extractMsg() throws IOException{
		int cycle  = 22; 
		int targetStep = 9;
		String path = MyConfiguration.basePath +"/path_sharing/1m_power4/log.html";
		String outAll =MyConfiguration.basePath +"/path_sharing/1m_power4/msg_all_1m_5.txt";
		String outOne =MyConfiguration.basePath +"/path_sharing/1m_power4/msg_sample_1m_5.txt";
		String regex = "Completed superstep (\\d+) with global stats.*msgCount=(\\d+)";
		Pattern pattern = Pattern.compile(regex);
		BufferedReader log = new BufferedReader(new FileReader(path));
		BufferedWriter log_outAll = new BufferedWriter(new FileWriter(outAll));
		BufferedWriter log_outOne = new BufferedWriter(new FileWriter(outOne));
		String line = "";
		int i = 0;
		while ((line = log.readLine())!=null){
			Matcher match = pattern.matcher(line);
			if(match.find()){
				String step = match.group(1);
				String msgCount = match.group(2);
				log_outAll.append(step+"\t"+msgCount+"\n");
				if ( i % cycle == targetStep){
					log_outOne.append(step +"\t"+msgCount+"\n");
				}
				i++;
			}
		}
		log.close();
		log_outAll.close();
		log_outOne.close();
		System.out.println("done");
	}
	public static void main(String[] args) throws IOException {
		extractMsg();
		
//		String line = "eWorker: finishSuperstep: Completed superstep 1 with global stats (vtx=9937,finVtx=0,edges=50000,msgCount=50,haltComputation=false)";
//		String regex = "Completed superstep (\\d+) with global stats.*msgCount=(\\d+)";
//		Pattern pattern = Pattern.compile(regex);
//		Matcher match = pattern.matcher(line);
//		if(match.find()){
//			System.out.println(match.group(0));
//			System.out.println(match.group(1));
//			System.out.println(match.group(2));
//		}
	}

}
