package utils;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

public class CleanRealData {

	/**
	 * @param args
	 * @throws IOException 
	 */
	public static void main(String[] args) throws IOException {
		String base = "K:/lxcdata/lshrank_data/realdata";
		
		String path =base + "/out.citeulike-ti";
		String outPath = base +"/citeulike-ti.txt";
		String sep = " ";
		int skipLine = 0;
		int increNum = 153277;
		 
		BufferedReader realIn = new BufferedReader(new FileReader(path));
		BufferedWriter realOut = new BufferedWriter(new FileWriter(outPath));
		String line = "";
		int i = 0;
		while((line = realIn.readLine())!=null){
			i++;
			if (i <= skipLine) continue;
			String[] tokens = line.split(sep);
			if (tokens.length < 2) {
				System.out.println(line);
				continue;
			}
			int leftId = Integer.valueOf(tokens[0]) - 1;
			int rightId = Integer.valueOf(tokens[1]) - 1 + increNum;
			realOut.append(leftId +"\t" + rightId +"\n");
			if (i % 10000 == 0)
				System.out.println(i);
		}
		realIn.close();
		realOut.close();
		System.out.println("done, real edges: "+i);
	}

}
