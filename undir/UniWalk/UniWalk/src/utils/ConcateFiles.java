package utils;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.File;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;

import conf.MyConfiguration;

public class ConcateFiles {


    public static void concatFiles(String srcDir, String targetFile) throws IOException{
    	File dir = new File(srcDir);
    	BufferedWriter output = new BufferedWriter(new FileWriter(targetFile));
    	for (File f: dir.listFiles()){
    		BufferedReader buff = new BufferedReader(new FileReader(f));
    		String line = "";
    		while((line = buff.readLine())!=null){
    			output.append(line+"\r\n");
    		}
    		buff.close();
    		f.delete();
    	}
    	
    	output.close();
    } 
	public static void main(String[] args) throws IOException {
		String dir = MyConfiguration.basePath+"/biGraph_100m_5";
		String output = MyConfiguration.basePath + "/biGraph_100m_5.txt";
		concatFiles(dir,output);
		System.out.println("done");
	}

}
