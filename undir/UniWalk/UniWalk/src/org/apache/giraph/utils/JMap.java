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

package org.apache.giraph.utils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.lang.management.ManagementFactory;
import java.util.Date;

/**
 * Helper to run jmap and print the output
 */
public class JMap {
  /** The command to run */
  public static final String CMD = "jmap ";
  /** Arguments to pass in to command */
  public static final String ARGS = " -histo ";

  /** Do not construct */
  protected JMap() { }

  /**
   * Get the process ID of the current running process
   *
   * @return Integer process ID
   */
  public static int getProcessId() {
    String processId = ManagementFactory.getRuntimeMXBean().getName();
    if (processId.contains("@")) {
      processId = processId.substring(0, processId.indexOf("@"));
    }
    return Integer.parseInt(processId);
  }

  /**
   * Run jmap, print numLines of output from it to stderr.
   *
   * @param numLines Number of lines to print
   */
  public static void heapHistogramDump(int numLines) {
    heapHistogramDump(numLines, System.err);
  }

  /**
   * Run jmap, print numLines of output from it to stream passed in.
   *
   * @param numLines Number of lines to print
   * @param printStream Stream to print to
   */
  public static void heapHistogramDump(int numLines, PrintStream printStream) {
    try {
      Process p = Runtime.getRuntime().exec(CMD + ARGS + getProcessId());
      BufferedReader in = new BufferedReader(
          new InputStreamReader(p.getInputStream()));
      printStream.println("JMap histo dump at " + new Date());
      String line = in.readLine();
      for (int i = 0; i < numLines && line != null; ++i) {
        printStream.println("--\t" + line);
        line = in.readLine();
      }
      in.close();
    } catch (IOException e) {
      e.printStackTrace();
    }
  }
}
