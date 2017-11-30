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

package org.apache.giraph.bsp;

import org.apache.giraph.comm.ServerData;
import org.apache.giraph.comm.WorkerClient;
import org.apache.giraph.graph.FinishedSuperstepStats;
import org.apache.giraph.graph.GraphState;
import org.apache.giraph.graph.GraphTaskManager;
import org.apache.giraph.graph.VertexEdgeCount;
import org.apache.giraph.io.superstep_output.SuperstepOutput;
import org.apache.giraph.master.MasterInfo;
import org.apache.giraph.partition.PartitionOwner;
import org.apache.giraph.partition.PartitionStats;
import org.apache.giraph.partition.PartitionStore;
import org.apache.giraph.worker.WorkerAggregatorHandler;
import org.apache.giraph.worker.WorkerContext;
import org.apache.giraph.worker.WorkerInfo;
import org.apache.giraph.worker.WorkerObserver;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;

import java.io.IOException;
import java.util.Collection;
import java.util.List;

/**
 * All workers should have access to this centralized service to
 * execute the following methods.
 *
 * @param <I> Vertex id
 * @param <V> Vertex value
 * @param <E> Edge value
 * @param <M> Message data
 */
@SuppressWarnings("rawtypes")
public interface CentralizedServiceWorker<I extends WritableComparable,
  V extends Writable, E extends Writable, M extends Writable>
  extends CentralizedService<I, V, E, M> {
  /**
   * Setup (must be called prior to any other function)
   *
   * @return Finished superstep stats for the input superstep
   */
  FinishedSuperstepStats setup();

  /**
   * Get the worker information
   *
   * @return Worker information
   */
  WorkerInfo getWorkerInfo();

  /**
   * Get the worker client (for instantiating WorkerClientRequestProcessor
   * instances.
   *
   * @return Worker client
   */
  WorkerClient<I, V, E, M> getWorkerClient();

  /**
   * Get the worker context.
   *
   * @return worker's WorkerContext
   */
  WorkerContext getWorkerContext();

  /**
   * Get the observers for this Worker.
   *
   * @return array of WorkerObservers.
   */
  WorkerObserver[] getWorkerObservers();

  /**
   * Get the partition store for this worker.
   * The partitions contain the vertices for
   * this worker and can be used to run compute() for the vertices or do
   * checkpointing.
   *
   * @return The partition store for this worker.
   */
  PartitionStore<I, V, E, M> getPartitionStore();

  /**
   *  Both the vertices and the messages need to be checkpointed in order
   *  for them to be used.  This is done after all messages have been
   *  delivered, but prior to a superstep starting.
   */
  void storeCheckpoint() throws IOException;

  /**
   * Load the vertices, edges, messages from the beginning of a superstep.
   * Will load the vertex partitions as designated by the master and set the
   * appropriate superstep.
   *
   * @param superstep which checkpoint to use
   * @return Graph-wide vertex and edge counts
   * @throws IOException
   */
  VertexEdgeCount loadCheckpoint(long superstep) throws IOException;

  /**
   * Take all steps prior to actually beginning the computation of a
   * superstep.
   *
   * @param graphState Current graph state
   * @return Collection of all the partition owners from the master for this
   *         superstep.
   */
  Collection<? extends PartitionOwner> startSuperstep(
      GraphState<I, V, E, M> graphState);

  /**
   * Worker is done with its portion of the superstep.  Report the
   * worker level statistics after the computation.
   *
   * @param graphState Current graph state
   * @param partitionStatsList All the partition stats for this worker
   * @return Stats of the superstep completion
   */
  FinishedSuperstepStats finishSuperstep(
      GraphState<I, V, E, M> graphState,
      List<PartitionStats> partitionStatsList);

  /**
   * Get the partition id that a vertex id would belong to.
   *
   * @param vertexId Vertex id
   * @return Partition id
   */
  Integer getPartitionId(I vertexId);

  /**
   * Whether a partition with given id exists on this worker.
   *
   * @param partitionId Partition id
   * @return True iff this worker has the specified partition
   */
  boolean hasPartition(Integer partitionId);

  /**
   * Every client will need to get a partition owner from a vertex id so that
   * they know which worker to sent the request to.
   *
   * @param vertexId Vertex index to look for
   * @return PartitionOnwer that should contain this vertex if it exists
   */
  PartitionOwner getVertexPartitionOwner(I vertexId);

  /**
   * Get all partition owners.
   *
   * @return Iterable through partition owners
   */
  Iterable<? extends PartitionOwner> getPartitionOwners();

  /**
   * If desired by the user, vertex partitions are redistributed among
   * workers according to the chosen WorkerGraphPartitioner.
   *
   * @param masterSetPartitionOwners Partition owner info passed from the
   *        master.
   */
  void exchangeVertexPartitions(
      Collection<? extends PartitionOwner> masterSetPartitionOwners);

  /**
   * Get master info
   *
   * @return Master info
   */
  MasterInfo getMasterInfo();

  /**
   * Get the GraphTaskManager that this service is using.  Vertices need to know
   * this.
   *
   * @return the GraphTaskManager instance for this compute node
   */
  GraphTaskManager<I, V, E, M> getGraphTaskManager();

  /**
   * Operations that will be called if there is a failure by a worker.
   */
  void failureCleanup();

  /**
   * Get server data
   *
   * @return Server data
   */
  ServerData<I, V, E, M> getServerData();

  /**
   * Get worker aggregator handler
   *
   * @return Worker aggregator handler
   */
  WorkerAggregatorHandler getAggregatorHandler();

  /**
   * Final preparation for superstep, called after startSuperstep and
   * potential loading from checkpoint, right before the computation started
   * TODO how to avoid this additional function
   */
  void prepareSuperstep();

  /**
   * Get the superstep output class
   *
   * @return SuperstepOutput
   */
  SuperstepOutput<I, V, E> getSuperstepOutput();

  /**
   * Clean up the service (no calls may be issued after this)
   *
   * @param finishedSuperstepStats Finished supestep stats
   * @throws IOException
   * @throws InterruptedException
   */
  void cleanup(FinishedSuperstepStats finishedSuperstepStats)
    throws IOException, InterruptedException;
}
