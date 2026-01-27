package com.ui.cef_control.ipc;

import java.io.IOException;
import java.net.URI;
import java.util.ArrayList;
import java.util.List;

/**
 * Fire-and-forget WebSocket Secure (WSS) channel implementation.
 *
 * Phase 4: Simple state management with no transport implementation.
 * No retries, no reconnection logic, no threading.
 */
public class WssChannel implements IMessageChannel {

	private final URI endpoint;
	private final String sessionToken;
	private final List<ConnectionListener> listeners = new ArrayList<>();;
	private boolean connected = false;

	public WssChannel(URI endpoint, String sessionToken) {
		if (endpoint == null) {
			throw new IllegalArgumentException("Endpoint cannot be null");
		}

		if (!"wss".equals(endpoint.getScheme())) {
			throw new IllegalArgumentException(
					"Only secure WebSocket (wss://) endpoints are allowed"
			);
		}

		if (sessionToken == null || sessionToken.isEmpty()) {
			throw new IllegalArgumentException("Session token cannot be null or empty");
		}

		this.endpoint = endpoint;
		this.sessionToken = sessionToken;
//		this.listeners = new ArrayList<>();
//		this.connected = false;
	}

	public void connect() {
		if (connected) {
			return;
		}

		connected = true;
		for (ConnectionListener l : new ArrayList<>(listeners)) {
			l.onConnected();
		}
	}

	public void send(String message) {
		if (!connected) {
			return; // fire-and-forget, ignore
//			throw new IllegalStateException("Cannot send message: not connected");
		}
		// Fire-and-forget: accept the message but do nothing
	}

	public void close() {
		if (!connected) {
			return;
		}

		connected = false;
		for (ConnectionListener l : new ArrayList<>(listeners)) {
			l.onDisconnected();
		}
	}


	@Override
	public void queryPageStatus(String commandId) throws IOException
	{
		// Not implemented in WSS channel
		throw new UnsupportedOperationException("queryPageStatus is not supported in WssChannel");
	}


	public void addConnectionListener(ConnectionListener listener) {
		if (listener != null && !listeners.contains(listener)) {
			listeners.add(listener);
		}
	}

	public void removeConnectionListener(ConnectionListener listener) {
		listeners.remove(listener);
	}

	public boolean isConnected() {
		return connected;
	}

	private void notifyConnected() {
		List<ConnectionListener> listenersCopy = new ArrayList<>(listeners);
		for (ConnectionListener listener : listenersCopy) {
			try {
				listener.onConnected();
			} catch (Exception e) {
				// Ignore listener exceptions
			}
		}
	}

	private void notifyDisconnected() {
		List<ConnectionListener> listenersCopy = new ArrayList<>(listeners);
		for (ConnectionListener listener : listenersCopy) {
			try {
				listener.onDisconnected();
			} catch (Exception e) {
				// Ignore listener exceptions
			}
		}
	}
}
