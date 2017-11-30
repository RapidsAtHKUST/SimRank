package giraph;

import org.apache.giraph.edge.Edge;
import org.apache.giraph.graph.Vertex;
import org.apache.giraph.io.formats.GiraphFileInputFormat;
import org.apache.giraph.io.formats.IntNullReverseTextEdgeInputFormat;
import org.apache.giraph.job.GiraphJob;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.mapreduce.lib.output.FileOutputFormat;

import utils.EJob;
import giraph.ioformat.SimilaritytVertexOutputFormat;
import giraph.writables.IntArrayWritable;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;

import lxctools.FixedHashMap;

/**
 * be careful about outputformat!
 * @author luoxiongcai
 *
 */ 
public class SingleWalkVertexDev extends
		Vertex<IntWritable, NullWritable, NullWritable, IntArrayWritable> {
	public static String countFlag = "10k_5";
	public static int machineCount = 9;
	public static final float C  = 0.6f;
	public static final int SAMPLE = 10000;
	public static final int STEP = 6;
	public static final int TOPK = 20;
	public static final int M = 5;
	public static final int MAX_MSG_SLOT = STEP;
	public static final IntWritable MINUS_ONE = new IntWritable(-1); // indicate null;
	public static Random rand = new Random();
	
	
	private FixedHashMap results = new FixedHashMap(M * TOPK);
	
	
	public FixedHashMap getResults(){
		return this.results;
	}
	
	public IntWritable randNeighbor(){
		RandOutEdges<IntWritable, NullWritable> edges = (RandOutEdges<IntWritable, NullWritable>) this.getEdges();
		Edge<IntWritable, NullWritable> next = edges.randEdge();
		if (next == null)  return MINUS_ONE;
		else return next.getTargetVertexId();
	}
	
	@Override
	public void compute(Iterable<IntArrayWritable> messages) throws IOException {
		
		if (getNumEdges() == 0) voteToHalt();  // single point.
		int step = (int) this.getSuperstep();
		
		
		if (step == 0) {
			// songjs: only deal some calculation. !!!
			if((int)this.getId().get() < 1000){
				for (int i = 0; i < SAMPLE; i++){
					IntArrayWritable msg = new IntArrayWritable(MAX_MSG_SLOT + 1);
					msg.set(0, this.getId().get());			// degree数组0位置，保存初始顶点id(后面用)
					sendMessage(randNeighbor(), msg);
				}
			}
		} else if ( step <= STEP * 2){
			Iterator<IntArrayWritable> msgs = messages.iterator();
			while (msgs.hasNext()){
				IntArrayWritable msg =  msgs.next();
				// deal with possible similarity increments.
				int target  = msg.getTarget();
				if (target != -1){
					results.add(target, msg.getSim());
					continue;
				}
				if(step <= STEP){
					msg.set(step, getNumEdges());		// 保存第step个节点度数
				}
				if ( step % 2 == 0){  // even path . store the increment similarity.
					float incre =(float) Math.pow(C, step / 2) * msg.get(step / 2) / getNumEdges() / SAMPLE;
					sendMessage(new IntWritable(msg.getSource()), new IntArrayWritable(getId().get(), incre));
				}
				sendMessage(randNeighbor(), msg);
				
			}
			
		} else {
			voteToHalt();
		}
	}

	public static void main(String[] args) throws IOException,
			InterruptedException, ClassNotFoundException {

		Path inputPath = new Path("hdfs://xiaoyueali5:54310/home/junshuai.sjs/simrank/input/crime.txt");
		Path outputPath = new Path("hdfs://xiaoyueali5:54310/home/junshuai.sjs/simrank/output/crime_naive.txt");
		

		GiraphJob job = new GiraphJob(new Configuration(), "s_walk_test_"+countFlag);
		
//		job.getConfiguration().setWorkerContextClass(workerContextClass)
		
		job.getConfiguration().set("hadoop.job.ugi", "luoxc,supergroup");
		job.getConfiguration().set("mapred.child.java.opts", "-Xms35g -Xmx35g -XX:+UseSerialGC");
		job.getConfiguration().set("mapred.job.map.memory.mb", "35000");
		File jarFile = EJob.createTempJar("bin");
		job.getConfiguration().set("mapred.jar", jarFile.getAbsolutePath());
//		job.getInternalJob().setJarByClass(getClass());
		
		job.getConfiguration().setVertexClass(SingleWalkVertexDev.class);
		
		// random outedge class.
		job.getConfiguration().setOutEdgesClass(RandOutEdges.class);

		job.getConfiguration().setEdgeInputFormatClass(  // be careful to synchronize with .addVertex(edge)InputPath.
				IntNullReverseTextEdgeInputFormat.class);
		job.getConfiguration().setVertexOutputFormatClass(
				SimilaritytVertexOutputFormat.class);

		GiraphFileInputFormat.addEdgeInputPath(job.getConfiguration(), inputPath);
		FileOutputFormat.setOutputPath(job.getInternalJob(), outputPath);

		job.getConfiguration().setWorkerConfiguration(machineCount,machineCount, 100.0f);

		FileSystem fs = FileSystem.get(job.getConfiguration());
		if (fs.exists(outputPath)) {
			fs.delete(outputPath, true);
		}
		boolean isVerbose = true;
		job.run(isVerbose);
		
	}
	

}
