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
 * @author luoxiongcai
 *
 */
public class MixMsgWritable implements Writable {
	byte flag = 0; // indicate type of the message.
	
	int pathLen;
	byte srcp;  //source pointer to indicate the start of the path. the path is a circular array.
	int[] path;
	byte[] degrees;
	
	int targetLen;
	int[] targets;
	float[] sims;
	
	public MixMsgWritable(){}
	
	// flag should be 1,or 2.
	public MixMsgWritable(byte flag, int len){
		this.flag = flag;
		if (flag == 1){
			this.pathLen = len;
			this.srcp = 0;
			this.path = new int[pathLen];
			this.degrees = new byte[pathLen];
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
			out.writeInt(pathLen);
			out.writeByte(srcp);
			for (int p : path){
				out.writeInt(p);
			}
			for (byte d : degrees){
				out.writeByte(d);
			}
		} else if(flag == 2){
			out.writeInt(targetLen);
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
			this.pathLen = in.readInt();
			this.srcp = in.readByte();
			this.path = new int[pathLen];
			this.degrees = new byte[pathLen];
			for (int i = 0; i < pathLen; i++){
				path[i] = in.readInt();
			}
			for (int i = 0; i < pathLen; i++){
				degrees[i] = in.readByte();
			}
		} else if (flag == 2){
			this.targetLen = in.readInt();
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

	public int getPathLen() {
		return pathLen;
	}

	public byte getSrcp() {
		return srcp;
	}

	public int[] getPath() {
		return path;
	}

	public byte[] getDegrees() {
		return degrees;
	}

	public int getTargetLen() {
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

	public void setPathLen(int pathLen) {
		this.pathLen = pathLen;
	}

	public void setSrcp(byte srcp) {
		this.srcp = srcp;
	}

	public void setPath(int[] path) {
		this.path = path;
	}

	public void setDegrees(byte[] degrees) {
		this.degrees = degrees;
	}

	public void setTargetLen(int targetLen) {
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
