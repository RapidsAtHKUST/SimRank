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

import java.util.List;
import org.apache.giraph.worker.WorkerInfo;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;

/**
 * Basic service interface shared by both {@link CentralizedServiceMaster} and
 * {@link CentralizedServiceWorker}.
 *
 * @param <I> Vertex id
 * @param <V> Vertex value
 * @param <E> Edge value
 * @param <M> Message data
 */
@SuppressWarnings("rawtypes")
public interface CentralizedService<I extends WritableComparable,
    V extends Writable, E extends Writable, M extends Writable> {


  /**
   * Get the current global superstep of the application to work on.
   *
   * @return global superstep (begins at INPUT_SUPERSTEP)
   */
  long getSuperstep();

  /**
   * Get the restarted superstep
   *
   * @return -1 if not manually restarted, otherwise the superstep id
   */
  long getRestartedSuperstep();

  /**
   * Given a superstep, should it be checkpointed based on the
   * checkpoint frequency?
   *
   * @param superstep superstep to check against frequency
   * @return true if checkpoint frequency met or superstep is 1.
   */
  boolean checkpointFrequencyMet(long superstep);

  /**
   * Get list of workers
   *
   * @return List of workers
   */
  List<WorkerInfo> getWorkerInfoList();
}
