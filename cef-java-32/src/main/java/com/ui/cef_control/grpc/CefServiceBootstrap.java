package com.ui.cef_control.grpc;

import com.ui.cef_control.grpc.ConnectionListener;
import io.grpc.ManagedChannel;
import io.grpc.ManagedChannelBuilder;
import java.io.IOException;
import org.json.simple.JSONObject;

/**
 * Phase-6 MVP Bootstrap
 *
 * Minimal orchestrator for Phase-6 gRPC IPC integration testing.
 *
 * Lifecycle:
 * 1. Start gRPC server on configured port
 * 2. Register service implementation and connection listener
 * 3. Wait for CEF client handshake
 * 4. Send OPEN_PAGE command via message channel
 * 5. Wait briefly for PageStatus response
 * 6. Shutdown server cleanly
 *
 * This is a synchronous, single-threaded MVP with no async or reactive
 * patterns.
 * No lifecycle supervision, no retry logic, no logging framework.
 *
 * Phase-7 TODO: Move to proper application bootstrap/main.
 * Phase-7 TODO: Add configuration from environment or config file.
 * Phase-7 TODO: Add graceful shutdown hooks (SIGTERM, etc.).
 * Phase-7 TODO: Add metrics and structured logging.
 */
public class CefServiceBootstrap {

	private final int ipcPort;
	private final String sessionToken;
	private final String startUrl;
	private GrpcIpcServer server;
	private CefControlServiceImpl serviceImpl;
	private ManagedChannel channel;
	private volatile boolean handshakeDone = false;
	private String lastCommandId = null;

	/**
	 * Creates a bootstrap instance.
	 *
	 * @param ipcPort      the port to bind the gRPC server to (e.g., 50051)
	 * @param sessionToken the session token for handshake validation
	 * @param startUrl     the URL to open in the CEF browser
	 */
	public CefServiceBootstrap(int ipcPort, String sessionToken, String startUrl) {
		this.ipcPort = ipcPort;
		this.sessionToken = sessionToken;
		this.startUrl = startUrl;
	}

	/**
	 * Runs the complete Phase-6 MVP flow.
	 *
	 * 1. Starts gRPC server
	 * 2. Waits for handshake from CEF client
	 * 3. Sends OPEN_PAGE command
	 * 4. Waits for PageStatus response
	 * 5. Shuts down server
	 *
	 * @throws IOException          if server cannot start
	 * @throws InterruptedException if waiting is interrupted
	 */
	public void run() throws IOException, InterruptedException {
		try {
			startServer();
			waitForHandshake();
			sendOpenPageCommand();
			waitForPageStatus();
			shutdown();
		} catch (Exception e) {
			System.err.println("MVP flow error: " + e.getMessage());
			e.printStackTrace();
			shutdown();
			throw e;
		}
	}

	/**
	 * Starts the gRPC server and registers the service implementation.
	 *
	 * Creates:
	 * - GrpcIpcServer: gRPC server bound to localhost:{ipcPort}
	 * - CefControlServiceImpl: Service handler with connection listener
	 * - gRPC channel to CEF service (for sending commands)
	 *
	 * @throws IOException if server cannot bind to port
	 */
	private void startServer() throws IOException {
		System.out.println("[Bootstrap] Starting gRPC IPC server on port " + ipcPort);

		// Create gRPC server
		this.server = new GrpcIpcServer(ipcPort);

		// Create service implementation with connection listener
		this.serviceImpl = new CefControlServiceImpl(new ConnectionListener() {
			@Override
			public void onConnected() {
				System.out.println("[Bootstrap] CEF client connected and handshake successful");
				handshakeDone = true;
				// Wake up waitForHandshake()
				synchronized (CefServiceBootstrap.this) {
					CefServiceBootstrap.this.notifyAll();
				}
			}

			@Override
			public void onDisconnected() {
				System.out.println("[Bootstrap] CEF client disconnected");
			}

			@Override
			public void onError(Throwable error) {
				System.err.println("[Bootstrap] Connection error: " + error.getMessage());
			}
		});

		// Start server
		server.start();

		// Create channel to CEF service for sending commands
		// Phase-6 MVP: Assumes server will be on the same port (loopback)
		this.channel = ManagedChannelBuilder.forAddress("localhost", ipcPort)
				.usePlaintext()
				.build();

		System.out.println("[Bootstrap] gRPC server started and channel created");
	}

