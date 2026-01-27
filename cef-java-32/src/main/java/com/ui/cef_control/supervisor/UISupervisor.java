package com.ui.cef_control.supervisor;

import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

/**
 * Supervisor for UI process lifecycle.
 * Phase 5: Handles UI startup failures, crashes, and command queueing.
 * Non-blocking, deterministic, treats UI as black box.
 * Responsibilities:
 * - Start the {@link UIProcess} and notify registered {@link SupervisorListener}s.
 * - Retry startup according to the configured {@link RetryPolicy}.
 * - Maintain a queued set of commands while the UI is not running and deliver them
 *   once the UI becomes available.
 * - Track UI running state and the last intent command.
 */
public class UISupervisor
{

	private final UIProcess uiProcess;
	private final RetryPolicy retryPolicy;
	private final List<SupervisorListener> listeners;
	private final Queue<String> commandQueue;
	private boolean uiRunning;
	private String lastIntent;

	/**
	 * Create a supervisor for the given UI process and retry policy.
	 * @param uiProcess non-null UI process implementation
	 * @param retryPolicy non-null retry policy
	 * @throws IllegalArgumentException if {@code uiProcess} or {@code retryPolicy} is null
	 */
	public UISupervisor(UIProcess uiProcess, RetryPolicy retryPolicy)
	{
		if (uiProcess == null)
		{
			throw new IllegalArgumentException("UIProcess cannot be null");
		}
		if (retryPolicy == null)
		{
			throw new IllegalArgumentException("RetryPolicy cannot be null");
		}

		this.uiProcess = uiProcess;
		this.retryPolicy = retryPolicy;
		this.listeners = new ArrayList<>();
		this.commandQueue = new LinkedList<>();
		this.uiRunning = false;
		this.lastIntent = null;
	}

	/**
	 * Attempt to start the UI process. This method will loop attempting to start
	 * the UI and will consult {@link RetryPolicy#shouldRetry} after failures.
	 * On successful start, registered listeners are notified via {@link SupervisorListener#onUiAvailable()}
	 * and the method returns. If retries are exhausted, listeners are notified via
	 * {@link SupervisorListener#onUiUnavailable()} and the method returns.
	 * This implementation notifies listeners of individual crashes using {@link SupervisorListener#onUiCrashed(Throwable)}.
	 */
	public void start()
	{
		int attempt = 1;

		while (true)
		{
			try
			{
				uiProcess.start();

				// Snapshot ONCE for this lifecycle event
				List<SupervisorListener> snapshot = new ArrayList<>(listeners);
				for (SupervisorListener l : snapshot)
				{
					l.onUiAvailable();
				}
				return; // success

			}
			catch (Exception e)
			{

				// Snapshot ONCE for this failure event
				List<SupervisorListener> snapshot = new ArrayList<>(listeners);
				for (SupervisorListener l : snapshot)
				{
					l.onUiCrashed(e);
				}

				if (!retryPolicy.shouldRetry(attempt, e))
				{

					// Snapshot ONCE for terminal event
					snapshot = new ArrayList<>(listeners);
					for (SupervisorListener l : snapshot)
					{
						l.onUiUnavailable();
					}
					return; // retries exhausted
				}

				attempt++;
			}
		}
	}

	/**
	 * Stop the UI process if it is running and clear any queued commands.
	 * This method sets the UI running flag to false.
	 */
	public void stop()
	{
		if (uiRunning)
		{
			uiProcess.stop();
			uiRunning = false;
		}
		commandQueue.clear();
	}

	/**
	 * Send a command to the UI. If the UI is not running the command is queued.
	 * Null or empty commands are ignored.
	 * This method updates the last intent to the provided command.
	 * @param command the command to send or queue
	 */
	public void sendCommand(String command)
	{
		if (command == null || command.isEmpty())
		{
			return;
		}

		lastIntent = command;

		if (uiRunning)
		{
			uiProcess.sendCommand(command);
		}
		else
		{
			commandQueue.offer(command);
		}
	}

	/**
	 * Notify the supervisor that the UI has crashed. This updates internal state
	 * and forwards the crash notification to registered listeners.
	 * @param error the error that caused the crash
	 */
	public void notifyUiCrash(Throwable error)
	{
		uiRunning = false;
		notifyUiCrashed(error);
	}

	/**
	 * Query whether the UI is currently considered running.
	 * @return {@code true} if the UI is marked as running
	 */
	public boolean isUiRunning()
	{
		return uiRunning;
	}

	/**
	 * Return the number of commands currently queued for delivery.
	 * @return number of queued commands
	 */
	public int getQueuedCommandCount()
	{
		return commandQueue.size();
	}

	/**
	 * Add a supervisor listener. Null listeners are ignored. Duplicate listeners
	 * are not added.
	 * @param listener listener to register
	 */
	public void addListener(SupervisorListener listener)
	{
		if (listener != null && !listeners.contains(listener))
		{
			listeners.add(listener);
		}
	}

	/**
	 * Remove a previously registered listener. If the listener is not registered
	 * this is a no-op.
	 * @param listener listener to remove
	 */
	public void removeListener(SupervisorListener listener)
	{
		listeners.remove(listener);
	}

	/**
	 * Replay the last intent command to the UI process immediately.
	 * No-op if there is no last intent.
	 */
	private void replayLastIntent()
	{
		if (lastIntent != null)
		{
			uiProcess.sendCommand(lastIntent);
		}
	}

	/**
	 * Flush the queued commands to the UI process in FIFO order.
	 * Uses {@link Queue#poll} to avoid concurrent modification issues.
	 */
	private void flushCommandQueue()
	{
		while (!commandQueue.isEmpty())
		{
			String command = commandQueue.poll();
			if (command != null)
			{
				uiProcess.sendCommand(command);
			}
		}
	}

	/**
	 * Notify registered listeners that the UI became available. Listener exceptions
	 * are ignored to avoid disrupting supervisor flow.
	 */
	private void notifyUiAvailable()
	{
		List<SupervisorListener> listenersCopy = new ArrayList<>(listeners);
		for (SupervisorListener listener : listenersCopy)
		{
			try
			{
				listener.onUiAvailable();
			}
			catch (Exception e)
			{
				// Ignore listener exceptions
			}
		}
	}

	/**
	 * Notify registered listeners that the UI is unavailable (terminal failure).
	 * Listener exceptions are ignored.
	 */
	private void notifyUiUnavailable()
	{
		List<SupervisorListener> listenersCopy = new ArrayList<>(listeners);
		for (SupervisorListener listener : listenersCopy)
		{
			try
			{
				listener.onUiUnavailable();
			}
			catch (Exception e)
			{
				// Ignore listener exceptions
			}
		}
	}

	/**
	 * Notify registered listeners that the UI has crashed. Listener exceptions are ignored.
	 * @param error the crash error to forward to listeners
	 */
	private void notifyUiCrashed(Throwable error)
	{
		List<SupervisorListener> listenersCopy = new ArrayList<>(listeners);
		for (SupervisorListener listener : listenersCopy)
		{
			try
			{
				listener.onUiCrashed(error);
			}
			catch (Exception e)
			{
				// Ignore listener exceptions
			}
		}
	}
}
