package com.ui.cef_control.supervisor;

/**
 * Abstraction of the UI process controlled by {@link UISupervisor}.
 * Implementations encapsulate the platform-specific details required to start, stop
 * and send commands to the UI process. Methods should be designed to be idempotent
 * where noted by the supervisor contract.
 */
public interface UIProcess
{

	/**
	 * Start the UI process.
	 * Throws exception on failure.
	 */
	void start();


	/**
	 * Stop the UI process.
	 * Must be idempotent.
	 */
	void stop();


	/**
	 * sendCommand to the UI process.
	 * Must be idempotent.
	 * @param command - non-null, non-empty command string
	 */
	void sendCommand(String command);
}
