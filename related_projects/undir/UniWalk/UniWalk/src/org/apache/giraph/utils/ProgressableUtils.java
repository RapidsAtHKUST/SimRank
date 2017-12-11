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

import org.apache.hadoop.util.Progressable;
import org.apache.log4j.Logger;
import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.group.ChannelGroupFuture;

import com.google.common.collect.Lists;
import com.google.common.util.concurrent.ThreadFactoryBuilder;

import java.util.List;
import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/** Functions for waiting on some events to happen while reporting progress */
public class ProgressableUtils {
  /** Class logger */
  private static final Logger LOG =
      Logger.getLogger(ProgressableUtils.class);
  /** Msecs to refresh the progress meter (one minute) */
  private static final int MSEC_PERIOD = 60 * 1000;

  /** Do not instantiate. */
  private ProgressableUtils() {
  }

  /**
   * Wait for executor tasks to terminate, while periodically reporting
   * progress.
   *
   * @param executor     Executor which we are waiting for
   * @param progressable Progressable for reporting progress (Job context)
   */
  public static void awaitExecutorTermination(ExecutorService executor,
      Progressable progressable) {
    waitForever(new ExecutorServiceWaitable(executor), progressable);
  }

  /**
   * Wait for the result of the future to be ready, while periodically
   * reporting progress.
   *
   * @param <T>          Type of the return value of the future
   * @param future       Future
   * @param progressable Progressable for reporting progress (Job context)
   * @return Computed result of the future.
   */
  public static <T> T getFutureResult(Future<T> future,
      Progressable progressable) {
    return waitForever(new FutureWaitable<T>(future), progressable);
  }

  /**
   * Wait for {@link ChannelGroupFuture} to finish, while periodically
   * reporting progress.
   *
   * @param future       ChannelGroupFuture
   * @param progressable Progressable for reporting progress (Job context)
   */
  public static void awaitChannelGroupFuture(ChannelGroupFuture future,
      Progressable progressable) {
    waitForever(new ChannelGroupFutureWaitable(future), progressable);
  }

  /**
   * Wait for {@link ChannelFuture} to finish, while periodically
   * reporting progress.
   *
   * @param future       ChannelFuture
   * @param progressable Progressable for reporting progress (Job context)
   */
  public static void awaitChannelFuture(ChannelFuture future,
      Progressable progressable) {
    waitForever(new ChannelFutureWaitable(future), progressable);
  }

  /**
   * Wait forever for waitable to finish. Periodically reports progress.
   *
   * @param waitable Waitable which we wait for
   * @param progressable Progressable for reporting progress (Job context)
   * @param <T> Result type
   * @return Result of waitable
   */
  private static <T> T waitForever(Waitable<T> waitable,
      Progressable progressable) {
    while (true) {
      waitFor(waitable, progressable, MSEC_PERIOD);
      if (waitable.isFinished()) {
        try {
          return waitable.getResult();
        } catch (ExecutionException e) {
          throw new IllegalStateException("waitForever: " +
              "ExecutionException occurred while waiting for " + waitable, e);
        } catch (InterruptedException e) {
          throw new IllegalStateException("waitForever: " +
              "InterruptedException occurred while waiting for " + waitable, e);
        }
      }
    }
  }

  /**
   *  Wait for desired number of milliseconds for waitable to finish.
   *  Periodically reports progress.
   *
   * @param waitable Waitable which we wait for
   * @param progressable Progressable for reporting progress (Job context)
   * @param msecs Number of milliseconds to wait for
   * @param <T> Result type
   * @return Result of waitable
   */
  private static <T> T waitFor(Waitable<T> waitable, Progressable progressable,
      int msecs) {
    long timeoutTimeMsecs = System.currentTimeMillis() + msecs;
    int currentWaitMsecs;
    while (true) {
      currentWaitMsecs = Math.min(msecs, MSEC_PERIOD);
      try {
        waitable.waitFor(currentWaitMsecs);
        if (waitable.isFinished()) {
          return waitable.getResult();
        }
      } catch (InterruptedException e) {
        throw new IllegalStateException("waitFor: " +
            "InterruptedException occurred while waiting for " + waitable, e);
      } catch (ExecutionException e) {
        throw new IllegalStateException("waitFor: " +
            "ExecutionException occurred while waiting for " + waitable, e);
      }
      if (LOG.isInfoEnabled()) {
        LOG.info("waitFor: Waiting for " + waitable);
      }
      if (System.currentTimeMillis() >= timeoutTimeMsecs) {
        return waitable.getTimeoutResult();
      }
      progressable.progress();
      msecs = Math.max(0, msecs - currentWaitMsecs);
    }
  }

