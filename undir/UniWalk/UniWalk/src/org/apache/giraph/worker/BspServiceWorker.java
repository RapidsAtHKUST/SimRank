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

package org.apache.giraph.worker;


import org.apache.giraph.bsp.ApplicationState;
import org.apache.giraph.bsp.CentralizedServiceWorker;
import org.apache.giraph.comm.ServerData;
import org.apache.giraph.comm.WorkerClient;
import org.apache.giraph.comm.WorkerClientRequestProcessor;
import org.apache.giraph.comm.WorkerServer;
import org.apache.giraph.comm.aggregators.WorkerAggregatorRequestProcessor;
import org.apache.giraph.comm.netty.NettyWorkerAggregatorRequestProcessor;
import org.apache.giraph.comm.netty.NettyWorkerClient;
import org.apache.giraph.comm.netty.NettyWorkerClientRequestProcessor;
import org.apache.giraph.comm.netty.NettyWorkerServer;
import org.apache.giraph.conf.GiraphConstants;
import org.apache.giraph.conf.ImmutableClassesGiraphConfiguration;
import org.apache.giraph.graph.GraphState;
import org.apache.giraph.bsp.BspService;
import org.apache.giraph.graph.GraphTaskManager;
import org.apache.giraph.graph.VertexEdgeCount;
import org.apache.giraph.graph.InputSplitPaths;
import org.apache.giraph.graph.InputSplitEvents;
import org.apache.giraph.graph.FinishedSuperstepStats;
import org.apache.giraph.graph.AddressesAndPartitionsWritable;
import org.apache.giraph.graph.GlobalStats;
import org.apache.giraph.io.superstep_output.SuperstepOutput;
import org.apache.giraph.utils.CallableFactory;
import org.apache.giraph.utils.JMapHistoDumper;
import org.apache.giraph.graph.Vertex;
import org.apache.giraph.io.VertexOutputFormat;
import org.apache.giraph.io.VertexWriter;
import org.apache.giraph.partition.Partition;
import org.apache.giraph.partition.PartitionExchange;
import org.apache.giraph.partition.PartitionOwner;
import org.apache.giraph.partition.PartitionStats;
import org.apache.giraph.partition.PartitionStore;
import org.apache.giraph.partition.WorkerGraphPartitioner;
import org.apache.giraph.master.MasterInfo;
import org.apache.giraph.metrics.GiraphMetrics;
import org.apache.giraph.metrics.GiraphTimer;
import org.apache.giraph.metrics.GiraphTimerContext;
import org.apache.giraph.metrics.ResetSuperstepMetricsObserver;
import org.apache.giraph.metrics.SuperstepMetricsRegistry;
import org.apache.giraph.metrics.WorkerSuperstepMetrics;
import org.apache.giraph.utils.LoggerUtils;
import org.apache.giraph.utils.MemoryUtils;
import org.apache.giraph.utils.ProgressableUtils;
import org.apache.giraph.utils.WritableUtils;
import org.apache.giraph.zk.BspEvent;
import org.apache.giraph.zk.PredicateLock;
import org.apache.hadoop.fs.FSDataOutputStream;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.hadoop.mapreduce.OutputCommitter;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.WatchedEvent;
import org.apache.zookeeper.Watcher.Event.EventType;
import org.apache.zookeeper.ZooDefs.Ids;
import org.apache.zookeeper.data.Stat;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import com.google.common.collect.Lists;
import net.iharder.Base64;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutput;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;

/**
 * ZooKeeper-based implementation of {@link CentralizedServiceWorker}.
 *
 * @param <I> Vertex id
 * @param <V> Vertex data
 * @param <E> Edge data
 * @param <M> Message data
 */
