package com.ui.cef_control.ipc;

import java.io.IOException;

/**
 * Message channel contract defining outbound message direction.
 *
 * Phase 3 defines the contract only. Implementation is provided in Phase 4.
 * No implementations of this interface exist in Phase 3.
 */
public interface IMessageChannel {
	/**
	 * Sends a message through this channel.
	 *
	 * @param message the message to send
	 */
	void send(String message);

	void close();

	/**
	 * Query page status synchronously.
	 * Must be called only after successful OpenPage command.
	 * Blocks until response received or timeout occurs.
	 *
	 * @param commandId the command ID from OpenPageResponse
	 * @return page status response with current state
	 * @throws IOException if RPC fails
	 * @throws IllegalStateException if handshake not completed
	 */
	void queryPageStatus(String commandId) throws IOException;
}