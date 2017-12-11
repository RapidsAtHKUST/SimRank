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

package org.apache.giraph.comm.netty;

import org.apache.giraph.bsp.CentralizedServiceWorker;
import org.apache.giraph.comm.ServerData;
import org.apache.giraph.comm.WorkerServer;
import org.apache.giraph.comm.messages.BasicMessageStore;
import org.apache.giraph.comm.messages.ByteArrayMessagesPerVertexStore;
import org.apache.giraph.comm.messages.DiskBackedMessageStore;
import org.apache.giraph.comm.messages.DiskBackedMessageStoreByPartition;
import org.apache.giraph.comm.messages.FlushableMessageStore;
import org.apache.giraph.comm.messages.MessageStoreByPartition;
import org.apache.giraph.comm.messages.MessageStoreFactory;
import org.apache.giraph.comm.messages.OneMessagePerVertexStore;
import org.apache.giraph.comm.messages.SequentialFileMessageStore;
import org.apache.giraph.comm.netty.handler.WorkerRequestServerHandler;
import org.apache.giraph.conf.ImmutableClassesGiraphConfiguration;
import org.apache.giraph.graph.GraphState;
import org.apache.giraph.graph.Vertex;
import org.apache.giraph.graph.VertexMutations;
import org.apache.giraph.graph.VertexResolver;
import org.apache.giraph.partition.Partition;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.log4j.Logger;

import com.google.common.collect.HashMultimap;
import com.google.common.collect.Iterables;
import com.google.common.collect.Multimap;

import java.net.InetSocketAddress;
import java.util.Collection;
import java.util.Map.Entry;

import static org.apache.giraph.conf.GiraphConstants.MAX_MESSAGES_IN_MEMORY;
import static org.apache.giraph.conf.GiraphConstants.USE_OUT_OF_CORE_MESSAGES;

/**
 * Netty worker server that implement {@link WorkerServer} and contains
 * the actual {@link ServerData}.
 *
 * @param <I> Vertex id
 * @param <V> Vertex data
 * @param <E> Edge data
 * @param <M> Message data
 */
