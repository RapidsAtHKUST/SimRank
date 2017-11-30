package util;

import java.io.*;

public class TextReader {
	private boolean ignoreEmptyLine;
	private String[] ignoreStart;
	
	private BufferedReader in;
	
	public TextReader(String filename) throws Exception {
		in = new BufferedReader(new InputStreamReader(new FileInputStream(filename), "UTF-8")); 
		ignoreEmptyLine = true;
		ignoreStart = new String[0];
	}
	
	public void setIgnoreEmptyLine(boolean val) {
		this.ignoreEmptyLine = val;
	}
	
	public void setIgnoreStart(String[] words) {
		ignoreStart = new String[words.length];
		for (int i = 0; i < words.length; i++)
			ignoreStart[i] = words[i];
	}
	
	public boolean hasIgnoredStart(String line) {
		for (String s : ignoreStart)
			if (line.startsWith(s))
				return true;
		
		return false;
	}
	
	public void close() throws Exception {
		in.close();
	}
	
	public String readln() throws Exception {
		String s = in.readLine();
		while (s != null) {
			s = s.trim();
			if ( (ignoreEmptyLine && s.isEmpty()) || hasIgnoredStart(s))
				s = in.readLine();
			else
				break;
		}
		return s;
	}
	
	public String readOrigninalLine() throws Exception {
		return in.readLine();
	}
		
}
