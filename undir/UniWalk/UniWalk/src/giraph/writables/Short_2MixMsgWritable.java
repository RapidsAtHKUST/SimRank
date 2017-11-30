package giraph.writables;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Arrays;

import org.apache.hadoop.io.Writable;

/**
 * message of two mixed types.
 * flag:
 * 1.for path message:
 * 2.for similarity message:
 * byte pathLen: the lenght of a path should not be larger than 127.
 * byte targetLen:  no more than 127.
 * @author luoxiongcai
 *
 */
public class Short_2MixMsgWritable implements Writable {
	byte flag = 0; // indicate type of the message.
	
	byte pathLen;
	byte srcp;  //source pointer to indicate the start of the path. the path is a circular array.
	int[] path;
	short[] degrees;
	
	byte targetLen;
	int[] targets;
	float[] sims;
	
	public Short_2MixMsgWritable(){}
	
	// flag should be 1,or 2.
	public Short_2MixMsgWritable(byte flag, byte len){
		this.flag = flag;
		if (flag == 1){
			this.pathLen = len;
			this.srcp = 0;
			this.path = new int[pathLen];
			this.degrees = new short[pathLen];
		} else if (flag == 2){
			this.targetLen =  len;
			this.targets = new int[targetLen];
			this.sims = new float[targetLen];
		}
	}
	
	@Override
	public void write(DataOutput out) throws IOException {
		out.writeByte(flag);
		if (flag == 1){
			out.writeByte(pathLen);
			out.writeByte(srcp);
			for (int p : path){
				out.writeInt(p);
			}
			for (short d : degrees){
				out.writeShort(d);
			}
		} else if(flag == 2){
			out.writeByte(targetLen);
			for (int t : targets){
				out.writeInt(t);
			}
			for (float s : sims){
				out.writeFloat(s);
			}
		}
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		this.flag = in.readByte();
		if (flag == 1){
			this.pathLen = in.readByte();
			this.srcp = in.readByte();
			this.path = new int[pathLen];
			this.degrees = new short[pathLen];
			for (int i = 0; i < pathLen; i++){
				path[i] = in.readInt();
			}
			for (int i = 0; i < pathLen; i++){
				degrees[i] = in.readShort();
			}
		} else if (flag == 2){
			this.targetLen = in.readByte();
			this.targets = new int[targetLen];
			this.sims = new float[targetLen];
			for (int i = 0; i < targetLen; i++){
				targets[i] = in.readInt();
			}
			for (int i = 0; i < targetLen; i++){
				sims[i] = in.readFloat();
			}
		}
	}

	public int getSource(){
		return path[srcp];
	}
	
	public void setDirectPath(int index, int id, byte degree){
		path[index] = id;
		degrees[index] = degree;
	}
	
	public void appendCircularPath(int id, byte degree){
		path[srcp] = id;
		degrees[srcp] = degree;
		srcp = (byte) ((srcp+1) % pathLen);
	}

	public byte getFlag() {
		return flag;
	}

	public byte getPathLen() {
		return pathLen;
	}

	public byte getSrcp() {
		return srcp;
	}

	public int[] getPath() {
		return path;
	}

	public short[] getDegrees() {
		return degrees;
	}

	public byte getTargetLen() {
		return targetLen;
	}

	public int[] getTargets() {
		return targets;
	}

	public float[] getSims() {
		return sims;
	}

	public void setFlag(byte flag) {
		this.flag = flag;
	}

	public void setPathLen(byte pathLen) {
		this.pathLen = pathLen;
	}

	public void setSrcp(byte srcp) {
		this.srcp = srcp;
	}

	public void setPath(int[] path) {
		this.path = path;
	}

	public void setDegrees(short[] degrees) {
		this.degrees = degrees;
	}

	public void setTargetLen(byte targetLen) {
		this.targetLen = targetLen;
	}

	public void setTargets(int[] targets) {
		this.targets = targets;
	}

	public void setSims(float[] sims) {
		this.sims = sims;
	}
	


	@Override
	public String toString() {
		return "MixMsgWritable [flag=" + flag + ", pathLen=" + pathLen
				+ ", srcp=" + srcp + ", path=" + Arrays.toString(path)
				+ ", degrees=" + Arrays.toString(degrees) + ", targetLen="
				+ targetLen + ", targets=" + Arrays.toString(targets)
				+ ", sims=" + Arrays.toString(sims) + "]";
	}

	
	

}