@SuppressWarnings("rawtypes")
public class NettyWorkerServer<I extends WritableComparable,
    V extends Writable, E extends Writable, M extends Writable>
    implements WorkerServer<I, V, E, M> {
  /** Class logger */
  private static final Logger LOG =
    Logger.getLogger(NettyWorkerServer.class);
  /** Hadoop configuration */
  private final ImmutableClassesGiraphConfiguration<I, V, E, M> conf;
  /** Service worker */
  private final CentralizedServiceWorker<I, V, E, M> service;
  /** Netty server that does that actual I/O */
  private final NettyServer nettyServer;
  /** Server data storage */
  private final ServerData<I, V, E, M> serverData;

  /**
   * Constructor to start the server.
   *
   * @param conf Configuration
   * @param service Service to get partition mappings
   * @param context Mapper context
   */
  public NettyWorkerServer(ImmutableClassesGiraphConfiguration<I, V, E, M> conf,
      CentralizedServiceWorker<I, V, E, M> service,
      Mapper<?, ?, ?, ?>.Context context) {
    this.conf = conf;
    this.service = service;

    serverData =
        new ServerData<I, V, E, M>(service, conf, createMessageStoreFactory(),
            context);

    nettyServer = new NettyServer(conf,
        new WorkerRequestServerHandler.Factory<I, V, E, M>(serverData),
        service.getWorkerInfo(), context);
    nettyServer.start();
  }

  /**
   * Decide which message store should be used for current application,
   * and create the factory for that store
   *
   * @return Message store factory
   */
  private MessageStoreFactory<I, M, MessageStoreByPartition<I, M>>
  createMessageStoreFactory() {
    boolean useOutOfCoreMessaging = USE_OUT_OF_CORE_MESSAGES.get(conf);
    if (!useOutOfCoreMessaging) {
      if (conf.useCombiner()) {
        if (LOG.isInfoEnabled()) {
          LOG.info("createMessageStoreFactory: " +
              "Using OneMessagePerVertexStore since combiner enabled");
        }
        return OneMessagePerVertexStore.newFactory(service, conf);
      } else {
        if (LOG.isInfoEnabled()) {
          LOG.info("createMessageStoreFactory: " +
              "Using ByteArrayMessagesPerVertexStore " +
              "since there is no combiner");
        }
        return ByteArrayMessagesPerVertexStore.newFactory(service, conf);
      }
    } else {
      int maxMessagesInMemory = MAX_MESSAGES_IN_MEMORY.get(conf);
      if (LOG.isInfoEnabled()) {
        LOG.info("createMessageStoreFactory: Using DiskBackedMessageStore, " +
            "maxMessagesInMemory = " + maxMessagesInMemory);
      }
      MessageStoreFactory<I, M, BasicMessageStore<I, M>> fileStoreFactory =
          SequentialFileMessageStore.newFactory(conf);
      MessageStoreFactory<I, M, FlushableMessageStore<I, M>>
          partitionStoreFactory =
          DiskBackedMessageStore.newFactory(conf, fileStoreFactory);
      return DiskBackedMessageStoreByPartition.newFactory(service,
          maxMessagesInMemory, partitionStoreFactory);
    }
  }

  @Override
  public InetSocketAddress getMyAddress() {
    return nettyServer.getMyAddress();
  }

  @Override
  public void prepareSuperstep(GraphState<I, V, E, M> graphState) {
    serverData.prepareSuperstep();
    resolveMutations(graphState);
  }

  /**
   * Resolve mutation requests.
   *
   * @param graphState Graph state
   */
  private void resolveMutations(GraphState<I, V, E, M> graphState) {
    Multimap<Integer, I> resolveVertexIndices = HashMultimap.create(
        service.getPartitionStore().getNumPartitions(), 100);
      // Add any mutated vertex indices to be resolved
    for (Entry<I, VertexMutations<I, V, E, M>> e :
        serverData.getVertexMutations().entrySet()) {
      I vertexId = e.getKey();
      Integer partitionId = service.getPartitionId(vertexId);
      if (!resolveVertexIndices.put(partitionId, vertexId)) {
        throw new IllegalStateException(
            "resolveMutations: Already has missing vertex on this " +
                "worker for " + vertexId);
      }
    }
    // Keep track of the vertices which are not here but have received messages
    for (Integer partitionId : service.getPartitionStore().getPartitionIds()) {
      Iterable<I> destinations = serverData.getCurrentMessageStore().
          getPartitionDestinationVertices(partitionId);
      if (!Iterables.isEmpty(destinations)) {
        Partition<I, V, E, M> partition =
            service.getPartitionStore().getPartition(partitionId);
        for (I vertexId : destinations) {
          if (partition.getVertex(vertexId) == null) {
            if (!resolveVertexIndices.put(partitionId, vertexId)) {
              throw new IllegalStateException(
                  "resolveMutations: Already has missing vertex on this " +
                      "worker for " + vertexId);
            }
          }
        }
        service.getPartitionStore().putPartition(partition);
      }
    }
    // Resolve all graph mutations
    VertexResolver<I, V, E, M> vertexResolver =
        conf.createVertexResolver(graphState);
    for (Entry<Integer, Collection<I>> e :
        resolveVertexIndices.asMap().entrySet()) {
      Partition<I, V, E, M> partition =
          service.getPartitionStore().getPartition(e.getKey());
      for (I vertexIndex : e.getValue()) {
        Vertex<I, V, E, M> originalVertex =
            partition.getVertex(vertexIndex);

        VertexMutations<I, V, E, M> mutations = null;
        VertexMutations<I, V, E, M> vertexMutations =
            serverData.getVertexMutations().get(vertexIndex);
        if (vertexMutations != null) {
          synchronized (vertexMutations) {
            mutations = vertexMutations.copy();
          }
          serverData.getVertexMutations().remove(vertexIndex);
        }
        Vertex<I, V, E, M> vertex = vertexResolver.resolve(
            vertexIndex, originalVertex, mutations,
            serverData.getCurrentMessageStore().
                hasMessagesForVertex(vertexIndex));
        graphState.getContext().progress();

        if (LOG.isDebugEnabled()) {
          LOG.debug("resolveMutations: Resolved vertex index " +
              vertexIndex + " with original vertex " +
              originalVertex + ", returned vertex " + vertex +
              " on superstep " + service.getSuperstep() +
              " with mutations " +
              mutations);
        }
        if (vertex != null) {
          partition.putVertex(vertex);
        } else if (originalVertex != null) {
          partition.removeVertex(originalVertex.getId());
        }
      }
      service.getPartitionStore().putPartition(partition);
    }
    if (!serverData.getVertexMutations().isEmpty()) {
      throw new IllegalStateException("resolveMutations: Illegally " +
          "still has " + serverData.getVertexMutations().size() +
          " mutations left.");
    }
  }

  @Override
  public ServerData<I, V, E, M> getServerData() {
    return serverData;
  }

  @Override
  public void close() {
    nettyServer.stop();
  }
}