	/**
	 * Waits for CEF client handshake to complete.
	 *
	 * Blocks until:
	 * - Handshake succeeds (onConnected() called) OR
	 * - Timeout expires (10 seconds for MVP)
	 *
	 * Phase-6 MVP: Simple polling with sleep.
	 * Phase-7 TODO: Use proper condition variables or latches.
	 *
	 * @throws InterruptedException if waiting is interrupted
	 */
	private void waitForHandshake() throws InterruptedException {
		System.out.println("[Bootstrap] Waiting for CEF client handshake...");

		long deadline = System.currentTimeMillis() + 10000; // 10 second timeout
		while (!handshakeDone && System.currentTimeMillis() < deadline) {
			synchronized (this) {
				try {
					// Wait up to 100ms for notification
					this.wait(100);
				} catch (InterruptedException e) {
					System.err.println("[Bootstrap] Interrupted while waiting for handshake");
					throw e;
				}
			}
		}

		if (!handshakeDone) {
			throw new InterruptedException("Handshake timeout: CEF client did not connect within 10 seconds");
		}

		System.out.println("[Bootstrap] Handshake completed successfully");
	}

	/**
	 * Sends an OPEN_PAGE command to CEF.
	 *
	 * Creates:
	 * - GrpcMessageChannel: IMessageChannel implementation
	 * - OPEN_PAGE message with startUrl
	 * - Sends via channel.send()
	 *
	 * Phase-6 MVP: Synchronous send with no response handling.
	 * Phase-7 TODO: Add response callbacks and async handling.
	 */
	private void sendOpenPageCommand() {
		System.out.println("[Bootstrap] Creating message channel and sending OPEN_PAGE command");

		// Create message channel for sending commands
		GrpcMessageChannel messageChannel = new GrpcMessageChannel(channel);

		// Build OPEN_PAGE message
		JSONObject payload = new JSONObject();
		payload.put("page_url", startUrl);
		payload.put("page_title", "CEF Browser Window");

		JSONObject message = new JSONObject();
		message.put("commandId", "cmd-open-page-1");
		message.put("type", "OPEN_PAGE");
		message.put("payload", payload);

		// Send message
		try {
			messageChannel.send(message.toJSONString());
			System.out.println("[Bootstrap] OPEN_PAGE command sent: " + startUrl);
		} catch (Exception e) {
			System.err.println("[Bootstrap] Error sending OPEN_PAGE: " + e.getMessage());
			throw e;
		}
	}

	/**
	 * Queries page status from CEF.
	 *
	 * Creates:
	 * - PAGE_STATUS message with the command ID from OpenPage
	 * - Sends via message channel
	 * - Waits for synchronous response
	 *
	 * Phase-6 MVP: Direct synchronous query.
	 * Phase-7 TODO: Add async status polling.
	 */
	public void queryPageStatusCommand() {
		System.out.println("[Bootstrap] Querying page status from CEF");

		// Create message channel for sending commands
		GrpcMessageChannel messageChannel = new GrpcMessageChannel(channel);

		// Build PAGE_STATUS query message
		JSONObject message = new JSONObject();
		message.put("commandId", "cmd-open-page-1");
		message.put("type", "PAGE_STATUS");

		// Send message (no payload needed for PAGE_STATUS)
		try {
			messageChannel.send(message.toJSONString());
			System.out.println("[Bootstrap] PAGE_STATUS query sent");
		} catch (Exception e) {
			System.err.println("[Bootstrap] Error querying PAGE_STATUS: " + e.getMessage());
			throw e;
		}
	}

