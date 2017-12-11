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

package org.apache.giraph.graph;

import org.apache.giraph.bsp.BspService;
import org.apache.giraph.bsp.CentralizedServiceMaster;
import org.apache.giraph.bsp.CentralizedServiceWorker;
import org.apache.giraph.comm.messages.MessageStoreByPartition;
import org.apache.giraph.conf.GiraphConstants;
import org.apache.giraph.conf.ImmutableClassesGiraphConfiguration;
import org.apache.giraph.master.BspServiceMaster;
import org.apache.giraph.master.MasterAggregatorUsage;
import org.apache.giraph.master.MasterThread;
import org.apache.giraph.metrics.GiraphMetrics;
import org.apache.giraph.metrics.GiraphMetricsRegistry;
import org.apache.giraph.metrics.GiraphTimer;
import org.apache.giraph.metrics.GiraphTimerContext;
import org.apache.giraph.metrics.ResetSuperstepMetricsObserver;
import org.apache.giraph.metrics.SuperstepMetricsRegistry;
import org.apache.giraph.partition.PartitionOwner;
import org.apache.giraph.partition.PartitionStats;
import org.apache.giraph.time.SystemTime;
import org.apache.giraph.time.Time;
import org.apache.giraph.utils.CallableFactory;
import org.apache.giraph.utils.MemoryUtils;
import org.apache.giraph.utils.ProgressableUtils;
import org.apache.giraph.utils.ReflectionUtils;
import org.apache.giraph.worker.BspServiceWorker;
import org.apache.giraph.worker.WorkerAggregatorUsage;
import org.apache.giraph.worker.WorkerContext;
import org.apache.giraph.worker.WorkerObserver;
import org.apache.giraph.zk.ZooKeeperManager;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;
import org.apache.hadoop.mapreduce.Mapper;
import org.apache.log4j.Appender;
import org.apache.log4j.Level;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;

import java.io.IOException;
import java.lang.reflect.Type;
import java.net.URL;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.Callable;
import java.util.concurrent.TimeUnit;

import static org.apache.giraph.conf.GiraphConstants.EDGE_VALUE_CLASS;
import static org.apache.giraph.conf.GiraphConstants.MESSAGE_VALUE_CLASS;
import static org.apache.giraph.conf.GiraphConstants.VERTEX_ID_CLASS;
import static org.apache.giraph.conf.GiraphConstants.VERTEX_VALUE_CLASS;

/**
 * The Giraph-specific business logic for a single BSP
 * compute node in whatever underlying type of cluster
 * our Giraph job will run on. Owning object will provide
 * the glue into the underlying cluster framework
 * and will call this object to perform Giraph work.
 *
 * @param <I> Vertex id
 * @param <V> Vertex data
 * @param <E> Edge data
 * @param <M> Message data
 */
