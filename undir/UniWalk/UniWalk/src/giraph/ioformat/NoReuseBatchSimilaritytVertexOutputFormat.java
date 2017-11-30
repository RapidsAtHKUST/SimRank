package giraph.ioformat;

import org.apache.giraph.graph.Vertex;
import org.apache.giraph.io.formats.AdjacencyListTextVertexInputFormat;
import org.apache.giraph.io.formats.TextVertexOutputFormat;
import org.apache.hadoop.io.NullWritable;
import org.apache.hadoop.io.Text;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.mapreduce.TaskAttemptContext;

import conf.MyConfiguration;
import conf.MyConfiguration2;
import giraph.NormalCombineBatchSingleWalkVertex;
import giraph.NormalCombineBatchSingleWalkVertexReuse;

import java.io.IOException;

import lxctools.FixedCacheMap;
import lxctools.Pair;

/**
 * OutputFormat to write out the graph nodes as text, value-separated (by tabs,
 * by default). With the default delimiter, a vertex is written out as:
 * 
 * srcId dst
 * 
 * @param <I>
 *            Vertex index value
 * @param <V>
 *            Vertex value
 * @param <E>
 *            Edge value
 */
@SuppressWarnings("rawtypes")
public class NoReuseBatchSimilaritytVertexOutputFormat<I extends WritableComparable, V extends Writable, E extends Writable>
		extends TextVertexOutputFormat<I, V, E> {
	/** Split delimiter */
	public static final String LINE_TOKENIZE_VALUE = "output.delimiter";
	/** Default split delimiter */
	public static final String LINE_TOKENIZE_VALUE_DEFAULT = AdjacencyListTextVertexInputFormat.LINE_TOKENIZE_VALUE_DEFAULT;

	@Override
	public AdjacencyListTextVertexWriter createVertexWriter(
			TaskAttemptContext context) {
		return new AdjacencyListTextVertexWriter();
	}

	/**
	 * Vertex writer associated with {@link SimilaritytVertexOutputFormat}.
	 */
	protected class AdjacencyListTextVertexWriter extends
			TextVertexWriterToEachLine {
		/** Cached split delimeter */
		private String delimiter;

		@Override
		public void initialize(TaskAttemptContext context) throws IOException,
				InterruptedException {
			super.initialize(context);
			delimiter = getConf().get(LINE_TOKENIZE_VALUE,
					LINE_TOKENIZE_VALUE_DEFAULT);
		}

		@Override
		public Text convertVertexToLine(Vertex<I, V, E, ?> vertex)
				throws IOException {
			
			NormalCombineBatchSingleWalkVertex swv = (NormalCombineBatchSingleWalkVertex) vertex;
			if (swv.getId().get() >= MyConfiguration2.stopV) 
				return null;
			StringBuffer sb = new StringBuffer(swv.getId().toString());

			FixedCacheMap results = swv.getResults();

			int i = 0;
			int size = results.size();

			for (Pair<Integer, Float> simPair : results) {
				if (i < size - MyConfiguration.TOPK) {
					i++;
					continue;
				}

				sb.append("\t"
						+ simPair.getKey()
						+ MyConfiguration.SEPARATOR_KV
						+ String.format("%.6f", simPair.getValue()));
			}
			return new Text(sb.toString());
		}
	}

}
