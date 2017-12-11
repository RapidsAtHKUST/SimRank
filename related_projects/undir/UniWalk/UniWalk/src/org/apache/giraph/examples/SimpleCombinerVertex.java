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

package org.apache.giraph.examples;

import org.apache.giraph.graph.Vertex;
import org.apache.hadoop.io.FloatWritable;
import org.apache.hadoop.io.IntWritable;
import org.apache.hadoop.io.LongWritable;
import org.apache.log4j.Logger;

/**
 * Test whether messages can go through a combiner.
 */
public class SimpleCombinerVertex extends
    Vertex<LongWritable, IntWritable, FloatWritable, IntWritable> {
  /** Class logger */
  private static Logger LOG = Logger.getLogger(SimpleCombinerVertex.class);

  @Override
  public void compute(Iterable<IntWritable> messages) {
    if (getId().equals(new LongWritable(2))) {
      sendMessage(new LongWritable(1), new IntWritable(101));
      sendMessage(new LongWritable(1), new IntWritable(102));
      sendMessage(new LongWritable(1), new IntWritable(103));
    }
    if (!getId().equals(new LongWritable(1))) {
      voteToHalt();
    } else {
      // Check the messages
      int sum = 0;
      int num = 0;
      for (IntWritable message : messages) {
        sum += message.get();
        num++;
      }
      LOG.info("TestCombinerVertex: Received a sum of " + sum +
          " (should have 306 with a single message value)");

      if (num == 1 && sum == 306) {
        voteToHalt();
      }
    }
    if (getSuperstep() > 3) {
      throw new IllegalStateException(
          "TestCombinerVertex: Vertex 1 failed to receive " +
          "messages in time");
    }
  }
}
