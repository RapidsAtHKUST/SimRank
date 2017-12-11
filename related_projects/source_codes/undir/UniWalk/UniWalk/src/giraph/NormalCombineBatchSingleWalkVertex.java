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

import conf.MyConfiguration;
import conf.MyConfiguration2;
import utils.EJob;
import giraph.ioformat.NoReuseBatchSimilaritytVertexOutputFormat;
import giraph.writables.ShortArrayWritable;

import java.io.File;
import java.io.IOException;
import java.util.Arrays;
import java.util.Iterator;
import java.util.Random;

import lxctools.FixedCacheMap;

/**
 * Implementation of PageRank in which vertex ids are ints, page rank values are
 * floats, and graph is unweighted.
 */
public class NormalCombineBatchSingleWalkVertex extends
		Vertex<IntWritable, NullWritable, NullWritable, ShortArrayWritable> {
	public static final String countFlag = MyConfiguration2.countFlag;
	public static final int V = MyConfiguration2.V;
	public static final int stopV = MyConfiguration2.stopV;
	public static final int machineCount = MyConfiguration2.machineCount;
	public static final int BATCH_SIZE = MyConfiguration2.BATCH_SIZE;
	public static final String SEPARATOR_KV = MyConfiguration.SEPARATOR_KV;
 
	public static final float C = (float) MyConfiguration.C;
	public static final int SAMPLE = MyConfiguration2.SAMPLE;
	public static final int STEP = MyConfiguration2.STEP; // the max length of a double path.
	public static final int CYCLE =MyConfiguration2.CYCLE;
	public static final byte pathLen = MyConfiguration2.pathLen;   // normally, shouldn't be larget than 127.
	public static final int TOPK = MyConfiguration.TOPK;
	public static final int M = MyConfiguration2.M;
	public static final IntWritable MINUS_ONE = new IntWritable(-1); // indicate null
	public static Random rand = new Random(); 

	private FixedCacheMap results = null; //new FixedHashMap(M * TOPK);

	private IntWritable randNeighbor() {
		RandOutEdges<IntWritable, NullWritable> edges = (RandOutEdges<IntWritable, NullWritable>) this.getEdges();
		Edge<IntWritable, NullWritable> next = edges.randEdge();
		if (next == null)
			return MINUS_ONE;
		else
			return next.getTargetVertexId();
	}
	public FixedCacheMap getResults() {
		// TODO Auto-generated method stub
		return results;
	}
	
	private void mySendMsg(ShortArrayWritable msg) {
		int pathCount = msg.getPathCount();
		if (pathCount == 1){
			this.sendMessage(randNeighbor(), msg);
			return;
		}
		int edgeNum = this.getNumEdges();;
		int avg = pathCount  / edgeNum;
		int remain = pathCount - avg * edgeNum;
		if (avg > 0){
			for (Edge<IntWritable, NullWritable> e : this.getEdges()){
				ShortArrayWritable newMsg = new ShortArrayWritable((byte)1, this.getId().get(), (byte)pathLen ,(short)avg);
				newMsg.setDegrees(Arrays.copyOf(msg.getDegrees(), pathLen));
				sendMessage(e.getTargetVertexId(), newMsg);
			}
		}

		for (int i = 0; i < remain; i++){
			ShortArrayWritable newMsg = new ShortArrayWritable((byte)1, this.getId().get(), (byte)pathLen ,(short)1);
			newMsg.setDegrees(Arrays.copyOf(msg.getDegrees(), pathLen));
			sendMessage(randNeighbor(), newMsg);
		}
	}

	@Override
	public void compute(Iterable<ShortArrayWritable> messages) throws IOException {
		int currentId = this.getId().get();
		if (this.getSuperstep() == 0 && currentId < stopV){
			results = new FixedCacheMap(M * TOPK);
		}
		if (getNumEdges() == 0)
			voteToHalt(); // single point.

		IntWritable low = this.getAggregatedValue("VID_LOWER");
		IntWritable up = this.getAggregatedValue("VID_UPPER");
		int beginV = low.get();
		int endV = up.get();
		if (this.getSuperstep() >= Math.ceil(stopV * 1.0 / BATCH_SIZE) * CYCLE){
			voteToHalt(); // all computation finished!
			return;
		}
			
		int step = (int) this.getSuperstep() % CYCLE;

		if (step == CYCLE - 1) {
			// deal with possible similarity increments.
			for (ShortArrayWritable msg : messages) {
				byte flag = msg.getFlag();
				if (flag == 2 ){   
					results.put(msg.getTarget(), msg.getSim() * msg.getPathCount());
				}
			}
		}else if ( step == CYCLE - 2){ 
			for (ShortArrayWritable msg : messages){
				byte flag = msg.getFlag();
				if ( flag ==  1 && msg.getSrc() < stopV && currentId != msg.getSrc()){     // just candidate vertices.
					float incre = (float)Math.pow(C, step / 2) * msg.getDegree(step / 2 - 1) / getNumEdges() / SAMPLE;
					sendMessage(new IntWritable(msg.getSrc()), new ShortArrayWritable((byte)2, currentId, incre, msg.getPathCount()));
				}
			}
		}else if (step == 0) {
			// only vertices in the  range [beginV, endV] are allowed to
			// "generate" paths.
			if (currentId < beginV ){
				voteToHalt();
				return;
			}
			
			// generate random walks!
			if ((currentId >= beginV) && (currentId <= endV) ){ // be careful about the range.
				int edgeNum = this.getNumEdges();;
				int avg = SAMPLE  / edgeNum;
				int remain = SAMPLE - avg * edgeNum;
				if (avg > 0){
					for (Edge<IntWritable, NullWritable> e : this.getEdges()){
						ShortArrayWritable msg = new ShortArrayWritable((byte)1, currentId, (byte)pathLen ,(short)avg);
						sendMessage(e.getTargetVertexId(), msg);
					}
				}
	
				for (int i = 0; i < remain; i++){
					ShortArrayWritable msg = new ShortArrayWritable((byte)1, currentId, (byte)pathLen ,(short)1);
					sendMessage(randNeighbor(), msg);
				}
			}
			
		} else {
			Iterator<ShortArrayWritable> msgs = messages.iterator();
			while (msgs.hasNext()) {
				ShortArrayWritable msg = msgs.next();
				// deal with possible similarity increments.
				if (msg.getFlag() == 2){
					results.put(msg.getTarget(), msg.getSim() * msg.getPathCount());
					continue;
				}
				
				// deal and pass the path msg.
				if (step <= STEP) {
					msg.setDegree(step - 1, (short)getNumEdges());
				}
				if (step % 2 == 0 && msg.getSrc() < stopV && currentId != msg.getSrc()) { // even path . store the increment similarity.
					float incre = (float)Math.pow(C, step / 2) * msg.getDegree(step / 2 - 1) / getNumEdges() / SAMPLE;
					sendMessage(new IntWritable(msg.getSrc()),new ShortArrayWritable((byte)2, currentId, incre , msg.getPathCount()));
				}
				// my sendMessage
				mySendMsg(msg);
			}
		}
	}


	public static void main(String[] args) throws IOException,
			InterruptedException, ClassNotFoundException {

		Path inputPath = new Path("hdfs://changping11:9000/user/luoxiongcai/simrank/input/"
				+ countFlag );
		Path outputPath = new Path("hdfs://changping11:9000/user/luoxiongcai/simrank/output/v_"
				+ countFlag + "_batch_normalCombine_noreuse");

		GiraphJob job = new GiraphJob(new Configuration(), "s_walk_1000_"
				+ countFlag+"_NormalCombine_noreuse");


		job.getConfiguration().set("hadoop.job.ugi", "luoxc,supergroup");
		job.getConfiguration().set("mapred.child.java.opts",
				"-Xms10g -Xmx10g -XX:+UseSerialGC");
		job.getConfiguration().set("mapred.job.map.memory.mb", "10000");
		job.getConfiguration().set("mapred.task.timeout", "6000000");
		job.getConfiguration().setInt("giraph.numComputeThreads", machineCount);
		File jarFile = EJob.createTempJar("bin");
		job.getConfiguration().set("mapred.jar", jarFile.getAbsolutePath());

		job.getConfiguration().setVertexClass(NormalCombineBatchSingleWalkVertex.class);
		

		// random outedge class.
		job.getConfiguration().setOutEdgesClass(RandOutEdges.class);
		// set mastercompute class
		job.getConfiguration().setMasterComputeClass(
				SingleWalkMasterCompute.class);

		job.getConfiguration().setEdgeInputFormatClass( // be careful to
														// synchronize with
														// .addVertex(edge)InputPath.
				IntNullReverseTextEdgeInputFormat.class);
		job.getConfiguration().setVertexOutputFormatClass(
				NoReuseBatchSimilaritytVertexOutputFormat.class);

		GiraphFileInputFormat.addEdgeInputPath(job.getConfiguration(),
				inputPath);

		FileOutputFormat.setOutputPath(job.getInternalJob(), outputPath);

		job.getConfiguration().setWorkerConfiguration(machineCount,
				machineCount, 100.0f);

		FileSystem fs = FileSystem.get(job.getConfiguration());
		if (fs.exists(outputPath)) {
			fs.delete(outputPath, true);
		}

		boolean isVerbose = true;
		job.run(isVerbose);

	}



}
