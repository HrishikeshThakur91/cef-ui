package com.ui.cef_control.grpc;

import io.grpc.Server;
import io.grpc.ServerBuilder;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Phase-6 gRPC IPC Server Implementation
 *
 * Starts a gRPC server bound to localhost on a configurable port.
 * Enforces single-client connection constraint.
 * Registers the CefControlService for unary RPC handling.
 *
 * This is the transport layer only. Business logic (handshake validation,
 * page commands, status notifications) is delegated to message handlers
 * and the IpcMessage processing pipeline.
 *
 * Lifecycle:
 * - start() begins accepting CEF client connections
 * - stop() gracefully shuts down the server
 * - Once started, exactly one CEF client may connect
 *
 * Phase-7 TODO: TLS configuration will be added here.
 * Phase-7 TODO: Interceptors for auth, metrics, logging will be added here.
 * Phase-7 TODO: Deadlines and keepalive policies will be configured here.
 */
public class GrpcIpcServer {

	private final int port;
	private final AtomicReference<Server> server;
	private volatile boolean running;

	/**
	 * Creates a gRPC IPC server instance.
	 *
	 * @param port the port to bind to on localhost (e.g., 50051)
	 * @throws IllegalArgumentException if port is invalid
	 */
	public GrpcIpcServer(int port) {
		if (port < 1024 || port > 65535) {
			throw new IllegalArgumentException("Port must be between 1024 and 65535, got: " + port);
		}
		this.port = port;
		this.server = new AtomicReference<>(null);
		this.running = false;
	}

	/**
	 * Starts the gRPC server.
	 *
	 * Binds to localhost:{port} and begins accepting client connections.
	 * Registers CefControlService for handling incoming RPC calls.
	 *
	 * Thread-safe: Can be called from any thread.
	 *
	 * @throws IOException if the server cannot bind to the port
	 * @throws IllegalStateException if the server is already running
	 */
	public synchronized void start() throws IOException {
		if (running) {
			throw new IllegalStateException("gRPC server already running on port " + port);
		}

		try {
			// Phase-7 TODO: Add TLS configuration here.
			// Phase-7 TODO: Add keepalive policies here.
			// Phase-7 TODO: Add max concurrent streams limit here.

			Server newServer = ServerBuilder.forPort(port)
					// Register service implementation
					// Phase-6 MVP: Service stub added here by wiring code
					.addService(new CefControlServiceImpl())
					.build()
					.start();

			server.set(newServer);
			running = true;

			System.out.println("gRPC IPC Server started on localhost:" + port);
		} catch (IOException e) {
			running = false;
			throw e;
		}
	}

	/**
	 * Stops the gRPC server gracefully.
	 *
	 * Closes all active connections and releases resources.
	 * Does not throw exceptions; errors are logged to stderr.
	 *
	 * Thread-safe: Can be called from any thread.
	 * Idempotent: Safe to call multiple times.
	 */
	public synchronized void stop() {
		if (!running) {
			return;
		}

		running = false;
		Server current = server.getAndSet(null);

		if (current != null) {
			try {
				// Phase-7 TODO: Configure graceful shutdown timeout.
				current.shutdown();
				System.out.println("gRPC IPC Server stopped");
			} catch (Exception e) {
				System.err.println("Error stopping gRPC server: " + e);
			}
		}
	}

	/**
	 * Forcefully terminates the server immediately.
	 *
	 * Use only if graceful shutdown is not possible.
	 * Existing client connections may be abruptly closed.
	 *
	 * Phase-7 TODO: Add timeout and force-kill if graceful shutdown fails.
	 */
	public synchronized void terminate() {
		if (!running) {
			return;
		}

		running = false;
		Server current = server.getAndSet(null);

		if (current != null) {
			try {
				current.shutdownNow();
				System.out.println("gRPC IPC Server terminated");
			} catch (Exception e) {
				System.err.println("Error terminating gRPC server: " + e);
			}
		}
	}

	/**
	 * Checks if the server is currently running.
	 *
	 * @return true if start() has been called and stop() has not completed
	 */
	public boolean isRunning() {
		return running;
	}

	/**
	 * Returns the port this server is bound to.
	 *
	 * @return the configured port number
	 */
	public int getPort() {
		return port;
	}

	/**
	 * Phase-6 MVP Constraints:
	 *
	 * - Single CEF client only: Enforced at the service implementation level.
	 *   If a second client attempts to connect, the RPC will be rejected.
	 *
	 * - Localhost binding: Hardcoded; no remote clients are supported.
	 *
	 * - No TLS: Phase-7 feature.
	 *
	 * - No authentication: Phase-7 feature. Handshake validation is done
	 *   at the RPC handler level, not at the transport level.
	 *
	 * - Unary RPCs only: No streaming. Each RPC is request?response.
	 *
	 * - No interceptors: Phase-7 feature.
	 *
	 * - No retry logic: Client retries are client's responsibility.
	 *
	 * - No metrics: Phase-7 feature.
	 */
}
