package utils;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import conf.MyConfiguration;

public class SuperstepTimes {
	static String basePath = MyConfiguration.basePath +"/path_sharing/1m_power4";
	static String fileName = "raw_supertimes.txt";
	static int maxStep = 1000000;
	static int cycle = 22;

	public static void main(String[] args) throws IOException {
		
		BufferedReader buff = new BufferedReader(new FileReader(basePath+"/"+fileName));
		BufferedWriter out_all = new BufferedWriter(new FileWriter(basePath+"/"+"steptime_all.txt"));
		BufferedWriter out_one = new BufferedWriter(new FileWriter(basePath+"/"+"steptime_one.txt"));
		BufferedWriter out_batch = new BufferedWriter(new FileWriter(basePath+"/"+"steptime_batch.txt"));
		BufferedWriter out_batch_no_output = new BufferedWriter(new FileWriter(basePath+"/"+"steptime_batch_no_output.txt"));
		String line ="";
		int[] times = new int[maxStep];
		
		int realStep = 0; 
		while((line = buff.readLine())!=null){
			if (!line.startsWith("Superstep")) {
				System.out.println(line);
				continue;
			}
			String[] tokens = line.split("\t");
			int time = Integer.valueOf(tokens[1].replace(",", "")) / 1000;
			int step = Integer.valueOf(tokens[0].split(" ")[1]);
			realStep = Math.max(realStep, step);
			times[step] = time;
			
		}
		
		//all steps
		int batchTime = 0;
		for (int i = 0; i <= realStep; i++){
			out_all.append(i+"\t"+times[i]+"\r\n");
			batchTime += times[i];
			if ((i+1) % cycle == 0){
				out_batch.append((i/cycle) + "\t"+ batchTime+"\r\n");
				out_batch_no_output.append((i/cycle) + "\t"+ (batchTime-times[i])+"\r\n");
				batchTime = 0;
			}
		}
			
		out_all.close();
		out_batch.close();
		out_batch_no_output.close();
		
		// one step in each batch.
		for (int i = cycle - 2; i <= realStep; i += cycle){
			out_one.append(i+"\t"+times[i]+"\r\n");
		}
		out_one.close();
		

		System.out.println("done!");
	}

}
