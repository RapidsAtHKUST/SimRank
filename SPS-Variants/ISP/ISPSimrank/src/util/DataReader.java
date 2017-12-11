package util;

import java.io.*;

public class DataReader {
	private DataInputStream in;
	private FileInputStream fin;
	
	public DataReader(String filename) throws Exception {
		fin = new FileInputStream(filename);
		in = new DataInputStream(new BufferedInputStream(fin));
	}

	public void close() throws Exception {
		in.close();
	}
	
	public double readDouble() throws Exception {
		return in.readDouble();
	}
	
	public float readFloat(float v) throws Exception {
		return in.readFloat();
	}
	
	public int readInteger() throws Exception {
		return in.readInt();
	}
	
	public short readShort() throws Exception {
		return in.readShort();
	}
	
	public byte readByte() throws Exception {
		return in.readByte();
	}
	
	public boolean readBool() throws Exception {
		return in.readBoolean();
	}
	
	public char readChar() throws Exception {
		return in.readChar();
	}
	
	public String readSmallString() throws Exception {
		return in.readUTF();
	}
	
	public String readBigString() throws Exception {
		StringBuilder sb = new StringBuilder();
		int len = in.readInt();
		for (int i = 0; i < len; i ++) {
			sb.append(in.readChar());
		}
		return sb.toString();
	}
}
