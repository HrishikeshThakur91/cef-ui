package com.ui.cef_control.ipc;

/**
 * Interface for sending control commands to the CEF process.
 * 
 * This is a pure contract interface with no implementation.
 * Implementations will handle the actual communication mechanism
 * (e.g., gRPC, named pipes, etc.).
 * 
 * Constraints:
 * - No file IO
 * - No encryption
 * - No IPC implementation
 * - No process logic
 * - No UI logic
 * - No threading
 * - No static state
 */
public interface IControlCommandChannel {

    /**
     * Sends a control command through this channel.
     * 
     * @param command the command to send (must be non-null)
     * @throws NullPointerException if command is null
     */
    void sendCommand(ControlCommand command);

    /**
     * Shuts down this command channel.
     * 
     * Releases any resources associated with this channel.
     * After calling shutdown, no further commands should be sent.
     */
    void shutdown();
}
