package com.ui.cef_control.grpc;

import com.ui.cef_control.ipc.ConnectionListener;
import io.grpc.stub.StreamObserver;

import com.ui.cef_control.grpc.gen.CefControlServiceGrpc;
import com.ui.cef_control.grpc.gen.HandshakeRequest;
import com.ui.cef_control.grpc.gen.HandshakeResponse;
import com.ui.cef_control.grpc.gen.OpenPageRequest;
import com.ui.cef_control.grpc.gen.OpenPageResponse;
import com.ui.cef_control.grpc.gen.PageStatusRequest;
import com.ui.cef_control.grpc.gen.PageStatusResponse;
import com.ui.cef_control.grpc.gen.ShutdownRequest;
import com.ui.cef_control.grpc.gen.ShutdownResponse;

/**
 * Phase-6 gRPC Service Implementation
 *
 * Implements CefControlServiceGrpc.CefControlServiceImplBase to handle
 * incoming RPC calls from the CEF client.
 *
 * Responsibilities:
 * - Handshake: Delegates protocol validation to HandshakeValidator
 * - Validates gRPC transport-level fields
 * - Enforces single-client connection (via HandshakeValidator)
 * - Notifies ConnectionListener on successful handshake
 * - OpenPage: Accepts page load commands from Java
 * - Shutdown: Phase-6 placeholder (no-op)
 *
 * This is a minimal MVP implementation:
 * - No security checks (Phase-7)
 * - No token validation (Phase-7 - delegated to HandshakeValidator)
 * - No retry logic
 * - No business logic
 * - Validation logic delegated to HandshakeValidator
 *
 * Thread-safe: Uses synchronized HandshakeValidator for handshake state.
 *
 * Phase-7 TODO: Add interceptors for authentication and encryption.
 * Phase-7 TODO: Add metadata validation and processing.
 * Phase-7 TODO: Add structured error codes and exceptions.
 * Phase-7 TODO: Add metrics collection (latency, throughput).
 */
public class CefControlServiceImpl extends CefControlServiceGrpc.CefControlServiceImplBase {

	/**
	 * Server version identifier. Returned in HandshakeResponse.
	 * Used for protocol negotiation.
	 */
	private static final String SERVER_VERSION = "1.0.0";

	/**
	 * Handshake validation orchestrator.
	 * Handles protocol-level validation (delegates to Handshake class)
	 * and gRPC-level validation (protocolVersion, parentPid).
	 * Enforces single-client constraint.
	 */
	private final HandshakeValidator handshakeValidator = new HandshakeValidator();

	/**
	 * Listener for connection lifecycle events.
	 * Called on successful handshake and disconnection.
	 * May be null if not registered.
	 */
	private volatile ConnectionListener connectionListener;

	/**
	 * Creates a new service implementation with a connection listener.
	 *
	 * @param connectionListener optional listener for connection events.
	 *                          If null, no lifecycle notifications are sent.
	 */
	public CefControlServiceImpl(ConnectionListener connectionListener) {
		this.connectionListener = connectionListener;
	}

	/**
	 * Creates a new service implementation without a listener.
	 */
	public CefControlServiceImpl() {
		this(null);
	}

	/**
	 * Sets or updates the connection listener.
	 *
	 * @param listener the new listener, or null to disable notifications
	 */
	public void setConnectionListener(ConnectionListener listener) {
		this.connectionListener = listener;
	}

	/**
	 * Handshake RPC implementation.
	 *
	 * Called once by CEF client on initial connection.
	 * Delegates validation to HandshakeValidator which:
	 * 1. Checks single-client constraint (handshake_done flag)
	 * 2. Validates gRPC-level fields:
	 *    - session_token (non-empty)
	 *    - client_version (non-empty)
	 *    - metadata.protocolVersion (non-empty)
	 *    - metadata.parentPid (valid integer > 0)
	 * 3. Delegates to Handshake.handle() for protocol validation
	 *
	 * On success:
	 * - Returns HandshakeResponse with success=true
	 * - Calls ConnectionListener.onConnected()
	 *
	 * On failure:
	 * - Returns HandshakeResponse with success=false and error message
	 * - gRPC session terminated (client must disconnect)
	 *
	 * @param request the HandshakeRequest from CEF
	 * @param responseObserver the gRPC response observer
	 */
	@Override
	public void handshake(
			HandshakeRequest request,
			StreamObserver<HandshakeResponse> responseObserver) {

		// Extract fields from gRPC request
		String sessionToken = request.getSessionToken();
		String clientVersion = request.getClientVersion();
		String protocolVersion = request.getMetadataMap().get("protocolVersion");
		String parentPid = request.getMetadataMap().get("parentPid");

		// Delegate to HandshakeValidator for protocol and gRPC-level validation
		HandshakeValidator.HandshakeValidationResult result = handshakeValidator.validate(
				sessionToken,
				clientVersion,
				protocolVersion,
				parentPid);

		// Build response
		HandshakeResponse response = HandshakeResponse.newBuilder()
				.setSuccess(result.isSuccess())
				.setMessage(result.getMessage())
				.setServerVersion(SERVER_VERSION)
				.build();

		responseObserver.onNext(response);
		responseObserver.onCompleted();

		// If validation succeeded, notify listener
		if (result.isSuccess()) {
			if (connectionListener != null) {
				try {
					connectionListener.onConnected();
				} catch (Exception e) {
					System.err.println("Error notifying listener on connect: " + e);
				}
			}
		}
	}

