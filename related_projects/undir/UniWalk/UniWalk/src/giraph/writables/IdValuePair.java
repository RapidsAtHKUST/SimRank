package giraph.writables;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;

import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;

public class IdValuePair implements Writable , WritableComparable {

	public  int id;
	public double value;
	public IdValuePair(){}
	public IdValuePair(int id,double value){
		this.id=id;
		this.value=value;
	}
	@Override
	public void write(DataOutput out) throws IOException {
		out.writeInt(id);
		out.writeDouble(value);
	}

	@Override
	public void readFields(DataInput in) throws IOException {
		this.id=in.readInt();
		this.value=in.readDouble();
	}

	@Override
	public boolean equals(Object other){
		IdValuePair o = (IdValuePair) other;
		return (o.id==id && o.value==value);
	}
	
	@Override
	public int hashCode(){
		return (new Integer(id)).hashCode()*163+(new Double(value)).hashCode();
	}
	@Override
	public int compareTo(Object o) {
		IdValuePair pair=(IdValuePair)o;
		if(this.value<pair.value) return -1;
		else return 1;		
	}
	
	@Override
	public String toString(){
		return id+":"+value;
	}

	public static void main(String[]arg){
		ArrayList<IdValuePair> list=new ArrayList<IdValuePair>();
		list.add(new IdValuePair(1,0.3));
		list.add(new IdValuePair(2,0.4));
		list.add(new IdValuePair(3,0.1));
		System.out.println(list.contains(new IdValuePair(1,0.3)));
		System.out.println(list.contains(new IdValuePair(1,0.4)));
		Collections.sort(list);
		for(int i=0;i<list.size();i++)
	    System.out.println(list.get(i).toString());
	}

	
}
