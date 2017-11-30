package lxctools;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Date;

import structures.Graph;

public class Log {
	private long stime = System.currentTimeMillis();
	private BufferedWriter output;
	public Log(String path){
		try {
			output = new BufferedWriter(new FileWriter(path,true));
			output.append("\r\nBEGIN TIME: " + new Date() +"\r\n");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

	public void info(String msg){
		try {
			output.append("DURATION: " + (System.currentTimeMillis() - stime) /1000.0 +"s\t=>\t"+ msg +"\r\n");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public void print(int[] ps,Graph g){
		try {
			for (int p : ps)
				output.append(p+"("+g.degree(p)+") ");
			output.append("\r\n");
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	public void close(){
		try {
			output.append("END TIME: "+new Date() + "\r\n");
			output.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}
	
	/**
	 * open and close the file may be very inefficient.
	 * @param path
	 * @param msg
	 */
	public static void info(String path, String msg){
		BufferedWriter out = null;
		try {
			out = new BufferedWriter(new FileWriter(path,true));
			out.append("\r\n"+msg+"\t\t"+ new Date());
			out.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} 
	}
	
	public void flush(){
		if (output != null){
			try {
				output.flush();
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}
	}

	public static void main(String[] args) {
		// TODO Auto-generated method stub

	}

}