  /**
   * Create {#link numThreads} callables from {#link callableFactory},
   * execute them and gather results.
   *
   * @param callableFactory Factory for Callables
   * @param numThreads Number of threads to use
   * @param threadNameFormat Format for thread name
   * @param progressable Progressable for reporting progress
   * @param <R> Type of Callable's results
   * @return List of results from Callables
   */
  public static <R> List<R> getResultsWithNCallables(
      CallableFactory<R> callableFactory, int numThreads,
      String threadNameFormat, Progressable progressable) {
    ExecutorService executorService =
        Executors.newFixedThreadPool(numThreads,
            new ThreadFactoryBuilder().setNameFormat(threadNameFormat).build());
    List<Future<R>> futures = Lists.newArrayListWithCapacity(numThreads);
    for (int i = 0; i < numThreads; i++) {
      Callable<R> callable = callableFactory.newCallable(i);
      Future<R> future = executorService.submit(
          new LogStacktraceCallable<R>(callable));
      futures.add(future);
    }
    executorService.shutdown();
    List<R> futureResults = Lists.newArrayListWithCapacity(numThreads);
    for (Future<R> future : futures) {
      R result = ProgressableUtils.getFutureResult(future, progressable);
      futureResults.add(result);
    }
    return futureResults;
  }

  /**
   * Interface for waiting on a result from some operation.
   *
   * @param <T> Result type.
   */
  private interface Waitable<T> {
    /**
     * Wait for desired number of milliseconds for waitable to finish.
     *
     * @param msecs Number of milliseconds to wait.
     */
    void waitFor(int msecs) throws InterruptedException, ExecutionException;

    /**
     * Check if waitable is finished.
     *
     * @return True iff waitable finished.
     */
    boolean isFinished();

    /**
     * Get result of waitable. Call after isFinished() returns true.
     *
     * @return Result of waitable.
     */
    T getResult() throws ExecutionException, InterruptedException;

    /**
     * Get the result which we want to return in case of timeout.
     *
     * @return Timeout result.
     */
    T getTimeoutResult();
  }

  /**
   * abstract class for waitables which don't have the result.
   */
  private abstract static class WaitableWithoutResult
      implements Waitable<Void> {
    @Override
    public Void getResult() throws ExecutionException, InterruptedException {
      return null;
    }

    @Override
    public Void getTimeoutResult() {
      return null;
    }
  }

  /**
   * {@link Waitable} for waiting on a result of a {@link Future}.
   *
   * @param <T> Future result type
   */
  private static class FutureWaitable<T> implements Waitable<T> {
    /** Future which we want to wait for */
    private final Future<T> future;

    /**
     * Constructor
     *
     * @param future Future which we want to wait for
     */
    public FutureWaitable(Future<T> future) {
      this.future = future;
    }

    @Override
    public void waitFor(int msecs) throws InterruptedException,
        ExecutionException {
      try {
        future.get(msecs, TimeUnit.MILLISECONDS);
      } catch (TimeoutException e) {
        if (LOG.isInfoEnabled()) {
          LOG.info("waitFor: Future result not ready yet " + future);
        }
      }
    }

    @Override
    public boolean isFinished() {
      return future.isDone();
    }

    @Override
    public T getResult() throws ExecutionException, InterruptedException {
      return future.get();
    }

    @Override
    public T getTimeoutResult() {
      return null;
    }
  }

  /**
   * {@link Waitable} for waiting on an {@link ExecutorService} to terminate.
   */
  private static class ExecutorServiceWaitable extends WaitableWithoutResult {
    /** ExecutorService which we want to wait for */
    private final ExecutorService executorService;

    /**
     * Constructor
     *
     * @param executorService ExecutorService which we want to wait for
     */
    public ExecutorServiceWaitable(ExecutorService executorService) {
      this.executorService = executorService;
    }

    @Override
    public void waitFor(int msecs) throws InterruptedException {
      executorService.awaitTermination(msecs, TimeUnit.MILLISECONDS);
    }

    @Override
    public boolean isFinished() {
      return executorService.isTerminated();
    }
  }

  /**
   * {@link Waitable} for waiting on a {@link ChannelGroupFuture} to
   * terminate.
   */
  private static class ChannelGroupFutureWaitable extends
      WaitableWithoutResult {
    /** ChannelGroupFuture which we want to wait for */
    private final ChannelGroupFuture future;

    /**
     * Constructor
     *
     * @param future ChannelGroupFuture which we want to wait for
     */
    public ChannelGroupFutureWaitable(ChannelGroupFuture future) {
      this.future = future;
    }

    @Override
    public void waitFor(int msecs) throws InterruptedException {
      future.await(msecs, TimeUnit.MILLISECONDS);
    }

    @Override
    public boolean isFinished() {
      return future.isDone();
    }
  }

  /**
   * {@link Waitable} for waiting on a {@link ChannelFuture} to
   * terminate.
   */
  private static class ChannelFutureWaitable extends WaitableWithoutResult {
    /** ChannelGroupFuture which we want to wait for */
    private final ChannelFuture future;

    /**
     * Constructor
     *
     * @param future ChannelFuture which we want to wait for
     */
    public ChannelFutureWaitable(ChannelFuture future) {
      this.future = future;
    }

    @Override
    public void waitFor(int msecs) throws InterruptedException {
      future.await(msecs, TimeUnit.MILLISECONDS);
    }

    @Override
    public boolean isFinished() {
      return future.isDone();
    }
  }
}
