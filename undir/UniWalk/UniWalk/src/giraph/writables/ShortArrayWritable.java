package giraph.writables;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Arrays;

import org.apache.hadoop.io.Writable;
/**
 * flag = 1 : path msg
 * flag = 2 : similarity msg;
 * @author luoxiongcai
 *
 */
public class ShortArrayWritable implements Writable {
	byte flag;
	short pathCount = 0;
	
	int src = -1;
	byte len = 0;
	short[] degrees;

	int target = -1;
	float sim = 0;
	
	public ShortArrayWritable(){
		
	}
	public ShortArrayWritable(byte flag, int src, byte len, short pathCount){
		this.flag = flag;
		this.src = src;
		this.len = len;
		this.degrees = new short[len];
		this.pathCount = pathCount;
	}
	
	public ShortArrayWritable(byte flag, int target, float sim, short pathCount){
		this.flag = flag;
		this.target = target;
		this.sim = sim;
		this.pathCount = pathCount;
	}
	
	@Override
	public void readFields(DataInput input) throws IOException {
		this.flag = input.readByte();
		this.pathCount = input.readShort();
		if (flag == 1){
			this.src = input.readInt();
			this.len = input.readByte();
			this.degrees = new short[len];
			for (int i =0; i < len; i++)
				degrees[i] = input.readShort();
			
		}else if (flag == 2){
			this.target = input.readInt();
			this.sim = input.readFloat();
		}
	}

	@Override
	public void write(DataOutput output) throws IOException {
		output.writeByte(flag);
		output.writeShort(pathCount);
		if (flag == 1){
			output.writeInt(src);
			output.writeByte(len);
			for (int i = 0; i < len; i++){
				output.writeShort(degrees[i]);
			}
		} else if (flag == 2){
			output.writeInt(target);
			output.writeFloat(sim);
		}
	}
	
	
	
	public byte getFlag() {
		return flag;
	}
	public void setFlag(byte flag) {
		this.flag = flag;
	}
	
	
	
	public short getPathCount() {
		return pathCount;
	}
	public void setPathCount(short pathCount) {
		this.pathCount = pathCount;
	}
	


	
	public int getSrc() {
		return src;
	}
	public void setSrc(int src) {
		this.src = src;
	}
	public byte getLen() {
		return len;
	}
	public void setLen(byte len) {
		this.len = len;
	}
	
	
	public short[] getDegrees() {
		return degrees;
	}
	public void setDegrees(short[] degrees) {
		this.degrees = degrees;
	}
	
	public short getDegree(int index){
		return degrees[index];
	}
	
	public void setDegree(int index, short d){
		this.degrees[index] = d;
	}
	
	public int getTarget() {
		return target;
	}
	public void setTarget(int target) {
		this.target = target;
	}
	public float getSim() {
		return sim;
	}
	public void setSim(float sim) {
		this.sim = sim;
	}
	public int size(){
		return len;
	}
	@Override
	public String toString() {
		return "ShortArrayWritable [flag=" + flag + ", src=" + src + ", len="
				+ len + ", degrees=" + Arrays.toString(degrees) + ", target="
				+ target + ", sim=" + sim + "]";
	}



}
