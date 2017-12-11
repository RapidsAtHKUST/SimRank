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

package org.apache.giraph.comm;

import org.apache.giraph.graph.GraphState;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;

import java.io.Closeable;
import java.net.InetSocketAddress;

/**
 * Interface for message communication server.
 *
 * @param <I> Vertex id
 * @param <V> Vertex value
 * @param <E> Edge value
 * @param <M> Message data
 */
@SuppressWarnings("rawtypes")
public interface WorkerServer<I extends WritableComparable,
    V extends Writable, E extends Writable, M extends Writable>
    extends Closeable {
  /**
   * Get server address
   *
   * @return Address used by this server
   */
  InetSocketAddress getMyAddress();

  /**
   * Prepare incoming messages for computation, and resolve mutation requests.
   *
   * @param graphState Current graph state
   */
  void prepareSuperstep(GraphState<I, V, E, M> graphState);

  /**
   * Get server data
   *
   * @return Server data
   */
  ServerData<I, V, E, M> getServerData();

  /**
   * Shuts down.
   */
  void close();
}
