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

package org.apache.giraph.partition;

import org.apache.giraph.conf.GiraphConstants;
import org.apache.giraph.conf.ImmutableClassesGiraphConfiguration;
import org.apache.hadoop.io.LongWritable;
import org.apache.hadoop.io.Writable;

/**
 * Factory for simple range-based partitioners based on long vertex ids.
 * Workers are assigned equal-sized ranges of partitions,
 * and partitions are assigned equal-sized ranges of vertices.
 *
 * @param <V> Vertex value type
 * @param <E> Edge value type
 * @param <M> Message data type
 */
public class SimpleLongRangePartitionerFactory<V extends Writable,
    E extends Writable, M extends Writable>
    implements GraphPartitionerFactory<LongWritable, V, E, M> {
  /** Configuration. */
  private ImmutableClassesGiraphConfiguration conf;
  /** Vertex key space size. */
  private long keySpaceSize;

  @Override
  public MasterGraphPartitioner<LongWritable, V, E, M>
  createMasterGraphPartitioner() {
    return new SimpleRangeMasterPartitioner<LongWritable, V, E, M>(conf);
  }

  @Override
  public WorkerGraphPartitioner<LongWritable, V, E, M>
  createWorkerGraphPartitioner() {
    return new SimpleRangeWorkerPartitioner<LongWritable, V, E, M>(
        keySpaceSize) {
      @Override
      protected long vertexKeyFromId(LongWritable id) {
        // The modulo is just a safeguard in case keySpaceSize is incorrect.
        return id.get() % keySpaceSize;
      }
    };
  }

  @Override
  public void setConf(ImmutableClassesGiraphConfiguration conf) {
    this.conf = conf;
    keySpaceSize = conf.getLong(GiraphConstants.PARTITION_VERTEX_KEY_SPACE_SIZE,
        -1);
    if (keySpaceSize == -1) {
      throw new IllegalStateException("Need to specify " + GiraphConstants
          .PARTITION_VERTEX_KEY_SPACE_SIZE + " when using " +
          "SimpleRangePartitioner");
    }
  }

  @Override
  public ImmutableClassesGiraphConfiguration getConf() {
    return conf;
  }
}