	/**
	 * OpenPage RPC implementation.
	 *
	 * Receives OpenPageRequest from Java (self).
	 * This RPC is called internally by the Java side to send
	 * page load commands to CEF.
	 *
	 * Phase-6 MVP: Stub implementation only.
	 * Accepts the request and responds immediately with accepted=true.
	 *
	 * Phase-7 TODO: Route to page rendering logic.
	 * Phase-7 TODO: Add timeout and error handling.
	 *
	 * @param request the OpenPageRequest
	 * @param responseObserver the gRPC response observer
	 */
	@Override
	public void openPage(
			OpenPageRequest request,
			StreamObserver<OpenPageResponse> responseObserver) {

		// Phase-6 MVP: Accept all requests
		// Phase-7 TODO: Add business logic to actually render the page

		String commandId = request.getCommandId();
		String pageUrl = request.getPageUrl();

		OpenPageResponse response = OpenPageResponse.newBuilder()
				.setCommandId(commandId)
				.setAccepted(true)
				.setMessage("Page request accepted: " + pageUrl)
				.build();

		responseObserver.onNext(response);
		responseObserver.onCompleted();
	}

	/**
	 * PageStatus RPC implementation.
	 *
	 * Receives PageStatusRequest from Java.
	 * Returns current page status synchronously.
	 *
	 * Phase-6 MVP: Placeholder implementation.
	 * Echoes commandId and returns mock status "LOADED".
	 *
	 * Phase-7 TODO: Query actual page state from internal registry.
	 * Phase-7 TODO: Add error handling and timeout logic.
	 *
	 * @param request the PageStatusRequest
	 * @param responseObserver the gRPC response observer
	 */
	@Override
	public void pageStatus(
			PageStatusRequest request,
			StreamObserver<PageStatusResponse> responseObserver) {

		// Phase-6 MVP: Return mock status
		// Phase-7 TODO: Query actual page rendering state

		String commandId = request.getCommandId();

		PageStatusResponse response = PageStatusResponse.newBuilder()
				.setCommandId(commandId)
				.setStatus("LOADED")
				.setMessage("Page status retrieved")
				.setProgressPercent(100)
				.setTimestampMillis(System.currentTimeMillis())
				.build();

		responseObserver.onNext(response);
		responseObserver.onCompleted();
	}

	/**
	 * Shutdown RPC implementation.
	 *
	 * Phase-6 MVP: Not required. Placeholder for Phase-7.
	 * Phase-7 TODO: Implement graceful CEF shutdown sequence.
	 *
	 * @param request the ShutdownRequest
	 * @param responseObserver the gRPC response observer
	 */
	@Override
	public void shutdown(
			ShutdownRequest request,
			StreamObserver<ShutdownResponse> responseObserver) {

		// Phase-6 MVP: Stub only
		ShutdownResponse response = ShutdownResponse.newBuilder()
				.setAcknowledged(true)
				.setMessage("Shutdown acknowledged")
				.build();

		responseObserver.onNext(response);
		responseObserver.onCompleted();
	}


	/**
	 * Phase-6 MVP Constraints:
	 *
	 * - Single client only: Enforced via HandshakeValidator.
	 *   Second handshake attempt is always rejected.
	 *
	 * - Protocol validation delegated: Handshake class is called via
	 *   HandshakeValidator without modification of Handshake API.
	 *   Protocol logic remains in Handshake; transport logic in gRPC layer.
	 *
	 * - gRPC-level validation: HandshakeValidator validates:
	 *   - session_token (non-empty)
	 *   - client_version (non-empty)
	 *   - protocolVersion from metadata (non-empty)
	 *   - parentPid from metadata (valid integer > 0)
	 *
	 * - No TLS: Not needed at this layer; configured at GrpcIpcServer.
	 *
	 * - No authentication: Phase-7 feature.
	 *   Session token is received but validated by Handshake class.
	 *
	 * - No encryption: Phase-7 feature.
	 *   Metadata is received but not processed for encryption.
	 *
	 * - No retry logic: Failures are terminal for that RPC.
	 *
	 * - Unary RPCs only: Each method is request?response. No streaming.
	 */
}
