package giraph.writables;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Arrays;

import org.apache.hadoop.io.Writable;

public class IntArrayWritable implements Writable {
	int len = 0;
	int[] degrees;
	int target = -1;
	float sim = 0;
	public IntArrayWritable(){}
	
	public IntArrayWritable(int len){
		this.len = len;
		degrees = new int[len];
	}
	
	public IntArrayWritable(IntArrayWritable other){
		this.len = other.len;
		this.degrees = Arrays.copyOf(other.degrees, len);
	}
	
	public IntArrayWritable(int target, float sim){
		this.target = target;
		this.sim = sim;
	}
	
	@Override
	public void readFields(DataInput input) throws IOException {
		this.len = input.readInt();
		this.degrees = new int[len];
		for (int i = 0; i < len; i++)
			degrees[i] = input.readInt();
		this.target = input.readInt();
		this.sim = input.readFloat();
	}

	@Override
	public void write(DataOutput output) throws IOException {
		output.writeInt(len);
		for (int i = 0; i < len; i++)
			output.writeInt(degrees[i]);
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
	
	public void set(int index, int value){
		degrees[index] = value;
	}
	
	public int get(int index){
		return degrees[index];
	}
	
	public int getSource(){
		return degrees[0];
	}
	
	public int size(){
		return len;
	}

	@Override
	public String toString() {
		return "IntArrayWritable [degrees=" + Arrays.toString(degrees) + ", len="
				+ len + "]";
	}

}
