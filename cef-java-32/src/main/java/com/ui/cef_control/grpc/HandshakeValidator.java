package com.ui.cef_control.grpc;

import com.ui.cef_control.ipc.Handshake;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import org.json.simple.JSONObject;

/**
 * Phase-6 Handshake Validation Wiring
 *
 * Bridges gRPC transport layer with protocol-level Handshake validation.
 * Orchestrates the handshake flow:
 * - Validates protocolVersion and parentPid from gRPC HandshakeRequest
 * - Delegates JSON validation to existing Handshake class
 * - Enforces single handshake per client
 * - Terminates gRPC session on failure
 *
 * Responsibilities:
 * - Convert gRPC HandshakeRequest ? JSON for Handshake.handle()
 * - Validate gRPC-level fields (protocolVersion, parentPid, sessionToken)
 * - Call existing Handshake class without modifying its API
 * - Track handshake state (one-time only)
 * - Return appropriate HandshakeResponse with success/failure status
 *
 * Thread-safe: Uses synchronized methods for state management.
 *
 * Phase-7 TODO: Add encryption key exchange in metadata.
 * Phase-7 TODO: Add detailed error codes and structured exceptions.
 */
public class HandshakeValidator {

	/**
	 * Tracks whether handshake has been completed for a single client.
	 * Once true, subsequent handshakes are rejected (single-client constraint).
	 */
	private volatile boolean handshakeDone = false;
	private Set<String> activeCommandIds = Collections.synchronizedSet(new HashSet<>());

	/**
	 * Validates a CEF client handshake request.
	 *
	 * Flow:
	 * 1. Check single-client constraint (handshake_done flag)
	 * 2. Validate required gRPC fields:
	 *    - session_token (non-empty)
	 *    - client_version (non-empty)
	 *    - metadata.protocolVersion (present)
	 *    - metadata.parentPid (valid integer > 0)
	 * 3. Build JSON envelope with required fields
	 * 4. Delegate to Handshake.handle(json) for protocol validation
	 * 5. Return success response if all validations pass
	 * 6. Return failure response with error message if any check fails
	 *
	 * @param sessionToken session token from HandshakeRequest
	 * @param clientVersion client version from HandshakeRequest
	 * @param protocolVersion protocol version from metadata
	 * @param parentPid parent process ID from metadata
	 * @return HandshakeValidationResult with success flag and message
	 */
	public synchronized HandshakeValidationResult validate(
			String sessionToken,
			String clientVersion,
			String protocolVersion,
			String parentPid) {

		// 1. Enforce single-client constraint
		if (handshakeDone) {
			return HandshakeValidationResult.failure(
					"Handshake already completed. Only one client allowed.");
		}

		// 2. Validate gRPC-level required fields

		// Validate sessionToken
		if (sessionToken == null || sessionToken.trim().isEmpty()) {
			return HandshakeValidationResult.failure("Invalid or missing session_token");
		}

		// Validate clientVersion
		if (clientVersion == null || clientVersion.trim().isEmpty()) {
			return HandshakeValidationResult.failure("Invalid or missing client_version");
		}

		// Validate protocolVersion (gRPC-specific, not delegated to Handshake)
		if (protocolVersion == null || protocolVersion.trim().isEmpty()) {
			return HandshakeValidationResult.failure(
					"Invalid or missing protocolVersion in metadata");
		}

		// Validate parentPid (gRPC-specific)
		if (parentPid == null || parentPid.trim().isEmpty()) {
			return HandshakeValidationResult.failure(
					"Invalid or missing parentPid in metadata");
		}

		// Parse and validate parentPid as integer
		int parentPidValue;
		try {
			parentPidValue = Integer.parseInt(parentPid);
		} catch (NumberFormatException e) {
			return HandshakeValidationResult.failure(
					"Invalid parentPid format: " + parentPid + " (must be integer)");
		}

		if (parentPidValue <= 0) {
			return HandshakeValidationResult.failure(
					"Invalid parentPid: must be > 0, got " + parentPidValue);
		}

		// 3. Build JSON envelope for Handshake.handle()
		// The Handshake class expects JSON with:
		// - type: "HELLO"
		// - sessionToken: from request
		// - Additional fields can be passed as metadata
		JSONObject envelopeJson = new JSONObject();
		envelopeJson.put("type", "HELLO");
		envelopeJson.put("sessionToken", sessionToken);
		envelopeJson.put("clientVersion", clientVersion);
		envelopeJson.put("protocolVersion", protocolVersion);
		envelopeJson.put("parentPid", parentPid);

		// Phase-7 TODO: Add encryption metadata to envelope
		// Phase-7 TODO: Add capability negotiation fields

		// 4. Delegate to protocol-level validation
		try {
			Handshake.handle(envelopeJson.toJSONString());
		} catch (IllegalArgumentException e) {
			// Handshake validation failed
			return HandshakeValidationResult.failure(
					"Handshake validation failed: " + e.getMessage());
		} catch (Exception e) {
			// Unexpected error during validation
			return HandshakeValidationResult.failure(
					"Handshake error: " + e.getMessage());
		}

		// 5. All validations passed - mark handshake as done
		handshakeDone = true;

		return HandshakeValidationResult.success();
	}

