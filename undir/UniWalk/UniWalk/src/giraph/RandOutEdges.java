
package giraph;

import com.google.common.collect.Lists;

import org.apache.giraph.edge.ConfigurableOutEdges;
import org.apache.giraph.edge.Edge;
import org.apache.giraph.edge.MutableEdge;
import org.apache.giraph.edge.MutableOutEdges;
import org.apache.giraph.utils.WritableUtils;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Random;

/**
 * {@link OutEdges} implementation backed by an {@link ArrayList}.
 * Parallel edges are allowed.
 * compared to ArrayListEdges, add a new method randNeighbor();
 * 
 *
 * @param <I> Vertex id
 * @param <E> Edge value
 */
public class RandOutEdges<I extends WritableComparable, E extends Writable>
    extends ConfigurableOutEdges<I, E>
    implements MutableOutEdges<I, E> {
  /** List of edges. */
  private ArrayList<Edge<I, E>> edgeList;
  private static final Random rand = new Random();
 
  @Override
  public void initialize(Iterable<Edge<I, E>> edges) {
    // If the iterable is actually an instance of ArrayList,
    // we simply copy the reference.
    // Otherwise we have to add every edge.
    if (edges instanceof ArrayList) {
      edgeList = (ArrayList<Edge<I, E>>) edges;
    } else {
      edgeList = Lists.newArrayList(edges);
    }
  }

  @Override
  public void initialize(int capacity) {
    edgeList = Lists.newArrayListWithCapacity(capacity);
  }

  @Override
  public void initialize() {
    edgeList = Lists.newArrayList();
  }

  @Override
  public void add(Edge<I, E> edge) {
    edgeList.add(edge);
  }

  @Override
  public void remove(I targetVertexId) {
    for (Iterator<Edge<I, E>> edges = edgeList.iterator(); edges.hasNext();) {
      Edge<I, E> edge = edges.next();
      if (edge.getTargetVertexId().equals(targetVertexId)) {
        edges.remove();
      }
    }
  }

  @Override
  public int size() {
    return edgeList.size();
  }

  @Override
  public final Iterator<Edge<I, E>> iterator() {
    return edgeList.iterator();
  }

  @Override
  @SuppressWarnings("unchecked")
  public Iterator<MutableEdge<I, E>> mutableIterator() {
    // The downcast is fine because all concrete Edge implementations are
    // mutable, but we only expose the mutation functionality when appropriate.
    return (Iterator) iterator();
  }

  @Override
  public void write(DataOutput out) throws IOException {
    out.writeInt(edgeList.size());
    for (Edge<I, E> edge : edgeList) {
      edge.getTargetVertexId().write(out);
      edge.getValue().write(out);
    }
  }

  @Override
  public void readFields(DataInput in) throws IOException {
    int numEdges = in.readInt();
    initialize(numEdges);
    for (int i = 0; i < numEdges; ++i) {
      Edge<I, E> edge = getConf().createEdge();
      WritableUtils.readEdge(in, edge);
      edgeList.add(edge);
    }
  }
  
  
  public Edge<I,E> randEdge(){
	  if (size() == 0) return null;
	  else return  edgeList.get(rand.nextInt(size()));
  }
}