@SuppressWarnings("rawtypes")
public class GraphTaskManager<I extends WritableComparable, V extends Writable,
  E extends Writable, M extends Writable> implements
  ResetSuperstepMetricsObserver {
  /*if_not[PURE_YARN]
  static { // Eliminate this? Even MRv1 tasks should not need it here.
    Configuration.addDefaultResource("giraph-site.xml");
  }
  end[PURE_YARN]*/
  /** Name of metric for superstep time in msec */
  public static final String TIMER_SUPERSTEP_TIME = "superstep-time-ms";
  /** Name of metric for compute on all vertices in msec */
  public static final String TIMER_COMPUTE_ALL = "compute-all-ms";
  /** Name of metric for time from begin compute to first message sent */
  public static final String TIMER_TIME_TO_FIRST_MSG =
      "time-to-first-message-ms";
  /** Name of metric for time from first message till last message flushed */
  public static final String TIMER_COMMUNICATION_TIME = "communication-time-ms";

  /** Time instance used for timing in this class */
  private static final Time TIME = SystemTime.get();
  /** Class logger */
  private static final Logger LOG = Logger.getLogger(GraphTaskManager.class);
  /** Coordination service worker */
  private CentralizedServiceWorker<I, V, E, M> serviceWorker;
  /** Coordination service master */
  private CentralizedServiceMaster<I, V, E, M> serviceMaster;
  /** Coordination service master thread */
  private Thread masterThread = null;
  /** The worker should be run exactly once, or else there is a problem. */
  private boolean alreadyRun = false;
  /** Manages the ZooKeeper servers if necessary (dynamic startup) */
  private ZooKeeperManager zkManager;
  /** Configuration */
  private ImmutableClassesGiraphConfiguration<I, V, E, M> conf;
  /** Already complete? */
  private boolean done = false;
  /** What kind of functions is this mapper doing? */
  private GraphFunctions graphFunctions = GraphFunctions.UNKNOWN;
  /** Superstep stats */
  private FinishedSuperstepStats finishedSuperstepStats =
      new FinishedSuperstepStats(0, false, 0, 0, false);

  // Per-Job Metrics
  /** Timer for WorkerContext#preApplication() */
  private GiraphTimer wcPreAppTimer;
  /** Timer for WorkerContext#postApplication() */
  private GiraphTimer wcPostAppTimer;

  // Per-Superstep Metrics
  /** Time for how long superstep took */
  private GiraphTimer superstepTimer;
  /** Time for all compute() calls in a superstep */
  private GiraphTimer computeAll;
  /** Time from starting compute to sending first message */
  private GiraphTimer timeToFirstMessage;
  /** Context for timing time to first message above */
  private GiraphTimerContext timeToFirstMessageTimerContext;
  /** Time from first sent message till last message flushed. */
  private GiraphTimer communicationTimer;
  /** Context for timing communication time above */
  private GiraphTimerContext communicationTimerContext;
  /** Timer for WorkerContext#preSuperstep() */
  private GiraphTimer wcPreSuperstepTimer;
  /** Zookeeper host:port list */
  private String serverPortList;
  /** The Hadoop Mapper#Context for this job */
  private Mapper<?, ?, ?, ?>.Context context;
  /** is this GraphTaskManager the master? */
  private boolean isMaster;

  /**
   * Default constructor for GiraphTaskManager.
   * @param context a handle to the underlying cluster framework.
   *                For Hadoop clusters, this is a Mapper#Context.
   */
  public GraphTaskManager(Mapper<?, ?, ?, ?>.Context context) {
    this.context = context;
    this.isMaster = false;
  }

  /**
   * Called by owner of this GraphTaskManager on each compute node
   * @param zkPathList the path to the ZK jars we need to run the job
   */
  public void setup(Path[] zkPathList)
    throws IOException, InterruptedException {
    context.setStatus("setup: Beginning worker setup.");
    conf = new ImmutableClassesGiraphConfiguration<I, V, E, M>(
      context.getConfiguration());
    determineClassTypes(conf);
    // configure global logging level for Giraph job
    initializeAndConfigureLogging();
    // init the metrics objects
    setupAndInitializeGiraphMetrics();
    // Do some task setup (possibly starting up a Zookeeper service)
    context.setStatus("setup: Initializing Zookeeper services.");
    locateZookeeperClasspath(zkPathList);
    serverPortList = conf.getZookeeperList();
    if (serverPortList == null && startZooKeeperManager()) {
      return; // ZK connect/startup failed
    }
    if (zkManager != null && zkManager.runsZooKeeper()) {
      if (LOG.isInfoEnabled()) {
        LOG.info("setup: Chosen to run ZooKeeper...");
      }
    }
    context.setStatus("setup: Connected to Zookeeper service " +
      serverPortList);
    this.graphFunctions = determineGraphFunctions(conf, zkManager);
    // Sometimes it takes a while to get multiple ZooKeeper servers up
    if (conf.getZooKeeperServerCount() > 1) {
      Thread.sleep(GiraphConstants.DEFAULT_ZOOKEEPER_INIT_LIMIT *
        GiraphConstants.DEFAULT_ZOOKEEPER_TICK_TIME);
    }
    int sessionMsecTimeout = conf.getZooKeeperSessionTimeout();
    try {
      instantiateBspService(serverPortList, sessionMsecTimeout);
    } catch (IOException e) {
      LOG.error("setup: Caught exception just before end of setup", e);
      if (zkManager != null) {
        zkManager.offlineZooKeeperServers(ZooKeeperManager.State.FAILED);
      }
      throw new RuntimeException(
        "setup: Offlining servers due to exception...", e);
    }
    context.setStatus(getGraphFunctions().toString() + " starting...");
  }

  /**
  * Perform the work assigned to this compute node for this job run.
  * 1) Run checkpoint per frequency policy.
  * 2) For every vertex on this mapper, run the compute() function
  * 3) Wait until all messaging is done.
  * 4) Check if all vertices are done.  If not goto 2).
  * 5) Dump output.
  */
  public void execute() throws IOException, InterruptedException {
    if (checkTaskState()) {
      return;
    }
    finishedSuperstepStats = serviceWorker.setup();
    if (collectInputSuperstepStats(finishedSuperstepStats)) {
      return;
    }
    WorkerAggregatorUsage aggregatorUsage =
      prepareAggregatorsAndGraphState();
    List<PartitionStats> partitionStatsList = new ArrayList<PartitionStats>();
    int numComputeThreads = conf.getNumComputeThreads();

    // main superstep processing loop
    do {
      final long superstep = serviceWorker.getSuperstep();
      GiraphTimerContext superstepTimerContext =
        getTimerForThisSuperstep(superstep);
      GraphState<I, V, E, M> graphState =
        new GraphState<I, V, E, M>(superstep,
            finishedSuperstepStats.getVertexCount(),
            finishedSuperstepStats.getEdgeCount(),
          context, this, null, aggregatorUsage);
      Collection<? extends PartitionOwner> masterAssignedPartitionOwners =
        serviceWorker.startSuperstep(graphState);
      if (LOG.isDebugEnabled()) {
        LOG.debug("execute: " + MemoryUtils.getRuntimeMemoryStats());
      }
      context.progress();
      serviceWorker.exchangeVertexPartitions(masterAssignedPartitionOwners);
      context.progress();
      graphState = checkSuperstepRestarted(
        aggregatorUsage, superstep, graphState);
      prepareForSuperstep(graphState);
      context.progress();
      MessageStoreByPartition<I, M> messageStore =
        serviceWorker.getServerData().getCurrentMessageStore();
      int numPartitions = serviceWorker.getPartitionStore().getNumPartitions();
      int numThreads = Math.min(numComputeThreads, numPartitions);
      if (LOG.isInfoEnabled()) {
        LOG.info("execute: " + numPartitions + " partitions to process with " +
          numThreads + " compute thread(s), originally " +
          numComputeThreads + " thread(s) on superstep " + superstep);
      }
      partitionStatsList.clear();
      // execute the current superstep
      if (numPartitions > 0) {
        processGraphPartitions(context, partitionStatsList, graphState,
          messageStore, numPartitions, numThreads);
      }
      finishedSuperstepStats = completeSuperstepAndCollectStats(
        partitionStatsList, superstepTimerContext, graphState);
      // END of superstep compute loop
    } while (!finishedSuperstepStats.allVerticesHalted());

    if (LOG.isInfoEnabled()) {
      LOG.info("execute: BSP application done (global vertices marked done)");
    }
    updateSuperstepGraphState(aggregatorUsage);
    postApplication();
  }

  /**
   * Handle post-application callbacks.
   */
  private void postApplication() throws IOException, InterruptedException {
    GiraphTimerContext postAppTimerContext = wcPostAppTimer.time();
    serviceWorker.getWorkerContext().postApplication();
    serviceWorker.getSuperstepOutput().postApplication();
    postAppTimerContext.stop();
    context.progress();

    for (WorkerObserver obs : serviceWorker.getWorkerObservers()) {
      obs.postApplication();
      context.progress();
    }
  }

  /**
   * Sets the "isMaster" flag for final output commit to happen on master.
   * @param im the boolean input to set isMaster. Applies to "pure YARN only"
   */
  public void setIsMaster(final boolean im) {
    this.isMaster = im;
  }

  /**
   * Get "isMaster" status flag -- we need to know if we're the master in the
   * "finally" block of our GiraphYarnTask#execute() to commit final job output.
   * @return true if this task IS the master.
   */
  public boolean isMaster() {
    return isMaster;
  }

  /**
   * Produce a reference to the "start" superstep timer for the current
   * superstep.
   * @param superstep the current superstep count
   * @return a GiraphTimerContext representing the "start" of the supestep
   */
  private GiraphTimerContext getTimerForThisSuperstep(long superstep) {
    GiraphMetrics.get().resetSuperstepMetrics(superstep);
    return superstepTimer.time();
  }

  /**
   * Utility to encapsulate Giraph metrics setup calls
   */
  private void setupAndInitializeGiraphMetrics() {
    GiraphMetrics.init(conf);
    GiraphMetrics.get().addSuperstepResetObserver(this);
    initJobMetrics();
    MemoryUtils.initMetrics();
  }

  /**
   * Instantiate and configure ZooKeeperManager for this job. This will
   * result in a Giraph-owned Zookeeper instance, a connection to an
   * existing quorum as specified in the job configuration, or task failure
   * @return true if this task should terminate
   */
  private boolean startZooKeeperManager()
    throws IOException, InterruptedException {
    zkManager = new ZooKeeperManager(context, conf);
    context.setStatus("setup: Setting up Zookeeper manager.");
    zkManager.setup();
    if (zkManager.computationDone()) {
      done = true;
      return true;
    }
    zkManager.onlineZooKeeperServers();
    serverPortList = zkManager.getZooKeeperServerPortString();
    return false;
  }

  /**
   * Utility to place a new, updated GraphState object into the serviceWorker.
   * @param aggregatorUsage handle to aggregation metadata
   */
  private void updateSuperstepGraphState(
    WorkerAggregatorUsage aggregatorUsage) {
    serviceWorker.getWorkerContext().setGraphState(
      new GraphState<I, V, E, M>(serviceWorker.getSuperstep(),
        finishedSuperstepStats.getVertexCount(),
          finishedSuperstepStats.getEdgeCount(), context, this, null,
          aggregatorUsage));
  }

  /**
   * Utility function for boilerplate updates and cleanup done at the
   * end of each superstep processing loop in the <code>execute</code> method.
   * @param partitionStatsList list of stas for each superstep to append to
   * @param superstepTimerContext for job metrics
   * @param graphState the graph state metadata
   * @return the collected stats at the close of the current superstep.
   */
  private FinishedSuperstepStats completeSuperstepAndCollectStats(
    List<PartitionStats> partitionStatsList,
    GiraphTimerContext superstepTimerContext,
    GraphState<I, V, E, M> graphState) {
    finishedSuperstepStats =
      serviceWorker.finishSuperstep(graphState, partitionStatsList);
    superstepTimerContext.stop();
    if (conf.metricsEnabled()) {
      GiraphMetrics.get().perSuperstep().printSummary(System.err);
    }
    return finishedSuperstepStats;
  }

  /**
   * Utility function to prepare various objects managing BSP superstep
   * operations for the next superstep.
   * @param graphState graph state metadata object
   */
  private void prepareForSuperstep(GraphState<I, V, E, M> graphState) {
    serviceWorker.prepareSuperstep();

    serviceWorker.getWorkerContext().setGraphState(graphState);
    GiraphTimerContext preSuperstepTimer = wcPreSuperstepTimer.time();
    serviceWorker.getWorkerContext().preSuperstep();
    preSuperstepTimer.stop();
    context.progress();

    for (WorkerObserver obs : serviceWorker.getWorkerObservers()) {
      obs.preSuperstep(graphState.getSuperstep());
      context.progress();
    }
  }

  /**
   * Prepare aggregators and worker context for superstep cycles.
   * @return aggregator metadata object
   */
  private WorkerAggregatorUsage prepareAggregatorsAndGraphState() {
    WorkerAggregatorUsage aggregatorUsage =
      serviceWorker.getAggregatorHandler();
    updateSuperstepGraphState(aggregatorUsage);
    workerContextPreApp();
    return aggregatorUsage;
  }

  /**
    * Get the worker function enum.
    *
    * @return an enum detailing the roles assigned to this
    *         compute node for this Giraph job.
    */
  public GraphFunctions getGraphFunctions() {
    return graphFunctions;
  }

  /**
   * Get master aggregator usage, a subset of the functionality
   *
   * @return Master aggregator usage interface
   */
  public final MasterAggregatorUsage getMasterAggregatorUsage() {
    return serviceMaster.getAggregatorHandler();
  }

  public final WorkerContext getWorkerContext() {
    return serviceWorker.getWorkerContext();
  }

 /**
   * Set the concrete, user-defined choices about generic methods
   * (validated earlier in GiraphRunner) into the Configuration.
   * @param conf the Configuration object for this job run.
   */
  public void determineClassTypes(Configuration conf) {
    ImmutableClassesGiraphConfiguration giraphConf =
        new ImmutableClassesGiraphConfiguration(conf);
    Class<? extends Vertex<I, V, E, M>> vertexClass =
        giraphConf.getVertexClass();
    List<Class<?>> classList = ReflectionUtils.<Vertex>getTypeArguments(
        Vertex.class, vertexClass);
    Type vertexIndexType = classList.get(0);
    Type vertexValueType = classList.get(1);
    Type edgeValueType = classList.get(2);
    Type messageValueType = classList.get(3);
    VERTEX_ID_CLASS.set(conf, (Class<WritableComparable>) vertexIndexType);
    VERTEX_VALUE_CLASS.set(conf, (Class<Writable>) vertexValueType);
    EDGE_VALUE_CLASS.set(conf, (Class<Writable>) edgeValueType);
    MESSAGE_VALUE_CLASS.set(conf, (Class<Writable>) messageValueType);
  }

  /**
   * Copied from JobConf to get the location of this jar.  Workaround for
   * things like Oozie map-reduce jobs. NOTE: Pure YARN profile cannot
   * make use of this, as the jars are unpacked at each container site.
   *
   * @param myClass Class to search the class loader path for to locate
   *        the relevant jar file
   * @return Location of the jar file containing myClass
   */
  private static String findContainingJar(Class<?> myClass) {
    ClassLoader loader = myClass.getClassLoader();
    String classFile =
        myClass.getName().replaceAll("\\.", "/") + ".class";
    try {
      for (Enumeration<?> itr = loader.getResources(classFile);
          itr.hasMoreElements();) {
        URL url = (URL) itr.nextElement();
        if ("jar".equals(url.getProtocol())) {
          String toReturn = url.getPath();
          if (toReturn.startsWith("file:")) {
            toReturn = toReturn.substring("file:".length());
          }
          toReturn = URLDecoder.decode(toReturn, "UTF-8");
          return toReturn.replaceAll("!.*$", "");
        }
      }
    } catch (IOException e) {
      throw new RuntimeException(e);
    }
    return null;
  }

  /**
   * Figure out what roles this BSP compute node should take on in the job.
   * Basic logic is as follows:
   * 1) If not split master, everyone does the everything and/or running
   *    ZooKeeper.
   * 2) If split master/worker, masters also run ZooKeeper
   *
   * 3) If split master/worker == true and <code>giraph.zkList</code> is set,
   *    the master will not instantiate a ZK instance, but will assume
   *    a quorum is already active on the cluster for Giraph to use.
   *
   * @param conf Configuration to use
   * @param zkManager ZooKeeper manager to help determine whether to run
   *        ZooKeeper.
   * @return Functions that this mapper should do.
   */
  private static GraphFunctions determineGraphFunctions(
      ImmutableClassesGiraphConfiguration conf,
      ZooKeeperManager zkManager) {
    boolean splitMasterWorker = conf.getSplitMasterWorker();
    int taskPartition = conf.getTaskPartition();
    boolean zkAlreadyProvided = conf.getZookeeperList() != null;
    GraphFunctions functions = GraphFunctions.UNKNOWN;
    // What functions should this mapper do?
    if (!splitMasterWorker) {
      if ((zkManager != null) && zkManager.runsZooKeeper()) {
        functions = GraphFunctions.ALL;
      } else {
        functions = GraphFunctions.ALL_EXCEPT_ZOOKEEPER;
      }
    } else {
      if (zkAlreadyProvided) {
        int masterCount = conf.getZooKeeperServerCount();
        if (taskPartition < masterCount) {
          functions = GraphFunctions.MASTER_ONLY;
        } else {
          functions = GraphFunctions.WORKER_ONLY;
        }
      } else {
        if ((zkManager != null) && zkManager.runsZooKeeper()) {
          functions = GraphFunctions.MASTER_ZOOKEEPER_ONLY;
        } else {
          functions = GraphFunctions.WORKER_ONLY;
        }
      }
    }
    return functions;
  }

  /**
   * Instantiate the appropriate BspService object (Master or Worker)
   * for this compute node.
   * @param serverPortList host:port list for connecting to ZK quorum
   * @param sessionMsecTimeout configurable session timeout
   */
  private void instantiateBspService(String serverPortList,
    int sessionMsecTimeout) throws IOException, InterruptedException {
    if (graphFunctions.isMaster()) {
      if (LOG.isInfoEnabled()) {
        LOG.info("setup: Starting up BspServiceMaster " +
          "(master thread)...");
      }
      serviceMaster = new BspServiceMaster<I, V, E, M>(
        serverPortList, sessionMsecTimeout, context, this);
      masterThread = new MasterThread<I, V, E, M>(serviceMaster, context);
      masterThread.start();
    }
    if (graphFunctions.isWorker()) {
      if (LOG.isInfoEnabled()) {
        LOG.info("setup: Starting up BspServiceWorker...");
      }
      serviceWorker = new BspServiceWorker<I, V, E, M>(
        serverPortList,
        sessionMsecTimeout,
        context,
        this);
      if (LOG.isInfoEnabled()) {
        LOG.info("setup: Registering health of this worker...");
      }
    }
  }

  /**
   * Attempt to locate the local copies of the ZK jar files, assuming
   * the underlying cluster framework has provided them for us.
   * @param fileClassPaths the path to the ZK jars on the local cluster.
   */
  private void locateZookeeperClasspath(Path[] fileClassPaths)
    throws IOException {
    if (!conf.getLocalTestMode()) {
      String zkClasspath = null;
      if (fileClassPaths == null) {
        if (LOG.isInfoEnabled()) {
          LOG.info("Distributed cache is empty. Assuming fatjar.");
        }
        String jarFile = context.getJar();
        if (jarFile == null) {
          jarFile = findContainingJar(getClass());
        }
        // Pure YARN profiles will use unpacked resources, so calls
        // to "findContainingJar()" in that context can return NULL!
        zkClasspath = null == jarFile ?
          "./*" : jarFile.replaceFirst("file:", "");
      } else {
        StringBuilder sb = new StringBuilder();
        sb.append(fileClassPaths[0]);

        for (int i = 1; i < fileClassPaths.length; i++) {
          sb.append(":");
          sb.append(fileClassPaths[i]);
        }
        zkClasspath = sb.toString();
      }

      if (LOG.isInfoEnabled()) {
        LOG.info("setup: classpath @ " + zkClasspath + " for job " +
          context.getJobName());
      }
      conf.setZooKeeperJar(zkClasspath);
    }
  }

  /**
   * Initialize the root logger and appender to the settings in conf.
   */
  private void initializeAndConfigureLogging() {
    // Set the log level
    String logLevel = conf.getLocalLevel();
    if (!Logger.getRootLogger().getLevel().equals(Level.toLevel(logLevel))) {
      Logger.getRootLogger().setLevel(Level.toLevel(logLevel));
      if (LOG.isInfoEnabled()) {
        LOG.info("setup: Set log level to " + logLevel);
      }
    } else {
      if (LOG.isInfoEnabled()) {
        LOG.info("setup: Log level remains at " + logLevel);
      }
    }
    // Sets pattern layout for all appenders
    if (conf.useLogThreadLayout()) {
      PatternLayout layout =
        new PatternLayout("%-7p %d [%t] %c %x - %m%n");
      Enumeration<Appender> appenderEnum =
        Logger.getRootLogger().getAllAppenders();
      while (appenderEnum.hasMoreElements()) {
        appenderEnum.nextElement().setLayout(layout);
      }
    }
  }

  /**
   * Initialize job-level metrics used by this class.
   */
  private void initJobMetrics() {
    GiraphMetricsRegistry jobMetrics = GiraphMetrics.get().perJobOptional();
    wcPreAppTimer = new GiraphTimer(jobMetrics, "worker-context-pre-app",
        TimeUnit.MILLISECONDS);
    wcPostAppTimer = new GiraphTimer(jobMetrics, "worker-context-post-app",
        TimeUnit.MILLISECONDS);
  }

  @Override
  public void newSuperstep(SuperstepMetricsRegistry superstepMetrics) {
    superstepTimer = new GiraphTimer(superstepMetrics,
        TIMER_SUPERSTEP_TIME, TimeUnit.MILLISECONDS);
    computeAll = new GiraphTimer(superstepMetrics,
        TIMER_COMPUTE_ALL, TimeUnit.MILLISECONDS);
    timeToFirstMessage = new GiraphTimer(superstepMetrics,
        TIMER_TIME_TO_FIRST_MSG, TimeUnit.MICROSECONDS);
    communicationTimer = new GiraphTimer(superstepMetrics,
        TIMER_COMMUNICATION_TIME, TimeUnit.MILLISECONDS);
    wcPreSuperstepTimer = new GiraphTimer(superstepMetrics,
        "worker-context-pre-superstep", TimeUnit.MILLISECONDS);
  }

  /**
   * Notification from Vertex that a message has been sent.
   */
  public void notifySentMessages() {
    // We are tracking the time between when the compute started and the first
    // message get sent. We use null to flag that we have already recorded it.
    GiraphTimerContext tmp = timeToFirstMessageTimerContext;
    if (tmp != null) {
      synchronized (timeToFirstMessage) {
        if (timeToFirstMessageTimerContext != null) {
          timeToFirstMessageTimerContext.stop();
          timeToFirstMessageTimerContext = null;
          communicationTimerContext = communicationTimer.time();
        }
      }
    }
  }

  /**
   * Notification of last message flushed. Comes when we finish the superstep
   * and are done waiting for all messages to send.
   */
  public void notifyFinishedCommunication() {
    GiraphTimerContext tmp = communicationTimerContext;
    if (tmp != null) {
      synchronized (communicationTimer) {
        if (communicationTimerContext != null) {
          communicationTimerContext.stop();
          communicationTimerContext = null;
        }
      }
    }
  }

  /**
   * Process graph data partitions active in this superstep.
   * @param context handle to the underlying cluster framework
   * @param partitionStatsList to pick up this superstep's processing stats
   * @param graphState the BSP graph state
   * @param messageStore the messages to be processed in this superstep
   * @param numPartitions the number of data partitions (vertices) to process
   * @param numThreads number of concurrent threads to do processing
   */
  private void processGraphPartitions(final Mapper<?, ?, ?, ?>.Context context,
      List<PartitionStats> partitionStatsList,
      final GraphState<I, V, E, M> graphState,
      final MessageStoreByPartition<I, M> messageStore,
      int numPartitions,
      int numThreads) {
    final BlockingQueue<Integer> computePartitionIdQueue =
      new ArrayBlockingQueue<Integer>(numPartitions);
    for (Integer partitionId :
      serviceWorker.getPartitionStore().getPartitionIds()) {
      computePartitionIdQueue.add(partitionId);
    }

    GiraphTimerContext computeAllTimerContext = computeAll.time();
    timeToFirstMessageTimerContext = timeToFirstMessage.time();

    CallableFactory<Collection<PartitionStats>> callableFactory =
        new CallableFactory<Collection<PartitionStats>>() {
          @Override
          public Callable<Collection<PartitionStats>> newCallable(
              int callableId) {
            return new ComputeCallable<I, V, E, M>(
                context,
                graphState,
                messageStore,
                computePartitionIdQueue,
                conf,
                serviceWorker);
          }
        };
    List<Collection<PartitionStats>> results =
        ProgressableUtils.getResultsWithNCallables(callableFactory,
            numThreads, "compute-%d", context);
    for (Collection<PartitionStats> result : results) {
      partitionStatsList.addAll(result);
    }

    computeAllTimerContext.stop();
  }

  /**
   * Handle the event that this superstep is a restart of a failed one.
   * @param aggregatorUsage aggregator metadata
   * @param superstep current superstep
   * @param graphState the BSP graph state
   * @return the graph state, updated if this is a restart superstep
   */
  private GraphState<I, V, E, M> checkSuperstepRestarted(
    WorkerAggregatorUsage aggregatorUsage, long superstep,
    GraphState<I, V, E, M> graphState) throws IOException {
    // Might need to restart from another superstep
    // (manually or automatic), or store a checkpoint
    if (serviceWorker.getRestartedSuperstep() == superstep) {
      if (LOG.isInfoEnabled()) {
        LOG.info("execute: Loading from checkpoint " + superstep);
      }
      VertexEdgeCount vertexEdgeCount = serviceWorker.loadCheckpoint(
        serviceWorker.getRestartedSuperstep());
      finishedSuperstepStats = new FinishedSuperstepStats(0, false,
          vertexEdgeCount.getVertexCount(), vertexEdgeCount.getEdgeCount(),
          false);
      graphState = new GraphState<I, V, E, M>(superstep,
          finishedSuperstepStats.getVertexCount(),
          finishedSuperstepStats.getEdgeCount(),
          context, this, null, aggregatorUsage);
    } else if (serviceWorker.checkpointFrequencyMet(superstep)) {
      serviceWorker.storeCheckpoint();
    }
    return graphState;
  }

  /**
   * Attempt to collect the final statistics on the graph data
   * processed in this superstep by this compute node
   * @param inputSuperstepStats the final graph data stats object for the
   *                            input superstep
   * @return true if the graph data has no vertices (error?) and
   *         this node should terminate
   */
  private boolean collectInputSuperstepStats(
    FinishedSuperstepStats inputSuperstepStats) {
    if (inputSuperstepStats.getVertexCount() == 0 &&
        !inputSuperstepStats.mustLoadCheckpoint()) {
      LOG.warn("map: No vertices in the graph, exiting.");
      return true;
    }
    return false;
  }

  /**
   * Did the state of this compute node change?
   * @return true if the processing of supersteps should terminate.
   */
  private boolean checkTaskState() {
    if (done) {
      return true;
    }
    GiraphMetrics.get().resetSuperstepMetrics(BspService.INPUT_SUPERSTEP);
    if (graphFunctions.isNotAWorker()) {
      if (LOG.isInfoEnabled()) {
        LOG.info("map: No need to do anything when not a worker");
      }
      return true;
    }
    if (alreadyRun) {
      throw new RuntimeException("map: In BSP, map should have only been" +
        " run exactly once, (already run)");
    }
    alreadyRun = true;
    return false;
  }

  /**
   * Call to the WorkerContext before application begins.
   */
  private void workerContextPreApp() {
    GiraphTimerContext preAppTimerContext = wcPreAppTimer.time();
    try {
      serviceWorker.getWorkerContext().preApplication();
    } catch (InstantiationException e) {
      LOG.fatal("execute: preApplication failed in instantiation", e);
      throw new RuntimeException(
          "execute: preApplication failed in instantiation", e);
    } catch (IllegalAccessException e) {
      LOG.fatal("execute: preApplication failed in access", e);
      throw new RuntimeException(
          "execute: preApplication failed in access", e);
    }
    preAppTimerContext.stop();
    context.progress();

    for (WorkerObserver obs : serviceWorker.getWorkerObservers()) {
      obs.preApplication();
      context.progress();
    }
  }

  /**
   * Called by owner of this GraphTaskManager object on each compute node
   */
  public void cleanup()
    throws IOException, InterruptedException {
    if (LOG.isInfoEnabled()) {
      LOG.info("cleanup: Starting for " + getGraphFunctions());
    }
    if (done) {
      return;
    }

    if (serviceWorker != null) {
      serviceWorker.cleanup(finishedSuperstepStats);
    }
    try {
      if (masterThread != null) {
        masterThread.join();
      }
    } catch (InterruptedException e) {
      // cleanup phase -- just log the error
      LOG.error("cleanup: Master thread couldn't join");
    }
    if (zkManager != null) {
      zkManager.offlineZooKeeperServers(ZooKeeperManager.State.FINISHED);
    }
  }

  /**
   * Cleanup a ZooKeeper instance managed by this
   * GiraphWorker upon job run failure.
   */
  public void zooKeeperCleanup() {
    if (graphFunctions.isZooKeeper()) {
      // ZooKeeper may have had an issue
      if (zkManager != null) {
        zkManager.logZooKeeperOutput(Level.WARN);
      }
    }
  }

  /**
   * Cleanup all of Giraph's framework-agnostic resources
   * regardless of which type of cluster Giraph is running on.
   */
  public void workerFailureCleanup() {
    try {
      if (graphFunctions.isWorker()) {
        serviceWorker.failureCleanup();
      }
    // Checkstyle exception due to needing to get the original
    // exception on failure
    // CHECKSTYLE: stop IllegalCatch
    } catch (RuntimeException e1) {
    // CHECKSTYLE: resume IllegalCatch
      LOG.error("run: Worker failure failed on another RuntimeException, " +
          "original expection will be rethrown", e1);
    }
  }
}
