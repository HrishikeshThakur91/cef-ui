package com.ui.cef_control.grpc;

/**
 * Listener for connection lifecycle events.
 *
 * Implemented by clients that need to be notified of connection
 * state changes and errors.
 */
public interface ConnectionListener {

    /**
     * Called when the connection is successfully established.
     */
    void onConnected();

    /**
     * Called when the connection is closed.
     */
    void onDisconnected();

    /**
     * Called when an error occurs during connection or communication.
     *
     * @param error the error that occurred
     */
    void onError(Throwable error);
}
