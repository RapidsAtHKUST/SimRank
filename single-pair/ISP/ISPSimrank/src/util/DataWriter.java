package util;

import java.io.*;

public class DataWriter {
	private DataOutputStream out;
	
	public DataWriter(String filename) throws Exception {
		out = new DataOutputStream(new BufferedOutputStream(new FileOutputStream(filename)));
	}
	
	public void close() throws Exception {
		out.close();
	}
	
	public void writeDouble(double v) throws Exception {
		out.writeDouble(v);
	}
	
	public void writeFloat(float v) throws Exception {
		out.writeFloat(v);
	}
	
	public void writeInteger(int v) throws Exception {
		out.writeInt(v);
	}
	
	public void writeShort(short v) throws Exception {
		out.writeShort(v);
	}
	
	public void writeByte(byte v) throws Exception {
		out.writeByte(v);
	}
	
	public void writeBool(boolean v) throws Exception {
		out.writeBoolean(v);
	}
	
	public void writeChar(char v) throws Exception {
		out.writeChar(v);
	}
	
	public void writeSmallString(String v) throws Exception {
		out.writeUTF(v);
	}
	
	public void writeBigString(String v) throws Exception {
		out.writeInt(v.length());
		for (int i = 0; i < v.length(); i ++) {
			out.writeChar(v.charAt(i));
		}
	}
	
	public void flush() throws Exception {
		out.flush();
	}
}
