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
import giraph.ioformat.BatchSimilaritytVertexOutputFormat;
import giraph.writables.CombineMixMsgWritable;

import java.io.File;
import java.io.IOException;
import java.util.Iterator;
import java.util.Random;

import lxctools.FixedCacheMap;

/**
 * 
 * @author luoxiongcai
 *
 */
public class NormalCombineBatchSingleWalkVertexReuse extends
		Vertex<IntWritable, NullWritable, NullWritable, CombineMixMsgWritable> {
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
	public static final int pathLen = MyConfiguration2.pathLen;   // normally, shouldn't be larget than 127.
	public static final int targetLen = MyConfiguration2.targetLen;
	public static final int TOPK = MyConfiguration.TOPK;
	public static final int M = MyConfiguration2.M;
	public static final IntWritable MINUS_ONE = new IntWritable(-1); // indicate null
	public Random rand = new Random(); 
	private FixedCacheMap results = null; 
	public int sampleCount = 0;
	
	
	public FixedCacheMap getResults(){
		return results;
	}

	private IntWritable randNeighbor() {
		RandOutEdges<IntWritable, NullWritable> edges = (RandOutEdges<IntWritable, NullWritable>) this.getEdges();
		Edge<IntWritable, NullWritable> next = edges.randEdge();
		if (next == null)
			return MINUS_ONE;
		else
			return next.getTargetVertexId();
	}
	
	private void incrementSims(CombineMixMsgWritable msg){
		int[] targets = msg.getTargets();
		float[] sims = msg.getSims();
		short pathCount = msg.getPathCount();
		for (int i = 0; i < targetLen; i++){
			if (targets[i] != this.getId().get())
				results.put(targets[i], sims[i] * pathCount);
		}
		sampleCount += pathCount;
	}
	
	private void computeAndSendSim(CombineMixMsgWritable msg){
		byte srcp = msg.getSrcp();
		int[] path = msg.getPath();
		// stopv requirements!!!
		// to reduce unnecessary msgs.
		if (path[srcp] >= stopV) return;
		short[] degrees = msg.getDegrees();
		short pathCount = msg.getPathCount();
		
		int[] targets = new int[targetLen];
		float[] sims = new float[targetLen];
		for (int i = 1; i <= targetLen; i++ ){
			targets[i - 1] = path[(srcp + i * 2) % pathLen];
			sims[i - 1] = (float) (Math.pow(C, i) * degrees[(srcp + i) % pathLen] / degrees[(srcp + i * 2) % pathLen]);
		}
		CombineMixMsgWritable simMsg = new CombineMixMsgWritable((byte) 2 , (byte)targetLen, (short)pathCount);
		simMsg.setTargets(targets);
		simMsg.setSims(sims);
		sendMessage(new IntWritable(path[srcp]),simMsg);
	}
	
	// send msg to neighbours.
	private void mySendMsg(CombineMixMsgWritable msg){
		short pathCount = msg.getPathCount();
		if (pathCount == 1){
			this.sendMessage(randNeighbor(), msg);
			return;
		}
		int edgeNum = this.getNumEdges();
		int avg = pathCount / edgeNum;
		int remain = pathCount - avg * edgeNum;
		if (avg > 0){
			for (Edge<IntWritable, NullWritable> e : this.getEdges()){
				CombineMixMsgWritable toMsg = new CombineMixMsgWritable(msg);
				toMsg.setPathCount((short)avg);
				sendMessage(new IntWritable(e.getTargetVertexId().get()), toMsg);
			}	
		}
		
		for (int i = 0; i < remain; i++){
			CombineMixMsgWritable toMsg = new CombineMixMsgWritable(msg);
			toMsg.setPathCount((short)1);
			sendMessage(randNeighbor(), toMsg);
		}
	}

	@Override
	public void compute(Iterable<CombineMixMsgWritable> messages) throws IOException {
		// initial storage for results.
		int currentId = this.getId().get();
		if (this.getSuperstep() == 0 && currentId < stopV){
			results = new FixedCacheMap(M * TOPK);
		}
		
		if (getNumEdges() == 0){// single point.
			voteToHalt(); 
			return;
		}
		
		
		IntWritable low = this.getAggregatedValue("VID_LOWER");
		IntWritable up = this.getAggregatedValue("VID_UPPER");
		int beginV = low.get();
		int endV = up.get();
		if (this.getSuperstep() >= Math.ceil(stopV * 1.0 / BATCH_SIZE) * CYCLE){
			voteToHalt(); 
			return;
		}
			
		int step = (int) this.getSuperstep() % CYCLE;
		if (step == CYCLE - 1){
			// deal with possible similarity increments or sampleCount increment.Because last superstep of last batch may send similarities.
			for (CombineMixMsgWritable msg : messages) {
				byte flag = msg.getFlag();
				if (flag == 2){
					incrementSims(msg);
				} 
			}
		}else if (step == 0) {
			if (currentId < beginV ){ // dealt vertex.
				voteToHalt();
				return;
			}
			
			// generate random walks!
			if ((currentId >= beginV) && (currentId <= endV) && sampleCount < SAMPLE){ // be careful about the range.
				int edgeNum = this.getNumEdges();
				int needSample = SAMPLE - sampleCount;
				int avg = needSample  / edgeNum;
				int remain = needSample - avg * edgeNum;
				
				if (avg > 0){
					for (Edge<IntWritable, NullWritable> e : this.getEdges()){
						CombineMixMsgWritable msg = new CombineMixMsgWritable((byte)1, (byte)pathLen, (short)avg);
						msg.setDirectPath(0, currentId,(byte) this.getNumEdges());
						sendMessage(e.getTargetVertexId(), msg);
					}
				}

				for (int i = 0; i < remain; i++){
					CombineMixMsgWritable msg = new CombineMixMsgWritable((byte)1, (byte)pathLen, (short)1);
					msg.setDirectPath(0, currentId,(byte) this.getNumEdges());
					sendMessage(randNeighbor(), msg);
				}
			}
			
		} else if (step <= 2 * STEP){ // just random walk.
			Iterator<CombineMixMsgWritable> msgs = messages.iterator();
			while (msgs.hasNext()) {
				CombineMixMsgWritable msg = msgs.next();
				msg.setDirectPath(step, currentId, (byte)getNumEdges());
				mySendMsg(msg);
				
				if (step == 2 *STEP && msg.getSource() >= beginV){ // be careful here, msgs to small ids are unnecessary.s
																   // since the results have been set null.
					computeAndSendSim(msg);
				}
			}
		} else {  // random walk and compute sim.
			Iterator<CombineMixMsgWritable> msgs = messages.iterator();
			while (msgs.hasNext()) {
				CombineMixMsgWritable msg = msgs.next();
				// deal with possible similarity increments.
				byte flag = msg.getFlag();
				if (flag == 2){
					incrementSims(msg);
				} else if (flag == 1){
					msg.appendCircularPath(currentId, (byte)this.getNumEdges());
					if (step < CYCLE - 2)
						mySendMsg(msg);
					if (msg.getSource() >= beginV){
						computeAndSendSim(msg);
					}
					
				}
			}
		}
	}

	public static void main(String[] args) throws IOException,
			InterruptedException, ClassNotFoundException {

		Path inputPath = new Path("hdfs://changping11:9000/user/luoxiongcai/simrank/input/"
				+ countFlag );
		Path outputPath = new Path("hdfs://changping11:9000/user/luoxiongcai/simrank/output/v_"
				+ countFlag + "_normalcombine_reuse"+machineCount);

		GiraphJob job = new GiraphJob(new Configuration(), "s_walk_batch_"
				+ countFlag+"_normalcombine_reuse_"+machineCount+"_"+MyConfiguration2.stopV);

		job.getConfiguration().set("hadoop.job.ugi", "hadoop,supergroup");
		job.getConfiguration().set("mapred.child.java.opts",
				"-Xms10g -Xmx10g -XX:+UseSerialGC");
		job.getConfiguration().set("mapred.job.map.memory.mb", "10000");
		job.getConfiguration().set("mapred.task.timeout", "60000000");
		job.getConfiguration().setInt("giraph.numComputeThreads", machineCount);
		File jarFile = EJob.createTempJar("bin");
		job.getConfiguration().set("mapred.jar", jarFile.getAbsolutePath());

		job.getConfiguration().setVertexClass(NormalCombineBatchSingleWalkVertexReuse.class);
		//set workcontext.
//		job.getConfiguration().setWorkerContextClass(SingleWalkWorkContext.class);

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
				BatchSimilaritytVertexOutputFormat.class);

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
