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

import org.apache.giraph.conf.GiraphConfiguration;
import org.apache.giraph.conf.ImmutableClassesGiraphConfiguration;
import org.apache.giraph.edge.Edge;
import org.apache.giraph.edge.EdgeFactory;
import org.apache.giraph.graph.Vertex;
import org.apache.hadoop.io.Writable;
import org.apache.hadoop.io.WritableComparable;

import com.google.common.base.Objects;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;

/**
 * TestGraph class for in-memory testing.
 *
 * @param <I> Vertex index type
 * @param <V> Vertex type
 * @param <E> Edge type
 * @param <M> Message type
 */
public class TestGraph<I extends WritableComparable,
                       V extends Writable,
                       E extends Writable,
                       M extends Writable>
                       implements Iterable<Vertex<I, V, E, M>> {
  /** The vertex values */
  private final HashMap<I, Vertex<I, V, E, M>> vertices = Maps.newHashMap();
  /** The configuration */
  private ImmutableClassesGiraphConfiguration<I, V, E, M> conf;

  /**
   * Constructor requiring classes
   *
   * @param conf Should have vertex and edge classes set.
   */
  public TestGraph(GiraphConfiguration conf) {
    this.conf = new ImmutableClassesGiraphConfiguration(conf);
  }

  public HashMap<I, Vertex<I, V, E, M>> getVertices() {
    return vertices;
  }

  /**
   * Clear all data
   *
   */
  public void clear() {
    vertices.clear();
  }

  /**
   * Add vertex with given ID
   *
   * @param id the index
   * @param value the value
   * @param edges all edges
   * @return this
   */
  public TestGraph<I, V, E, M> addVertex(I id, V value,
                                         Entry<I, E>... edges) {
    Vertex<I, V, E, M> v = makeVertex(id, value, edges);
    vertices.put(id, v);
    return this;
  }

  /**
   * Add an edge to an existing vertex
   *
   * @param vertexId Edge origin
   * @param edgePair The edge
   * @return this
   */
  public TestGraph<I, V, E, M> addEdge(I vertexId, Entry<I, E> edgePair) {
    if (!vertices.containsKey(vertexId)) {
      Vertex<I, V, E, M> v = conf.createVertex();
      v.initialize(vertexId, conf.createVertexValue());
      vertices.put(vertexId, v);
    }
    vertices.get(vertexId)
      .addEdge((Edge<I, E>) EdgeFactory.create(edgePair.getKey(),
                                               edgePair.getValue()));
    return this;
  }

  /**
   * Add an edge to an existing vertex
   *
   * @param vertexId Edge origin
   * @param toVertex Edge destination
   * @param edgeValue Edge value
   * @return this
   */
  public TestGraph<I, V, E, M> addEdge(I vertexId, I toVertex, E edgeValue) {
    if (!vertices.containsKey(vertexId)) {
      Vertex<I, V, E, M> v = conf.createVertex();
      v.initialize(vertexId, conf.createVertexValue());
      vertices.put(vertexId, v);
    }
    vertices.get(vertexId)
      .addEdge((Edge<I, E>) EdgeFactory.create(toVertex, edgeValue));
    return this;
  }
  /**
   * An iterator over the ids
   *
   * @return the iterator
   */
  public Iterator<I> idIterator() {
    return vertices.keySet().iterator();
  }

  /**
   * An iterator over the vertices
   *
   * @return the iterator
   */
  public Iterator<Vertex<I, V, E, M>> iterator() {
    return vertices.values().iterator();
  }

  /**
   * Return a given vertex
   *
   * @param id the id
   * @return the value
   */
  public Vertex<I, V, E, M> getVertex(I id) {
    return vertices.get(id);
  }

  /**
   * Create edges for given ids
   *
   * @param destEdgess ids to which the edges link
   * @return an iterable containing the edges
   */
  protected Iterable<Edge<I, E>>
  createEdges(Entry<I, E>... destEdgess) {
    List<Edge<I, E>> edgesList = Lists.newArrayList();
    for (Entry<I, E> e: destEdgess) {
      edgesList.add((Edge<I, E>) EdgeFactory.create(e.getKey(), e.getValue()));
    }
    return edgesList;
  }

  /**
   * Create a vertex
   *
   * @param id the id of the vertex
   * @param value the vertex value
   * @param edges edges to other vertices
   * @return a new vertex
   */
  protected Vertex<I, V, E, M> makeVertex(I id, V value,
      Entry<I, E>... edges) {
    @SuppressWarnings("unchecked")
    Vertex<I, V, E, M> vertex = conf.createVertex();
    vertex.initialize(id, value, createEdges(edges));
    return vertex;
  }

  @Override
  public String toString() {
    return Objects.toStringHelper(this).add("vertices", vertices).toString();
  }
}