	/**
	 * Waits briefly for PageStatus notification from CEF.
	 *
	 * Phase-6 MVP: Queries page status synchronously via PAGE_STATUS RPC.
	 * In real application, would wait for PageStatus RPC callback.
	 *
	 * Phase-7 TODO: Implement proper async notification delivery.
	 *
	 * @throws InterruptedException if sleep is interrupted
	 */
	private void waitForPageStatus() throws InterruptedException {
		System.out.println("[Bootstrap] Querying page status from CEF...");

		try {
			// Phase-6 MVP: Send synchronous PAGE_STATUS query
			queryPageStatusCommand();

			// Phase-7: Replace with proper callback mechanism
			Thread.sleep(2000);
		} catch (InterruptedException e) {
			System.err.println("[Bootstrap] Interrupted while waiting for page status");
			throw e;
		}

		System.out.println("[Bootstrap] Page status query completed");
	}

	/**
	 * Shuts down the server and closes resources.
	 *
	 * Cleans up:
	 * - gRPC server (graceful shutdown)
	 * - gRPC channel to CEF service
	 * - Any other resources
	 */
	private void shutdown() {
		System.out.println("[Bootstrap] Shutting down");

		if (channel != null && !channel.isShutdown()) {
			try {
				channel.shutdown();
				System.out.println("[Bootstrap] gRPC channel shutdown");
			} catch (Exception e) {
				System.err.println("[Bootstrap] Error shutting down channel: " + e.getMessage());
			}
		}

		if (server != null && server.isRunning()) {
			try {
				server.stop();
				System.out.println("[Bootstrap] gRPC server stopped");
			} catch (Exception e) {
				System.err.println("[Bootstrap] Error stopping server: " + e.getMessage());
			}
		}

		System.out.println("[Bootstrap] Shutdown complete");
	}

	/**
	 * Main entry point for Phase-6 MVP testing.
	 *
	 * Usage:
	 * java CefServiceBootstrap 50051 "test-token-123" "http://localhost:8080/docs"
	 *
	 * Args:
	 * [0] ipcPort - gRPC server port (e.g., 50051)
	 * [1] sessionToken - session token for handshake (e.g., "test-token-123")
	 * [2] startUrl - URL to open in CEF (e.g., "http://localhost:8080/docs")
	 *
	 * @param args command-line arguments
	 */
	public static void main(String[] args) {
		if (args.length < 3) {
			System.err.println("Usage: CefServiceBootstrap <ipcPort> <sessionToken> <startUrl>");
			System.err.println("Example: CefServiceBootstrap 50051 test-token-123 http://localhost:8080/docs");
			System.exit(1);
		}

		try {
			int ipcPort = Integer.parseInt(args[0]);
			String sessionToken = args[1];
			String startUrl = args[2];

			CefServiceBootstrap bootstrap = new CefServiceBootstrap(ipcPort, sessionToken, startUrl);
			bootstrap.run();

			System.out.println("[Bootstrap] MVP flow completed successfully");
			System.exit(0);

		} catch (NumberFormatException e) {
			System.err.println("Invalid port number: " + args[0]);
			System.exit(1);
		} catch (Exception e) {
			System.err.println("Bootstrap failed: " + e.getMessage());
			e.printStackTrace();
			System.exit(1);
		}
	}

	/**
	 * Phase-6 MVP Constraints:
	 *
	 * - Single-threaded: All operations synchronous, no thread pools.
	 * Phase-7 will add async/reactive patterns.
	 *
	 * - No lifecycle supervision: No process management or restart logic.
	 * Phase-7 will integrate with UIProcess and UISupervisor.
	 *
	 * - No retries: All operations are one-shot.
	 * Failures terminate the flow.
	 *
	 * - No logging framework: Uses System.out/System.err only.
	 * Phase-7 will add SLF4J or similar.
	 *
	 * - Blocking waits: Simple sleep() and wait() calls.
	 * Phase-7 will use CountDownLatch, CompletableFuture, etc.
	 *
	 * - Hardcoded timeouts: 10 seconds for handshake, 5 seconds for page status.
	 * Phase-7 will configure via AppConfig.
	 *
	 * - Command-line driven: Takes args for port, token, URL.
	 * Phase-7 will integrate with AppConfig parsing.
	 */
}
