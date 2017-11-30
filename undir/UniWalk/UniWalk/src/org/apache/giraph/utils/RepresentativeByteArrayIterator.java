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

import java.io.IOException;
import org.apache.giraph.conf.ImmutableClassesGiraphConfiguration;
import org.apache.hadoop.io.Writable;

/**
 * The objects provided by this iterator have lifetimes only until next() is
 * called.  In that sense, the object provided is only a representative object.
 *
 * @param <T> Type that extends Writable that will be iterated
 */
public abstract class RepresentativeByteArrayIterator<T extends
    Writable> extends ByteArrayIterator<T> {
  /** Representative writable */
  private final T representativeWritable = createWritable();

  /**
   * Constructor
   *
   * @param configuration Configuration
   * @param buf buffer to read from
   * @param off Offset into the buffer to start from
   * @param length Length of the buffer
   */
  public RepresentativeByteArrayIterator(
      ImmutableClassesGiraphConfiguration configuration,
      byte[] buf, int off, int length) {
    super(configuration, buf, off, length);
  }

  @Override
  public T next() {
    try {
      representativeWritable.readFields(extendedDataInput);
    } catch (IOException e) {
      throw new IllegalStateException("next: readFields got IOException", e);
    }
    return representativeWritable;
  }
}
