package giraph;

import org.apache.giraph.aggregators.BasicAggregator;
import org.apache.hadoop.io.IntWritable;
/**
 * here used to synchronize the value, which is matained by the mastercompute class.
 * @author luoxiongcai
 *
 */
public class BroadcastValueAggregator extends BasicAggregator<IntWritable>{

	@Override
	public void aggregate(IntWritable value) {
		getAggregatedValue().set(value.get());
	}

	@Override
	public IntWritable createInitialValue() {
		return new IntWritable(-1);
	}

	

}