	/**
	 * Resets the handshake state.
	 * Used for testing or client disconnection/reconnection scenarios.
	 *
	 * Phase-6 MVP: Internal use only.
	 * Phase-7 TODO: Implement proper reconnection logic.
	 */
	public synchronized void reset() {
		handshakeDone = false;
	}

	/**
	 * Checks if handshake has been completed.
	 *
	 * @return true if handshake RPC has been successfully invoked
	 */
	public synchronized boolean isHandshakeDone() {
		return handshakeDone;
	}

	/**
	 * Result wrapper for handshake validation.
	 *
	 * Encapsulates success/failure status and error message.
	 * Used to separate validation concerns from transport concerns.
	 */
	public static final class HandshakeValidationResult {
		private final boolean success;
		private final String message;

		private HandshakeValidationResult(boolean success, String message) {
			this.success = success;
			this.message = message;
		}

		/**
		 * Creates a successful validation result.
		 */
		public static HandshakeValidationResult success() {
			return new HandshakeValidationResult(true, "Handshake accepted");
		}

		/**
		 * Creates a failure validation result.
		 *
		 * @param message error or reason message
		 */
		public static HandshakeValidationResult failure(String message) {
			return new HandshakeValidationResult(false, message);
		}

		public boolean isSuccess() {
			return success;
		}

		public String getMessage() {
			return message;
		}
	}

	public void registerCommand(String commandId)
	{
		activeCommandIds.add(commandId);
	}

	public boolean isCommandValid(String commandId)
	{
		return activeCommandIds.contains(commandId);
	}

	public void unregisterCommand(String commandId)
	{
		activeCommandIds.remove(commandId);
	}

	/**
	 * Phase-6 MVP Constraints:
	 *
	 * - Single handshake only: Enforced via handshakeDone flag.
	 *
	 * - Delegated validation: Existing Handshake class is called without modification.
	 *   Protocol logic remains in Handshake; transport logic remains in gRPC layer.
	 *
	 * - Required field validation:
	 *   - sessionToken: Non-empty (passed to Handshake)
	 *   - clientVersion: Non-empty (gRPC-level validation)
	 *   - protocolVersion: Non-empty (gRPC-level validation)
	 *   - parentPid: Valid integer > 0 (gRPC-level validation)
	 *
	 * - No security checks: Phase-7 feature.
	 *   Session token is passed to Handshake for validation.
	 *
	 * - No retries: Validation failures are terminal.
	 *
	 * - No transport logic inside validation:
	 *   gRPC concerns (HandshakeResponse building, RPC termination)
	 *   are handled by CefControlServiceImpl, not here.
	 */
}
