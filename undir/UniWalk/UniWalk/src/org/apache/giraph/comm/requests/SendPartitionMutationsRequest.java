/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.giraph.comm.requests;

import org.apache.giraph.comm.ServerData;
import org.apache.giraph.graph.VertexMutations;
import org.apache.giraph.metrics.GiraphMetrics;
import org.apache.giraph.metrics.MetricNames;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;
import org.apache.log4j.Logger;

import com.google.common.collect.Maps;
import com.yammer.metrics.core.Histogram;

import java.io.DataInput;
import java.io.DataOutput;
import java.io.IOException;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Send a collection of vertex mutations for a partition.
 *
 * @param <I> Vertex id
 * @param <V> Vertex data
 * @param <E> Edge data
 * @param <M> Message data
 */
@SuppressWarnings("rawtypes")
public class SendPartitionMutationsRequest<I extends WritableComparable,
    V extends Writable, E extends Writable, M extends Writable> extends
    WritableRequest<I, V, E, M> implements WorkerRequest<I, V, E, M> {
  /** Class logger */
  private static final Logger LOG =
      Logger.getLogger(SendPartitionMutationsRequest.class);
  /** Partition id */
  private int partitionId;
  /** Mutations sent for a partition */
  private Map<I, VertexMutations<I, V, E, M>> vertexIdMutations;

  /**
   * Constructor used for reflection only
   */
  public SendPartitionMutationsRequest() { }

  /**
   * Constructor used to send request.
   *
   * @param partitionId Partition to send the request to
   * @param vertexIdMutations Map of mutations to send
   */
  public SendPartitionMutationsRequest(
      int partitionId,
      Map<I, VertexMutations<I, V, E, M>> vertexIdMutations) {
    this.partitionId = partitionId;
    this.vertexIdMutations = vertexIdMutations;
  }

  @Override
  public void readFieldsRequest(DataInput input) throws IOException {
    partitionId = input.readInt();
    int vertexIdMutationsSize = input.readInt();
    vertexIdMutations = Maps.newHashMapWithExpectedSize(vertexIdMutationsSize);
    for (int i = 0; i < vertexIdMutationsSize; ++i) {
      I vertexId = getConf().createVertexId();
      vertexId.readFields(input);
      VertexMutations<I, V, E, M> vertexMutations =
          new VertexMutations<I, V, E, M>();
      vertexMutations.setConf(getConf());
      vertexMutations.readFields(input);
      if (vertexIdMutations.put(vertexId, vertexMutations) != null) {
        throw new IllegalStateException(
            "readFields: Already has vertex id " + vertexId);
      }
    }
  }

  @Override
  public void writeRequest(DataOutput output) throws IOException {
    output.writeInt(partitionId);
    output.writeInt(vertexIdMutations.size());
    for (Entry<I, VertexMutations<I, V, E, M>> entry :
        vertexIdMutations.entrySet()) {
      entry.getKey().write(output);
      entry.getValue().write(output);
    }
  }

  @Override
  public RequestType getType() {
    return RequestType.SEND_PARTITION_MUTATIONS_REQUEST;
  }

  @Override
  public void doRequest(ServerData<I, V, E, M> serverData) {
    ConcurrentHashMap<I, VertexMutations<I, V, E, M>> vertexMutations =
      serverData.getVertexMutations();
    Histogram verticesInMutationHist = GiraphMetrics.get().perSuperstep()
        .getUniformHistogram(MetricNames.VERTICES_IN_MUTATION_REQUEST);
    verticesInMutationHist.update(vertexMutations.size());
    for (Entry<I, VertexMutations<I, V, E, M>> entry :
        vertexIdMutations.entrySet()) {
      VertexMutations<I, V, E, M> mutations =
          vertexMutations.get(entry.getKey());
      if (mutations == null) {
        mutations = vertexMutations.putIfAbsent(
            entry.getKey(), entry.getValue());
        if (mutations == null) {
          continue;
        }
      }
      synchronized (mutations) {
        mutations.addVertexMutations(entry.getValue());
      }
    }
  }

  @Override
  public int getSerializedSize() {
    return WritableRequest.UNKNOWN_SIZE;
  }
}