@SuppressWarnings("rawtypes")
public class BspServiceWorker<I extends WritableComparable,
    V extends Writable, E extends Writable, M extends Writable>
    extends BspService<I, V, E, M>
    implements CentralizedServiceWorker<I, V, E, M>,
    ResetSuperstepMetricsObserver {
  /** Name of gauge for time spent waiting on other workers */
  public static final String TIMER_WAIT_REQUESTS = "wait-requests-us";
  /** Class logger */
  private static final Logger LOG = Logger.getLogger(BspServiceWorker.class);
  /** My process health znode */
  private String myHealthZnode;
  /** Worker info */
  private final WorkerInfo workerInfo;
  /** Worker graph partitioner */
  private final WorkerGraphPartitioner<I, V, E, M> workerGraphPartitioner;

  /** IPC Client */
  private final WorkerClient<I, V, E, M> workerClient;
  /** IPC Server */
  private final WorkerServer<I, V, E, M> workerServer;
  /** Request processor for aggregator requests */
  private final WorkerAggregatorRequestProcessor
  workerAggregatorRequestProcessor;
  /** Master info */
  private MasterInfo masterInfo = new MasterInfo();
  /** List of workers */
  private List<WorkerInfo> workerInfoList = Lists.newArrayList();
  /** Have the partition exchange children (workers) changed? */
  private final BspEvent partitionExchangeChildrenChanged;

  /** Worker Context */
  private final WorkerContext workerContext;

  /** Handler for aggregators */
  private final WorkerAggregatorHandler aggregatorHandler;

  /** Superstep output */
  private SuperstepOutput<I, V, E> superstepOutput;

  /** array of observers to call back to */
  private final WorkerObserver[] observers;

  // Per-Superstep Metrics
  /** Timer for WorkerContext#postSuperstep */
  private GiraphTimer wcPostSuperstepTimer;
  /** Time spent waiting on requests to finish */
  private GiraphTimer waitRequestsTimer;

  /**
   * Constructor for setting up the worker.
   *
   * @param serverPortList ZooKeeper server port list
   * @param sessionMsecTimeout Msecs to timeout connecting to ZooKeeper
   * @param context Mapper context
   * @param graphTaskManager GraphTaskManager for this compute node
   * @throws IOException
   * @throws InterruptedException
   */
  public BspServiceWorker(
    String serverPortList,
    int sessionMsecTimeout,
    Mapper<?, ?, ?, ?>.Context context,
    GraphTaskManager<I, V, E, M> graphTaskManager)
    throws IOException, InterruptedException {
    super(serverPortList, sessionMsecTimeout, context, graphTaskManager);
    ImmutableClassesGiraphConfiguration<I, V, E, M> conf = getConfiguration();
    partitionExchangeChildrenChanged = new PredicateLock(context);
    registerBspEvent(partitionExchangeChildrenChanged);
    workerGraphPartitioner =
        getGraphPartitionerFactory().createWorkerGraphPartitioner();
    workerInfo = new WorkerInfo();
    workerServer = new NettyWorkerServer<I, V, E, M>(conf, this, context);
    workerInfo.setInetSocketAddress(workerServer.getMyAddress());
    workerInfo.setTaskId(getTaskPartition());
    workerClient = new NettyWorkerClient<I, V, E, M>(context, conf, this);

    workerAggregatorRequestProcessor =
        new NettyWorkerAggregatorRequestProcessor(getContext(), conf, this);

    workerContext = conf.createWorkerContext(null);

    aggregatorHandler = new WorkerAggregatorHandler(this, conf, context);

    superstepOutput = conf.createSuperstepOutput(context);

    if (conf.isJMapHistogramDumpEnabled()) {
      conf.addWorkerObserverClass(JMapHistoDumper.class);
    }
    observers = conf.createWorkerObservers();

    GiraphMetrics.get().addSuperstepResetObserver(this);
  }

  @Override
  public void newSuperstep(SuperstepMetricsRegistry superstepMetrics) {
    waitRequestsTimer = new GiraphTimer(superstepMetrics,
        TIMER_WAIT_REQUESTS, TimeUnit.MICROSECONDS);
    wcPostSuperstepTimer = new GiraphTimer(superstepMetrics,
        "worker-context-post-superstep", TimeUnit.MICROSECONDS);
  }

  @Override
  public WorkerContext getWorkerContext() {
    return workerContext;
  }

  @Override
  public WorkerObserver[] getWorkerObservers() {
    return observers;
  }

  @Override
  public WorkerClient<I, V, E, M> getWorkerClient() {
    return workerClient;
  }

  /**
   * Intended to check the health of the node.  For instance, can it ssh,
   * dmesg, etc. For now, does nothing.
   * TODO: Make this check configurable by the user (i.e. search dmesg for
   * problems).
   *
   * @return True if healthy (always in this case).
   */
  public boolean isHealthy() {
    return true;
  }

  /**
   * Load the vertices/edges from input slits. Do this until all the
   * InputSplits have been processed.
   * All workers will try to do as many InputSplits as they can.  The master
   * will monitor progress and stop this once all the InputSplits have been
   * loaded and check-pointed.  Keep track of the last input split path to
   * ensure the input split cache is flushed prior to marking the last input
   * split complete.
   *
   * Use one or more threads to do the loading.
   *
   * @param inputSplitPathList List of input split paths
   * @param inputSplitsCallableFactory Factory for {@link InputSplitsCallable}s
   * @return Statistics of the vertices and edges loaded
   * @throws InterruptedException
   * @throws KeeperException
   */
  private VertexEdgeCount loadInputSplits(
      List<String> inputSplitPathList,
      CallableFactory<VertexEdgeCount> inputSplitsCallableFactory)
    throws KeeperException, InterruptedException {
    VertexEdgeCount vertexEdgeCount = new VertexEdgeCount();
    // Determine how many threads to use based on the number of input splits
    int maxInputSplitThreads = (inputSplitPathList.size() - 1) /
        getConfiguration().getMaxWorkers() + 1;
    int numThreads = Math.min(getConfiguration().getNumInputSplitsThreads(),
        maxInputSplitThreads);
    if (LOG.isInfoEnabled()) {
      LOG.info("loadInputSplits: Using " + numThreads + " thread(s), " +
          "originally " + getConfiguration().getNumInputSplitsThreads() +
          " threads(s) for " + inputSplitPathList.size() + " total splits.");
    }

    List<VertexEdgeCount> results =
        ProgressableUtils.getResultsWithNCallables(inputSplitsCallableFactory,
            numThreads, "load-%d", getContext());
    for (VertexEdgeCount result : results) {
      vertexEdgeCount = vertexEdgeCount.incrVertexEdgeCount(result);
    }

    workerClient.waitAllRequests();
    return vertexEdgeCount;
  }


  /**
   * Load the vertices from the user-defined {@link VertexReader}
   *
   * @return Count of vertices and edges loaded
   */
  private VertexEdgeCount loadVertices() throws KeeperException,
      InterruptedException {
    List<String> inputSplitPathList =
        getZkExt().getChildrenExt(vertexInputSplitsPaths.getPath(),
            false, false, true);

    GraphState<I, V, E, M> graphState = new GraphState<I, V, E, M>(
        INPUT_SUPERSTEP, 0, 0, getContext(), getGraphTaskManager(),
        null, null);

    InputSplitPathOrganizer splitOrganizer =
        new InputSplitPathOrganizer(getZkExt(),
            inputSplitPathList, getWorkerInfo().getHostname(),
            getConfiguration().useInputSplitLocality());
    InputSplitsHandler splitsHandler = new InputSplitsHandler(
        splitOrganizer,
        getZkExt(),
        getContext(),
        BspService.VERTEX_INPUT_SPLIT_RESERVED_NODE,
        BspService.VERTEX_INPUT_SPLIT_FINISHED_NODE);

    VertexInputSplitsCallableFactory<I, V, E, M> inputSplitsCallableFactory =
        new VertexInputSplitsCallableFactory<I, V, E, M>(
            getContext(),
            graphState,
            getConfiguration(),
            this,
            splitsHandler,
            getZkExt());

    return loadInputSplits(inputSplitPathList, inputSplitsCallableFactory);
  }

  /**
   * Load the edges from the user-defined {@link EdgeReader}.
   *
   * @return Number of edges loaded
   */
  private long loadEdges() throws KeeperException, InterruptedException {
    List<String> inputSplitPathList =
        getZkExt().getChildrenExt(edgeInputSplitsPaths.getPath(),
            false, false, true);

    GraphState<I, V, E, M> graphState = new GraphState<I, V, E, M>(
        INPUT_SUPERSTEP, 0, 0, getContext(), getGraphTaskManager(),
        null, null);

    InputSplitPathOrganizer splitOrganizer =
        new InputSplitPathOrganizer(getZkExt(),
            inputSplitPathList, getWorkerInfo().getHostname(),
            getConfiguration().useInputSplitLocality());
    InputSplitsHandler splitsHandler = new InputSplitsHandler(
        splitOrganizer,
        getZkExt(),
        getContext(),
        BspService.EDGE_INPUT_SPLIT_RESERVED_NODE,
        BspService.EDGE_INPUT_SPLIT_FINISHED_NODE);

    EdgeInputSplitsCallableFactory<I, V, E, M> inputSplitsCallableFactory =
        new EdgeInputSplitsCallableFactory<I, V, E, M>(
            getContext(),
            graphState,
            getConfiguration(),
            this,
            splitsHandler,
            getZkExt());

    return loadInputSplits(inputSplitPathList, inputSplitsCallableFactory).
        getEdgeCount();
  }

  @Override
  public MasterInfo getMasterInfo() {
    return masterInfo;
  }

  @Override
  public List<WorkerInfo> getWorkerInfoList() {
    return workerInfoList;
  }

  /**
   * Ensure the input splits are ready for processing
   *
   * @param inputSplitPaths Input split paths
   * @param inputSplitEvents Input split events
   */
  private void ensureInputSplitsReady(InputSplitPaths inputSplitPaths,
                                      InputSplitEvents inputSplitEvents) {
    while (true) {
      Stat inputSplitsReadyStat;
      try {
        inputSplitsReadyStat = getZkExt().exists(
            inputSplitPaths.getAllReadyPath(), true);
      } catch (KeeperException e) {
        throw new IllegalStateException("ensureInputSplitsReady: " +
            "KeeperException waiting on input splits", e);
      } catch (InterruptedException e) {
        throw new IllegalStateException("ensureInputSplitsReady: " +
            "InterruptedException waiting on input splits", e);
      }
      if (inputSplitsReadyStat != null) {
        break;
      }
      inputSplitEvents.getAllReadyChanged().waitForever();
      inputSplitEvents.getAllReadyChanged().reset();
    }
  }

  /**
   * Wait for all workers to finish processing input splits.
   *
   * @param inputSplitPaths Input split paths
   * @param inputSplitEvents Input split events
   */
  private void waitForOtherWorkers(InputSplitPaths inputSplitPaths,
                                   InputSplitEvents inputSplitEvents) {
    String workerInputSplitsDonePath =
        inputSplitPaths.getDonePath() + "/" +
            getWorkerInfo().getHostnameId();
    try {
      getZkExt().createExt(workerInputSplitsDonePath,
          null,
          Ids.OPEN_ACL_UNSAFE,
          CreateMode.PERSISTENT,
          true);
    } catch (KeeperException e) {
      throw new IllegalStateException("waitForOtherWorkers: " +
          "KeeperException creating worker done splits", e);
    } catch (InterruptedException e) {
      throw new IllegalStateException("waitForOtherWorkers: " +
          "InterruptedException creating worker done splits", e);
    }
    while (true) {
      Stat inputSplitsDoneStat;
      try {
        inputSplitsDoneStat =
            getZkExt().exists(inputSplitPaths.getAllDonePath(),
                true);
      } catch (KeeperException e) {
        throw new IllegalStateException("waitForOtherWorkers: " +
            "KeeperException waiting on worker done splits", e);
      } catch (InterruptedException e) {
        throw new IllegalStateException("waitForOtherWorkers: " +
            "InterruptedException waiting on worker done splits", e);
      }
      if (inputSplitsDoneStat != null) {
        break;
      }
      inputSplitEvents.getAllDoneChanged().waitForever();
      inputSplitEvents.getAllDoneChanged().reset();
    }
  }

  @Override
  public FinishedSuperstepStats setup() {
    // Unless doing a restart, prepare for computation:
    // 1. Start superstep INPUT_SUPERSTEP (no computation)
    // 2. Wait until the INPUT_SPLIT_ALL_READY_PATH node has been created
    // 3. Process input splits until there are no more.
    // 4. Wait until the INPUT_SPLIT_ALL_DONE_PATH node has been created
    // 5. Process any mutations deriving from add edge requests
    // 6. Wait for superstep INPUT_SUPERSTEP to complete.
    if (getRestartedSuperstep() != UNSET_SUPERSTEP) {
      setCachedSuperstep(getRestartedSuperstep());
      return new FinishedSuperstepStats(0, false, 0, 0, true);
    }

    JSONObject jobState = getJobState();
    if (jobState != null) {
      try {
        if ((ApplicationState.valueOf(jobState.getString(JSONOBJ_STATE_KEY)) ==
            ApplicationState.START_SUPERSTEP) &&
            jobState.getLong(JSONOBJ_SUPERSTEP_KEY) ==
            getSuperstep()) {
          if (LOG.isInfoEnabled()) {
            LOG.info("setup: Restarting from an automated " +
                "checkpointed superstep " +
                getSuperstep() + ", attempt " +
                getApplicationAttempt());
          }
          setRestartedSuperstep(getSuperstep());
          return new FinishedSuperstepStats(0, false, 0, 0, true);
        }
      } catch (JSONException e) {
        throw new RuntimeException(
            "setup: Failed to get key-values from " +
                jobState.toString(), e);
      }
    }

    // Add the partitions that this worker owns
    GraphState<I, V, E, M> graphState =
        new GraphState<I, V, E, M>(INPUT_SUPERSTEP, 0, 0,
            getContext(), getGraphTaskManager(), null, null);
    Collection<? extends PartitionOwner> masterSetPartitionOwners =
        startSuperstep(graphState);
    workerGraphPartitioner.updatePartitionOwners(
        getWorkerInfo(), masterSetPartitionOwners, getPartitionStore());

/*if[HADOOP_NON_SECURE]
    workerClient.setup();
else[HADOOP_NON_SECURE]*/
    workerClient.setup(getConfiguration().authenticate());
/*end[HADOOP_NON_SECURE]*/

    VertexEdgeCount vertexEdgeCount;

    if (getConfiguration().hasVertexInputFormat()) {
      // Ensure the vertex InputSplits are ready for processing
      ensureInputSplitsReady(vertexInputSplitsPaths, vertexInputSplitsEvents);
      getContext().progress();
      try {
        vertexEdgeCount = loadVertices();
      } catch (InterruptedException e) {
        throw new IllegalStateException(
            "setup: loadVertices failed with InterruptedException", e);
      } catch (KeeperException e) {
        throw new IllegalStateException(
            "setup: loadVertices failed with KeeperException", e);
      }
      getContext().progress();
    } else {
      vertexEdgeCount = new VertexEdgeCount();
    }

    if (getConfiguration().hasEdgeInputFormat()) {
      // Ensure the edge InputSplits are ready for processing
      ensureInputSplitsReady(edgeInputSplitsPaths, edgeInputSplitsEvents);
      getContext().progress();
      try {
        vertexEdgeCount = vertexEdgeCount.incrVertexEdgeCount(0, loadEdges());
      } catch (InterruptedException e) {
        throw new IllegalStateException(
            "setup: loadEdges failed with InterruptedException", e);
      } catch (KeeperException e) {
        throw new IllegalStateException(
            "setup: loadEdges failed with KeeperException", e);
      }
      getContext().progress();
    }

    if (LOG.isInfoEnabled()) {
      LOG.info("setup: Finally loaded a total of " + vertexEdgeCount);
    }

    if (getConfiguration().hasVertexInputFormat()) {
      // Workers wait for each other to finish, coordinated by master
      waitForOtherWorkers(vertexInputSplitsPaths, vertexInputSplitsEvents);
    }

    if (getConfiguration().hasEdgeInputFormat()) {
      // Workers wait for each other to finish, coordinated by master
      waitForOtherWorkers(edgeInputSplitsPaths, edgeInputSplitsEvents);
    }

    // Create remaining partitions owned by this worker.
    for (PartitionOwner partitionOwner : masterSetPartitionOwners) {
      if (partitionOwner.getWorkerInfo().equals(getWorkerInfo()) &&
          !getPartitionStore().hasPartition(
              partitionOwner.getPartitionId())) {
        Partition<I, V, E, M> partition =
            getConfiguration().createPartition(
                partitionOwner.getPartitionId(), getContext());
        getPartitionStore().addPartition(partition);
      }
    }

    if (getConfiguration().hasEdgeInputFormat()) {
      // Move edges from temporary storage to their source vertices.
      getServerData().getEdgeStore().moveEdgesToVertices();
    }

    // Generate the partition stats for the input superstep and process
    // if necessary
    List<PartitionStats> partitionStatsList =
        new ArrayList<PartitionStats>();
    for (Integer partitionId : getPartitionStore().getPartitionIds()) {
      Partition<I, V, E, M> partition =
          getPartitionStore().getPartition(partitionId);
      PartitionStats partitionStats =
          new PartitionStats(partition.getId(),
              partition.getVertexCount(),
              0,
              partition.getEdgeCount(),
              0);
      partitionStatsList.add(partitionStats);
      getPartitionStore().putPartition(partition);
    }
    workerGraphPartitioner.finalizePartitionStats(
        partitionStatsList, getPartitionStore());

    return finishSuperstep(graphState, partitionStatsList);
  }

  /**
   * Register the health of this worker for a given superstep
   *
   * @param superstep Superstep to register health on
   */
  private void registerHealth(long superstep) {
    JSONArray hostnamePort = new JSONArray();
    hostnamePort.put(getHostname());

    hostnamePort.put(workerInfo.getPort());

    String myHealthPath = null;
    if (isHealthy()) {
      myHealthPath = getWorkerInfoHealthyPath(getApplicationAttempt(),
          getSuperstep());
    } else {
      myHealthPath = getWorkerInfoUnhealthyPath(getApplicationAttempt(),
          getSuperstep());
    }
    myHealthPath = myHealthPath + "/" + workerInfo.getHostnameId();
    try {
      myHealthZnode = getZkExt().createExt(
          myHealthPath,
          WritableUtils.writeToByteArray(workerInfo),
          Ids.OPEN_ACL_UNSAFE,
          CreateMode.EPHEMERAL,
          true);
    } catch (KeeperException.NodeExistsException e) {
      LOG.warn("registerHealth: myHealthPath already exists (likely " +
          "from previous failure): " + myHealthPath +
          ".  Waiting for change in attempts " +
          "to re-join the application");
      getApplicationAttemptChangedEvent().waitForever();
      if (LOG.isInfoEnabled()) {
        LOG.info("registerHealth: Got application " +
            "attempt changed event, killing self");
      }
      throw new IllegalStateException(
          "registerHealth: Trying " +
              "to get the new application attempt by killing self", e);
    } catch (KeeperException e) {
      throw new IllegalStateException("Creating " + myHealthPath +
          " failed with KeeperException", e);
    } catch (InterruptedException e) {
      throw new IllegalStateException("Creating " + myHealthPath +
          " failed with InterruptedException", e);
    }
    if (LOG.isInfoEnabled()) {
      LOG.info("registerHealth: Created my health node for attempt=" +
          getApplicationAttempt() + ", superstep=" +
          getSuperstep() + " with " + myHealthZnode +
          " and workerInfo= " + workerInfo);
    }
  }

  /**
   * Do this to help notify the master quicker that this worker has failed.
   */
  private void unregisterHealth() {
    LOG.error("unregisterHealth: Got failure, unregistering health on " +
        myHealthZnode + " on superstep " + getSuperstep());
    try {
      getZkExt().deleteExt(myHealthZnode, -1, false);
    } catch (InterruptedException e) {
      throw new IllegalStateException(
          "unregisterHealth: InterruptedException - Couldn't delete " +
              myHealthZnode, e);
    } catch (KeeperException e) {
      throw new IllegalStateException(
          "unregisterHealth: KeeperException - Couldn't delete " +
              myHealthZnode, e);
    }
  }

  @Override
  public void failureCleanup() {
    unregisterHealth();
  }

  @Override
  public Collection<? extends PartitionOwner> startSuperstep(
      GraphState<I, V, E, M> graphState) {
    // Algorithm:
    // 1. Communication service will combine message from previous
    //    superstep
    // 2. Register my health for the next superstep.
    // 3. Wait until the partition assignment is complete and get it
    // 4. Get the aggregator values from the previous superstep
    if (getSuperstep() != INPUT_SUPERSTEP) {
      workerServer.prepareSuperstep(graphState);
    }

    registerHealth(getSuperstep());

    String addressesAndPartitionsPath =
        getAddressesAndPartitionsPath(getApplicationAttempt(),
            getSuperstep());
    AddressesAndPartitionsWritable addressesAndPartitions =
        new AddressesAndPartitionsWritable(
            workerGraphPartitioner.createPartitionOwner().getClass());
    try {
      while (getZkExt().exists(addressesAndPartitionsPath, true) ==
          null) {
        getAddressesAndPartitionsReadyChangedEvent().waitForever();
        getAddressesAndPartitionsReadyChangedEvent().reset();
      }
      WritableUtils.readFieldsFromZnode(
          getZkExt(),
          addressesAndPartitionsPath,
          false,
          null,
          addressesAndPartitions);
    } catch (KeeperException e) {
      throw new IllegalStateException(
          "startSuperstep: KeeperException getting assignments", e);
    } catch (InterruptedException e) {
      throw new IllegalStateException(
          "startSuperstep: InterruptedException getting assignments", e);
    }

    workerInfoList.clear();
    workerInfoList = addressesAndPartitions.getWorkerInfos();
    masterInfo = addressesAndPartitions.getMasterInfo();

    if (LOG.isInfoEnabled()) {
      LOG.info("startSuperstep: " + masterInfo);
      LOG.info("startSuperstep: Ready for computation on superstep " +
          getSuperstep() + " since worker " +
          "selection and vertex range assignments are done in " +
          addressesAndPartitionsPath);
    }

    getContext().setStatus("startSuperstep: " +
        getGraphTaskManager().getGraphFunctions().toString() +
        " - Attempt=" + getApplicationAttempt() +
        ", Superstep=" + getSuperstep());
    return addressesAndPartitions.getPartitionOwners();
  }

  @Override
  public FinishedSuperstepStats finishSuperstep(
      GraphState<I, V, E, M> graphState,
      List<PartitionStats> partitionStatsList) {
    // This barrier blocks until success (or the master signals it to
    // restart).
    //
    // Master will coordinate the barriers and aggregate "doneness" of all
    // the vertices.  Each worker will:
    // 1. Ensure that the requests are complete
    // 2. Execute user postSuperstep() if necessary.
    // 3. Save aggregator values that are in use.
    // 4. Report the statistics (vertices, edges, messages, etc.)
    //    of this worker
    // 5. Let the master know it is finished.
    // 6. Wait for the master's global stats, and check if done
    waitForRequestsToFinish();

    graphState.getGraphTaskManager().notifyFinishedCommunication();

    long workerSentMessages = 0;
    long localVertices = 0;
    for (PartitionStats partitionStats : partitionStatsList) {
      workerSentMessages += partitionStats.getMessagesSentCount();
      localVertices += partitionStats.getVertexCount();
    }

    if (getSuperstep() != INPUT_SUPERSTEP) {
      postSuperstepCallbacks(graphState);
    }

    aggregatorHandler.finishSuperstep(workerAggregatorRequestProcessor);

    if (LOG.isInfoEnabled()) {
      LOG.info("finishSuperstep: Superstep " + getSuperstep() +
          ", messages = " + workerSentMessages + " " +
          MemoryUtils.getRuntimeMemoryStats());
    }

    writeFinshedSuperstepInfoToZK(partitionStatsList, workerSentMessages);

    LoggerUtils.setStatusAndLog(getContext(), LOG, Level.INFO,
        "finishSuperstep: (waiting for rest " +
            "of workers) " +
            getGraphTaskManager().getGraphFunctions().toString() +
            " - Attempt=" + getApplicationAttempt() +
            ", Superstep=" + getSuperstep());

    String superstepFinishedNode =
        getSuperstepFinishedPath(getApplicationAttempt(), getSuperstep());

    waitForOtherWorkers(superstepFinishedNode);

    GlobalStats globalStats = new GlobalStats();
    WritableUtils.readFieldsFromZnode(
        getZkExt(), superstepFinishedNode, false, null, globalStats);
    if (LOG.isInfoEnabled()) {
      LOG.info("finishSuperstep: Completed superstep " + getSuperstep() +
          " with global stats " + globalStats);
    }
    incrCachedSuperstep();
    getContext().setStatus("finishSuperstep: (all workers done) " +
        getGraphTaskManager().getGraphFunctions().toString() +
        " - Attempt=" + getApplicationAttempt() +
        ", Superstep=" + getSuperstep());

    return new FinishedSuperstepStats(
        localVertices,
        globalStats.getHaltComputation(),
        globalStats.getVertexCount(),
        globalStats.getEdgeCount(),
        false);
  }

  /**
   * Handle post-superstep callbacks
   *
   * @param graphState GraphState
   */
  private void postSuperstepCallbacks(GraphState<I, V, E, M> graphState) {
    getWorkerContext().setGraphState(graphState);
    GiraphTimerContext timerContext = wcPostSuperstepTimer.time();
    getWorkerContext().postSuperstep();
    timerContext.stop();
    getContext().progress();

    for (WorkerObserver obs : getWorkerObservers()) {
      obs.postSuperstep(graphState.getSuperstep());
      getContext().progress();
    }
  }

  /**
   * Wait for all the requests to finish.
   */
  private void waitForRequestsToFinish() {
    if (LOG.isInfoEnabled()) {
      LOG.info("finishSuperstep: Waiting on all requests, superstep " +
          getSuperstep() + " " +
          MemoryUtils.getRuntimeMemoryStats());
    }
    GiraphTimerContext timerContext = waitRequestsTimer.time();
    workerClient.waitAllRequests();
    timerContext.stop();
  }

  /**
   * Wait for all the other Workers to finish the superstep.
   *
   * @param superstepFinishedNode ZooKeeper path to wait on.
   */
  private void waitForOtherWorkers(String superstepFinishedNode) {
    try {
      while (getZkExt().exists(superstepFinishedNode, true) == null) {
        getSuperstepFinishedEvent().waitForever();
        getSuperstepFinishedEvent().reset();
      }
    } catch (KeeperException e) {
      throw new IllegalStateException(
          "finishSuperstep: Failed while waiting for master to " +
              "signal completion of superstep " + getSuperstep(), e);
    } catch (InterruptedException e) {
      throw new IllegalStateException(
          "finishSuperstep: Failed while waiting for master to " +
              "signal completion of superstep " + getSuperstep(), e);
    }
  }

  /**
   * Write finished superstep info to ZooKeeper.
   *
   * @param partitionStatsList List of partition stats from superstep.
   * @param workerSentMessages Number of messages sent in superstep.
   */
  private void writeFinshedSuperstepInfoToZK(
      List<PartitionStats> partitionStatsList, long workerSentMessages) {
    Collection<PartitionStats> finalizedPartitionStats =
        workerGraphPartitioner.finalizePartitionStats(
            partitionStatsList, getPartitionStore());
    List<PartitionStats> finalizedPartitionStatsList =
        new ArrayList<PartitionStats>(finalizedPartitionStats);
    byte[] partitionStatsBytes =
        WritableUtils.writeListToByteArray(finalizedPartitionStatsList);
    WorkerSuperstepMetrics metrics = new WorkerSuperstepMetrics();
    metrics.readFromRegistry();
    byte[] metricsBytes = WritableUtils.writeToByteArray(metrics);

    JSONObject workerFinishedInfoObj = new JSONObject();
    try {
      workerFinishedInfoObj.put(JSONOBJ_PARTITION_STATS_KEY,
          Base64.encodeBytes(partitionStatsBytes));
      workerFinishedInfoObj.put(JSONOBJ_NUM_MESSAGES_KEY, workerSentMessages);
      workerFinishedInfoObj.put(JSONOBJ_METRICS_KEY,
          Base64.encodeBytes(metricsBytes));
    } catch (JSONException e) {
      throw new RuntimeException(e);
    }

    String finishedWorkerPath =
        getWorkerFinishedPath(getApplicationAttempt(), getSuperstep()) +
        "/" + getHostnamePartitionId();
    try {
      getZkExt().createExt(finishedWorkerPath,
          workerFinishedInfoObj.toString().getBytes(),
          Ids.OPEN_ACL_UNSAFE,
          CreateMode.PERSISTENT,
          true);
    } catch (KeeperException.NodeExistsException e) {
      LOG.warn("finishSuperstep: finished worker path " +
          finishedWorkerPath + " already exists!");
    } catch (KeeperException e) {
      throw new IllegalStateException("Creating " + finishedWorkerPath +
          " failed with KeeperException", e);
    } catch (InterruptedException e) {
      throw new IllegalStateException("Creating " + finishedWorkerPath +
          " failed with InterruptedException", e);
    }
  }

  /**
   * Save the vertices using the user-defined VertexOutputFormat from our
   * vertexArray based on the split.
   *
   * @param numLocalVertices Number of local vertices
   * @throws InterruptedException
   */
  private void saveVertices(long numLocalVertices) throws IOException,
      InterruptedException {
    if (getConfiguration().getVertexOutputFormatClass() == null) {
      LOG.warn("saveVertices: " +
          GiraphConstants.VERTEX_OUTPUT_FORMAT_CLASS +
          " not specified -- there will be no saved output");
      return;
    }
    if (getConfiguration().doOutputDuringComputation()) {
      if (LOG.isInfoEnabled()) {
        LOG.info("saveVertices: The option for doing output during " +
            "computation is selected, so there will be no saving of the " +
            "output in the end of application");
      }
      return;
    }

    int numThreads = Math.min(getConfiguration().getNumOutputThreads(),
        getPartitionStore().getNumPartitions());
    LoggerUtils.setStatusAndLog(getContext(), LOG, Level.INFO,
        "saveVertices: Starting to save " + numLocalVertices + " vertices " +
            "using " + numThreads + " threads");
    final VertexOutputFormat<I, V, E> vertexOutputFormat =
        getConfiguration().createVertexOutputFormat();
    CallableFactory<Void> callableFactory = new CallableFactory<Void>() {
      @Override
      public Callable<Void> newCallable(int callableId) {
        return new Callable<Void>() {
          @Override
          public Void call() throws Exception {
            VertexWriter<I, V, E> vertexWriter =
                vertexOutputFormat.createVertexWriter(getContext());
            vertexWriter.setConf(
                (ImmutableClassesGiraphConfiguration<I, V, E, Writable>)
                    getConfiguration());
            vertexWriter.initialize(getContext());
            long verticesWritten = 0;
            long nextPrintVertices = 0;
            long nextPrintMsecs = System.currentTimeMillis() + 15000;
            int partitionIndex = 0;
            int numPartitions = getPartitionStore().getNumPartitions();
            for (Integer partitionId : getPartitionStore().getPartitionIds()) {
              Partition<I, V, E, M> partition =
                  getPartitionStore().getPartition(partitionId);
              for (Vertex<I, V, E, M> vertex : partition) {
                vertexWriter.writeVertex(vertex);
                ++verticesWritten;

                // Update status at most every 250k vertices or 15 seconds
                if (verticesWritten > nextPrintVertices &&
                    System.currentTimeMillis() > nextPrintMsecs) {
                  LoggerUtils.setStatusAndLog(getContext(), LOG, Level.INFO,
                      "saveVertices: Saved " + verticesWritten + " out of " +
                          partition.getVertexCount() + " partition vertices, " +
                          "on partition " + partitionIndex +
                          " out of " + numPartitions);
                  nextPrintMsecs = System.currentTimeMillis() + 15000;
                  nextPrintVertices = verticesWritten + 250000;
                }
              }
              ++partitionIndex;
            }
            vertexWriter.close(getContext()); // the temp results are saved now
            return null;
          }
        };
      }
    };
    ProgressableUtils.getResultsWithNCallables(callableFactory, numThreads,
        "save-vertices-%d", getContext());

    LoggerUtils.setStatusAndLog(getContext(), LOG, Level.INFO,
      "saveVertices: Done saving vertices.");
    // YARN: must complete the commit the "task" output, Hadoop isn't there.
    if (getConfiguration().isPureYarnJob() &&
      getConfiguration().getVertexOutputFormatClass() != null) {
      try {
        OutputCommitter outputCommitter =
          vertexOutputFormat.getOutputCommitter(getContext());
        if (outputCommitter.needsTaskCommit(getContext())) {
          LoggerUtils.setStatusAndLog(getContext(), LOG, Level.INFO,
            "OutputCommitter: committing task output.");
          // transfer from temp dirs to "task commit" dirs to prep for
          // the master's OutputCommitter#commitJob(context) call to finish.
          outputCommitter.commitTask(getContext());
        }
      } catch (InterruptedException ie) {
        LOG.error("Interrupted while attempting to obtain " +
          "OutputCommitter.", ie);
      } catch (IOException ioe) {
        LOG.error("Master task's attempt to commit output has " +
          "FAILED.", ioe);
      }
    }
  }

  @Override
  public void cleanup(FinishedSuperstepStats finishedSuperstepStats)
    throws IOException, InterruptedException {
    workerClient.closeConnections();
    setCachedSuperstep(getSuperstep() - 1);
    saveVertices(finishedSuperstepStats.getLocalVertexCount());
    getPartitionStore().shutdown();
    // All worker processes should denote they are done by adding special
    // znode.  Once the number of znodes equals the number of partitions
    // for workers and masters, the master will clean up the ZooKeeper
    // znodes associated with this job.
    String workerCleanedUpPath = cleanedUpPath  + "/" +
        getTaskPartition() + WORKER_SUFFIX;
    try {
      String finalFinishedPath =
          getZkExt().createExt(workerCleanedUpPath,
              null,
              Ids.OPEN_ACL_UNSAFE,
              CreateMode.PERSISTENT,
              true);
      if (LOG.isInfoEnabled()) {
        LOG.info("cleanup: Notifying master its okay to cleanup with " +
            finalFinishedPath);
      }
    } catch (KeeperException.NodeExistsException e) {
      if (LOG.isInfoEnabled()) {
        LOG.info("cleanup: Couldn't create finished node '" +
            workerCleanedUpPath);
      }
    } catch (KeeperException e) {
      // Cleaning up, it's okay to fail after cleanup is successful
      LOG.error("cleanup: Got KeeperException on notification " +
          "to master about cleanup", e);
    } catch (InterruptedException e) {
      // Cleaning up, it's okay to fail after cleanup is successful
      LOG.error("cleanup: Got InterruptedException on notification " +
          "to master about cleanup", e);
    }
    try {
      getZkExt().close();
    } catch (InterruptedException e) {
      // cleanup phase -- just log the error
      LOG.error("cleanup: Zookeeper failed to close with " + e);
    }

    if (getConfiguration().metricsEnabled()) {
      GiraphMetrics.get().dumpToStream(System.err);
    }

    // Preferably would shut down the service only after
    // all clients have disconnected (or the exceptions on the
    // client side ignored).
    workerServer.close();
  }

  @Override
  public void storeCheckpoint() throws IOException {
    LoggerUtils.setStatusAndLog(getContext(), LOG, Level.INFO,
        "storeCheckpoint: Starting checkpoint " +
            getGraphTaskManager().getGraphFunctions().toString() +
            " - Attempt=" + getApplicationAttempt() +
            ", Superstep=" + getSuperstep());

    // Algorithm:
    // For each partition, dump vertices and messages
    Path metadataFilePath =
        new Path(getCheckpointBasePath(getSuperstep()) + "." +
            getHostnamePartitionId() +
            CHECKPOINT_METADATA_POSTFIX);
    Path verticesFilePath =
        new Path(getCheckpointBasePath(getSuperstep()) + "." +
            getHostnamePartitionId() +
            CHECKPOINT_VERTICES_POSTFIX);
    Path validFilePath =
        new Path(getCheckpointBasePath(getSuperstep()) + "." +
            getHostnamePartitionId() +
            CHECKPOINT_VALID_POSTFIX);

    // Remove these files if they already exist (shouldn't though, unless
    // of previous failure of this worker)
    if (getFs().delete(validFilePath, false)) {
      LOG.warn("storeCheckpoint: Removed valid file " +
          validFilePath);
    }
    if (getFs().delete(metadataFilePath, false)) {
      LOG.warn("storeCheckpoint: Removed metadata file " +
          metadataFilePath);
    }
    if (getFs().delete(verticesFilePath, false)) {
      LOG.warn("storeCheckpoint: Removed file " + verticesFilePath);
    }

    FSDataOutputStream verticesOutputStream =
        getFs().create(verticesFilePath);
    ByteArrayOutputStream metadataByteStream = new ByteArrayOutputStream();
    DataOutput metadataOutput = new DataOutputStream(metadataByteStream);
    for (Integer partitionId : getPartitionStore().getPartitionIds()) {
      Partition<I, V, E, M> partition =
          getPartitionStore().getPartition(partitionId);
      long startPos = verticesOutputStream.getPos();
      partition.write(verticesOutputStream);
      // write messages
      getServerData().getCurrentMessageStore().writePartition(
          verticesOutputStream, partition.getId());
      // Write the metadata for this partition
      // Format:
      // <index count>
      //   <index 0 start pos><partition id>
      //   <index 1 start pos><partition id>
      metadataOutput.writeLong(startPos);
      metadataOutput.writeInt(partition.getId());
      if (LOG.isDebugEnabled()) {
        LOG.debug("storeCheckpoint: Vertex file starting " +
            "offset = " + startPos + ", length = " +
            (verticesOutputStream.getPos() - startPos) +
            ", partition = " + partition.toString());
      }
      getPartitionStore().putPartition(partition);
      getContext().progress();
    }
    // Metadata is buffered and written at the end since it's small and
    // needs to know how many partitions this worker owns
    FSDataOutputStream metadataOutputStream =
        getFs().create(metadataFilePath);
    metadataOutputStream.writeInt(getPartitionStore().getNumPartitions());
    metadataOutputStream.write(metadataByteStream.toByteArray());
    metadataOutputStream.close();
    verticesOutputStream.close();
    if (LOG.isInfoEnabled()) {
      LOG.info("storeCheckpoint: Finished metadata (" +
          metadataFilePath + ") and vertices (" + verticesFilePath + ").");
    }

    getFs().createNewFile(validFilePath);

    // Notify master that checkpoint is stored
    String workerWroteCheckpoint =
        getWorkerWroteCheckpointPath(getApplicationAttempt(),
            getSuperstep()) + "/" + getHostnamePartitionId();
    try {
      getZkExt().createExt(workerWroteCheckpoint,
          new byte[0],
          Ids.OPEN_ACL_UNSAFE,
          CreateMode.PERSISTENT,
          true);
    } catch (KeeperException.NodeExistsException e) {
      LOG.warn("storeCheckpoint: wrote checkpoint worker path " +
          workerWroteCheckpoint + " already exists!");
    } catch (KeeperException e) {
      throw new IllegalStateException("Creating " + workerWroteCheckpoint +
          " failed with KeeperException", e);
    } catch (InterruptedException e) {
      throw new IllegalStateException("Creating " +
          workerWroteCheckpoint +
          " failed with InterruptedException", e);
    }
  }

  @Override
  public VertexEdgeCount loadCheckpoint(long superstep) {
    try {
      // clear old message stores
      getServerData().getIncomingMessageStore().clearAll();
      getServerData().getCurrentMessageStore().clearAll();
    } catch (IOException e) {
      throw new RuntimeException(
          "loadCheckpoint: Failed to clear message stores ", e);
    }

    // Algorithm:
    // Examine all the partition owners and load the ones
    // that match my hostname and id from the master designated checkpoint
    // prefixes.
    long startPos = 0;
    int loadedPartitions = 0;
    for (PartitionOwner partitionOwner :
      workerGraphPartitioner.getPartitionOwners()) {
      if (partitionOwner.getWorkerInfo().equals(getWorkerInfo())) {
        String metadataFile =
            partitionOwner.getCheckpointFilesPrefix() +
            CHECKPOINT_METADATA_POSTFIX;
        String partitionsFile =
            partitionOwner.getCheckpointFilesPrefix() +
            CHECKPOINT_VERTICES_POSTFIX;
        try {
          int partitionId = -1;
          DataInputStream metadataStream =
              getFs().open(new Path(metadataFile));
          int partitions = metadataStream.readInt();
          for (int i = 0; i < partitions; ++i) {
            startPos = metadataStream.readLong();
            partitionId = metadataStream.readInt();
            if (partitionId == partitionOwner.getPartitionId()) {
              break;
            }
          }
          if (partitionId != partitionOwner.getPartitionId()) {
            throw new IllegalStateException(
                "loadCheckpoint: " + partitionOwner +
                " not found!");
          }
          metadataStream.close();
          Partition<I, V, E, M> partition =
              getConfiguration().createPartition(partitionId, getContext());
          DataInputStream partitionsStream =
              getFs().open(new Path(partitionsFile));
          if (partitionsStream.skip(startPos) != startPos) {
            throw new IllegalStateException(
                "loadCheckpoint: Failed to skip " + startPos +
                " on " + partitionsFile);
          }
          partition.readFields(partitionsStream);
          if (partitionsStream.readBoolean()) {
            getServerData().getCurrentMessageStore().readFieldsForPartition(
                partitionsStream, partitionId);
          }
          partitionsStream.close();
          if (LOG.isInfoEnabled()) {
            LOG.info("loadCheckpoint: Loaded partition " +
                partition);
          }
          if (getPartitionStore().hasPartition(partitionId)) {
            throw new IllegalStateException(
                "loadCheckpoint: Already has partition owner " +
                    partitionOwner);
          }
          getPartitionStore().addPartition(partition);
          getContext().progress();
          ++loadedPartitions;
        } catch (IOException e) {
          throw new RuntimeException(
              "loadCheckpoint: Failed to get partition owner " +
                  partitionOwner, e);
        }
      }
    }
    if (LOG.isInfoEnabled()) {
      LOG.info("loadCheckpoint: Loaded " + loadedPartitions +
          " partitions of out " +
          workerGraphPartitioner.getPartitionOwners().size() +
          " total.");
    }

    // Load global statistics
    GlobalStats globalStats = null;
    String finalizedCheckpointPath =
        getCheckpointBasePath(superstep) + CHECKPOINT_FINALIZED_POSTFIX;
    try {
      DataInputStream finalizedStream =
          getFs().open(new Path(finalizedCheckpointPath));
      globalStats = new GlobalStats();
      globalStats.readFields(finalizedStream);
    } catch (IOException e) {
      throw new IllegalStateException(
          "loadCheckpoint: Failed to load global statistics", e);
    }

    // Communication service needs to setup the connections prior to
    // processing vertices
/*if[HADOOP_NON_SECURE]
    workerClient.setup();
else[HADOOP_NON_SECURE]*/
    workerClient.setup(getConfiguration().authenticate());
/*end[HADOOP_NON_SECURE]*/
    return new VertexEdgeCount(globalStats.getVertexCount(),
        globalStats.getEdgeCount());
  }

  /**
   * Send the worker partitions to their destination workers
   *
   * @param workerPartitionMap Map of worker info to the partitions stored
   *        on this worker to be sent
   */
  private void sendWorkerPartitions(
      Map<WorkerInfo, List<Integer>> workerPartitionMap) {
    List<Entry<WorkerInfo, List<Integer>>> randomEntryList =
        new ArrayList<Entry<WorkerInfo, List<Integer>>>(
            workerPartitionMap.entrySet());
    Collections.shuffle(randomEntryList);
    WorkerClientRequestProcessor<I, V, E, M> workerClientRequestProcessor =
        new NettyWorkerClientRequestProcessor<I, V, E, M>(getContext(),
            getConfiguration(), this);
    for (Entry<WorkerInfo, List<Integer>> workerPartitionList :
      randomEntryList) {
      for (Integer partitionId : workerPartitionList.getValue()) {
        Partition<I, V, E, M> partition =
            getPartitionStore().removePartition(partitionId);
        if (partition == null) {
          throw new IllegalStateException(
              "sendWorkerPartitions: Couldn't find partition " +
                  partitionId + " to send to " +
                  workerPartitionList.getKey());
        }
        if (LOG.isInfoEnabled()) {
          LOG.info("sendWorkerPartitions: Sending worker " +
              workerPartitionList.getKey() + " partition " +
              partitionId);
        }
        workerClientRequestProcessor.sendPartitionRequest(
            workerPartitionList.getKey(),
            partition);
      }
    }


    try {
      workerClientRequestProcessor.flush();
      workerClient.waitAllRequests();
    } catch (IOException e) {
      throw new IllegalStateException("sendWorkerPartitions: Flush failed", e);
    }
    String myPartitionExchangeDonePath =
        getPartitionExchangeWorkerPath(
            getApplicationAttempt(), getSuperstep(), getWorkerInfo());
    try {
      getZkExt().createExt(myPartitionExchangeDonePath,
          null,
          Ids.OPEN_ACL_UNSAFE,
          CreateMode.PERSISTENT,
          true);
    } catch (KeeperException e) {
      throw new IllegalStateException(
          "sendWorkerPartitions: KeeperException to create " +
              myPartitionExchangeDonePath, e);
    } catch (InterruptedException e) {
      throw new IllegalStateException(
          "sendWorkerPartitions: InterruptedException to create " +
              myPartitionExchangeDonePath, e);
    }
    if (LOG.isInfoEnabled()) {
      LOG.info("sendWorkerPartitions: Done sending all my partitions.");
    }
  }

  @Override
  public final void exchangeVertexPartitions(
      Collection<? extends PartitionOwner> masterSetPartitionOwners) {
    // 1. Fix the addresses of the partition ids if they have changed.
    // 2. Send all the partitions to their destination workers in a random
    //    fashion.
    // 3. Notify completion with a ZooKeeper stamp
    // 4. Wait for all my dependencies to be done (if any)
    // 5. Add the partitions to myself.
    PartitionExchange partitionExchange =
        workerGraphPartitioner.updatePartitionOwners(
            getWorkerInfo(), masterSetPartitionOwners, getPartitionStore());
    workerClient.openConnections();

    Map<WorkerInfo, List<Integer>> sendWorkerPartitionMap =
        partitionExchange.getSendWorkerPartitionMap();
    if (!getPartitionStore().isEmpty()) {
      sendWorkerPartitions(sendWorkerPartitionMap);
    }

    Set<WorkerInfo> myDependencyWorkerSet =
        partitionExchange.getMyDependencyWorkerSet();
    Set<String> workerIdSet = new HashSet<String>();
    for (WorkerInfo tmpWorkerInfo : myDependencyWorkerSet) {
      if (!workerIdSet.add(tmpWorkerInfo.getHostnameId())) {
        throw new IllegalStateException(
            "exchangeVertexPartitions: Duplicate entry " + tmpWorkerInfo);
      }
    }
    if (myDependencyWorkerSet.isEmpty() && getPartitionStore().isEmpty()) {
      if (LOG.isInfoEnabled()) {
        LOG.info("exchangeVertexPartitions: Nothing to exchange, " +
            "exiting early");
      }
      return;
    }

    String vertexExchangePath =
        getPartitionExchangePath(getApplicationAttempt(), getSuperstep());
    List<String> workerDoneList;
    try {
      while (true) {
        workerDoneList = getZkExt().getChildrenExt(
            vertexExchangePath, true, false, false);
        workerIdSet.removeAll(workerDoneList);
        if (workerIdSet.isEmpty()) {
          break;
        }
        if (LOG.isInfoEnabled()) {
          LOG.info("exchangeVertexPartitions: Waiting for workers " +
              workerIdSet);
        }
        getPartitionExchangeChildrenChangedEvent().waitForever();
        getPartitionExchangeChildrenChangedEvent().reset();
      }
    } catch (KeeperException e) {
      throw new RuntimeException(e);
    } catch (InterruptedException e) {
      throw new RuntimeException(e);
    }

    if (LOG.isInfoEnabled()) {
      LOG.info("exchangeVertexPartitions: Done with exchange.");
    }
  }

  /**
   * Get event when the state of a partition exchange has changed.
   *
   * @return Event to check.
   */
  public final BspEvent getPartitionExchangeChildrenChangedEvent() {
    return partitionExchangeChildrenChanged;
  }

  @Override
  protected boolean processEvent(WatchedEvent event) {
    boolean foundEvent = false;
    if (event.getPath().startsWith(masterJobStatePath) &&
        (event.getType() == EventType.NodeChildrenChanged)) {
      if (LOG.isInfoEnabled()) {
        LOG.info("processEvent: Job state changed, checking " +
            "to see if it needs to restart");
      }
      JSONObject jsonObj = getJobState();
      // in YARN, we have to manually commit our own output in 2 stages that we
      // do not have to do in Hadoop-based Giraph. So jsonObj can be null.
      if (getConfiguration().isPureYarnJob() && null == jsonObj) {
        LOG.error("BspServiceWorker#getJobState() came back NULL.");
        return false; // the event has been processed.
      }
      try {
        if ((ApplicationState.valueOf(jsonObj.getString(JSONOBJ_STATE_KEY)) ==
            ApplicationState.START_SUPERSTEP) &&
            jsonObj.getLong(JSONOBJ_APPLICATION_ATTEMPT_KEY) !=
            getApplicationAttempt()) {
          LOG.fatal("processEvent: Worker will restart " +
              "from command - " + jsonObj.toString());
          System.exit(-1);
        }
      } catch (JSONException e) {
        throw new RuntimeException(
            "processEvent: Couldn't properly get job state from " +
                jsonObj.toString());
      }
      foundEvent = true;
    } else if (event.getPath().contains(PARTITION_EXCHANGE_DIR) &&
        event.getType() == EventType.NodeChildrenChanged) {
      if (LOG.isInfoEnabled()) {
        LOG.info("processEvent : partitionExchangeChildrenChanged " +
            "(at least one worker is done sending partitions)");
      }
      partitionExchangeChildrenChanged.signal();
      foundEvent = true;
    }

    return foundEvent;
  }

  @Override
  public WorkerInfo getWorkerInfo() {
    return workerInfo;
  }

  @Override
  public PartitionStore<I, V, E, M> getPartitionStore() {
    return getServerData().getPartitionStore();
  }

  @Override
  public PartitionOwner getVertexPartitionOwner(I vertexId) {
    return workerGraphPartitioner.getPartitionOwner(vertexId);
  }

  @Override
  public Iterable<? extends PartitionOwner> getPartitionOwners() {
    return workerGraphPartitioner.getPartitionOwners();
  }

  @Override
  public Integer getPartitionId(I vertexId) {
    PartitionOwner partitionOwner = getVertexPartitionOwner(vertexId);
    return partitionOwner.getPartitionId();
  }

  @Override
  public boolean hasPartition(Integer partitionId) {
    return getPartitionStore().hasPartition(partitionId);
  }

  @Override
  public ServerData<I, V, E, M> getServerData() {
    return workerServer.getServerData();
  }

  @Override
  public WorkerAggregatorHandler getAggregatorHandler() {
    return aggregatorHandler;
  }

  @Override
  public void prepareSuperstep() {
    if (getSuperstep() != INPUT_SUPERSTEP) {
      aggregatorHandler.prepareSuperstep(workerAggregatorRequestProcessor);
    }
  }

  @Override
  public SuperstepOutput<I, V, E> getSuperstepOutput() {
    return superstepOutput;
  }
}
