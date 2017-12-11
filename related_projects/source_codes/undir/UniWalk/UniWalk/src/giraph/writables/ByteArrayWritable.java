package giraph.writables;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Arrays;

import org.apache.hadoop.io.Writable;

public class ByteArrayWritable implements Writable {
	int src = -1;
	int len = 0;
	byte[] degrees;
	int target = -1;
	float sim = 0;
	public ByteArrayWritable(){
		
	}
	public ByteArrayWritable(int len){
		this.len = len;
		degrees = new byte[len];
	}
	
	public ByteArrayWritable(int target, float sim){
		this.target = target;
		this.sim = sim;
	}
	
	@Override
	public void readFields(DataInput input) throws IOException {
		this.src = input.readInt();
		this.len = input.readInt();
		this.degrees = new byte[len];
		for (int i = 0; i < len; i++)
			degrees[i] = input.readByte();
		this.target = input.readInt();
		this.sim = input.readFloat();
	}

	@Override
	public void write(DataOutput output) throws IOException {
		output.writeInt(src);
		output.writeInt(len);
		for (int i = 0; i < len; i++)
			output.writeByte(degrees[i]);
		output.writeInt(target);
		output.writeFloat(sim);
	}
	
	public void set(int target, float sim){
		this.target = target;
		this.sim = sim;
	}
	
	public int getTarget(){
		return target;
	}
	
	public float getSim(){
		return sim;
	}
	
	public void set(int index, byte value){
		degrees[index] = value;
	}
	

	public void setSource(int srcId){
		this.src = srcId;
	}
	public byte get(int index){
		return degrees[index];
	}
	
	public int getSource(){
		return src;
	}
	
	public int size(){
		return len;
	}

	@Override
	public String toString() {
		return "ByteArrayWritable [degrees=" + Arrays.toString(degrees) + ", len="
				+ len + "]";
	}

}
