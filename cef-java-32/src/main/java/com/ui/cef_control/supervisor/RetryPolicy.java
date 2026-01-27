package com.ui.cef_control.supervisor;

/**
 * Retry policy used by {@link com.ui.cef_control.supervisor.UISupervisor}
 * to decide whether to attempt another UI startup after a failure, and to determine
 * any delay before the next attempt.
 *
 * Implementations should be deterministic and side-effect free when possible.
 */
public interface RetryPolicy
{
	/**
	 * Decide whether another startup attempt should be made.
	 * @param attempt current attempt number (starting from 1)
	 * @param failure the last failure that caused the previous attempt to fail; may be null
	 * @return {@code true} if the supervisor should attempt another startup; {@code false}
	 *         to stop retrying and treat the UI as permanently unavailable
	 */
	boolean shouldRetry(int attempt, Throwable failure);

	/**
	 * Return the delay in milliseconds before the next retry attempt.
	 * The default implementation returns 0 (no delay).
	 * @param attemptNumber the attempt number that will be performed next (starting from 1)
	 * @return delay in milliseconds before performing the next attempt
	 */
	default int getRetryDelayMs(int attemptNumber)
	{
		return 0; // Phase 5: no delay
	}
	// ---------- Static helpers for tests ----------

	/**
	 * No-retry policy: never retry.
	 * @return a {@link RetryPolicy} that always returns {@code false} from {@link #shouldRetry}
	 */
	static RetryPolicy noRetry()
	{
		return (attempt, failure) -> false;
	}

	/**
	 * Always-retry policy: always allow another attempt.
	 * @return a {@link RetryPolicy} that always returns {@code true} from {@link #shouldRetry}
	 */
	static RetryPolicy alwaysRetry()
	{
		return (attempt, failure) -> true;
	}

	/**
	 * Creates a bounded retry policy.
	 * @param maxRetries maximum number of attempts allowed (inclusive). Attempts are counted
	 *                   starting from 1.
	 * @return a {@link RetryPolicy} that permits retries while {@code attempt <= maxRetries}
	 */
	static RetryPolicy maxRetries(int maxRetries)
	{
		return (attempt, failure) -> attempt <= maxRetries;
	}

	default long getBackoffMs(int attempt) {
		// Exponential backoff: 2^(attempt-1) * 100 ms
		return (1L << (attempt - 1)) * 100L;
	}
}
