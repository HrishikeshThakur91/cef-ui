package com.ui.cef_control.grpc;

import com.ui.cef_control.ipc.IMessageChannel;
import io.grpc.ManagedChannel;
import io.grpc.StatusRuntimeException;
import java.io.IOException;
import java.util.concurrent.TimeoutException;
import org.json.simple.JSONObject;

import com.ui.cef_control.grpc.gen.CefControlServiceGrpc;
import com.ui.cef_control.grpc.gen.OpenPageRequest;
import com.ui.cef_control.grpc.gen.PageStatusResponse;
import com.ui.cef_control.grpc.gen.OpenPageResponse;
import com.ui.cef_control.grpc.gen.PageStatusRequest;


/**
 * Phase-6 gRPC Message Channel Implementation
 *
 * Implements IMessageChannel to send commands to CEF via gRPC.
 * Converts IpcMessage objects to gRPC requests and dispatches them.
 *
 * Responsibilities:
 * - Convert IpcMessage ? OpenPageRequest proto
 * - Send OpenPageRequest via CefControlService.openPage() RPC
 * - Handle RPC responses
 * - Map message type to appropriate gRPC RPC
 *
 * This channel does NOT:
 * - Own the gRPC server or channel lifecycle
 * - Perform handshake validation (delegated to CefControlServiceImpl)
 * - Retry failed RPCs (client responsibility)
 * - Cache messages
 * - Perform JSON parsing (input is already IpcMessage object)
 *
 * Thread-safe: Can be called from any thread. Each RPC is independent.
 *
 * Phase-7 TODO: Add metadata conversion (encryption, signing, headers).
 * Phase-7 TODO: Add timeout and deadline handling.
 * Phase-7 TODO: Add error recovery and structured exception mapping.
 */
public class GrpcMessageChannel implements IMessageChannel {

	/**
	 * Message type constant for OPEN_PAGE commands.
	 * Phase-6 MVP: Only OPEN_PAGE is supported.
	 * Phase-7 TODO: Support additional message types (PAGE_QUERY, etc.)
	 */
	private static final String MESSAGE_TYPE_OPEN_PAGE = "OPEN_PAGE";

	/**
	 * Message type constant for PAGE_STATUS queries.
	 * Phase-6 MVP: Supported for synchronous page state queries.
	 */
	private static final String MESSAGE_TYPE_PAGE_STATUS = "PAGE_STATUS";

	/**
	 * gRPC channel for communication with CEF service.
	 * Managed externally; this channel does not own it.
	 */
	private final ManagedChannel channel;

	/**
	 * gRPC service stub for invoking CefControlService RPCs.
	 * Initialized from the channel.
	 */
	private final CefControlServiceGrpc.CefControlServiceBlockingStub stub;

	/**
	 * Creates a new GrpcMessageChannel instance.
	 *
	 * @param channel the gRPC ManagedChannel for communicating with CEF.
	 *               Must be open and initialized.
	 *               Ownership remains with the caller.
	 * @throws NullPointerException if channel is null
	 */
	public GrpcMessageChannel(ManagedChannel channel) {
		if (channel == null) {
			throw new NullPointerException("gRPC channel cannot be null");
		}
		this.channel = channel;
		this.stub = CefControlServiceGrpc.newBlockingStub(channel);
	}

	/**
	 * Sends a message through the gRPC channel.
	 *
	 * Converts the JSON message string to an IpcMessage object,
	 * then dispatches to the appropriate gRPC RPC based on message type.
	 *
	 * Phase-6 MVP: Supports OPEN_PAGE messages only.
	 *
	 * Thread-safe: Can be called from any thread.
	 *
	 * @param messageJson JSON string representation of the message.
	 *                   Must contain "commandId", "type", and "payload" fields.
	 * @throws IllegalArgumentException if messageJson is invalid or message type is unsupported
	 * @throws StatusRuntimeException if the gRPC RPC fails
	 */
	@Override
	public void send(String messageJson) {
		if (messageJson == null || messageJson.trim().isEmpty()) {
			throw new IllegalArgumentException("Message cannot be null or empty");
		}

		// Phase-6 MVP: Parse JSON to extract message fields
		// Note: We do NOT use IpcMessage.fromJson() here because the contract is:
		// send(String message) expects raw JSON, not an IpcMessage object.
		// The conversion happens at the boundary.

		// Phase-7 TODO: Use a proper JSON parser here (JSONParser from json-simple)
		// For Phase-6 MVP, we assume minimal JSON with required fields only.
		String commandId = extractJsonField(messageJson, "commandId");
		String messageType = extractJsonField(messageJson, "type");
		JSONObject payload = extractJsonPayload(messageJson);

		if (commandId == null || commandId.isEmpty()) {
			throw new IllegalArgumentException("Message missing required field: commandId");
		}

		if (messageType == null || messageType.isEmpty()) {
			throw new IllegalArgumentException("Message missing required field: type");
		}

		// Route to appropriate handler based on message type
		if (MESSAGE_TYPE_OPEN_PAGE.equals(messageType)) {
			sendOpenPage(commandId, payload);
		} else if (MESSAGE_TYPE_PAGE_STATUS.equals(messageType)) {
			queryPageStatus(commandId);
		} else {
			// Phase-7 TODO: Add support for other message types (PAGE_QUERY, etc.)
			throw new IllegalArgumentException("Unsupported message type: " + messageType);
		}
	}

	/**
	 * Sends an OPEN_PAGE command to CEF via gRPC.
	 *
	 * Converts IPC message payload to OpenPageRequest proto.
	 * Invokes CefControlService.openPage() and waits for response.
	 *
	 * @param commandId unique identifier for this command
	 * @param payload   JSON object containing page_url and optional page_title
	 * @throws StatusRuntimeException if the RPC fails
	 * @throws IllegalArgumentException if payload is invalid
	 */
	private void sendOpenPage(String commandId, JSONObject payload) {
		// Extract fields from payload
		String pageUrl = (String) payload.get("page_url");
		String pageTitle = (String) payload.get("page_title");

		if (pageUrl == null || pageUrl.isEmpty()) {
			throw new IllegalArgumentException("OPEN_PAGE message missing required payload field: page_url");
		}

		// Build OpenPageRequest proto
		OpenPageRequest request = OpenPageRequest.newBuilder()
				.setCommandId(commandId)
				.setPageUrl(pageUrl)
				.setPageTitle(pageTitle != null ? pageTitle : "")
				// Phase-7 TODO: Add metadata conversion from IpcMessage context
				// Phase-7 TODO: Add encryption/signing metadata
				.build();

		try {
			// Invoke gRPC RPC (blocking call)
			// Phase-7 TODO: Add timeout context
			// Phase-7 TODO: Add deadline from message metadata
			OpenPageResponse response = stub.openPage(request);

			// Phase-6 MVP: Log response for debugging
			if (!response.getAccepted()) {
				System.err.println("OpenPage RPC rejected: " + response.getMessage());
			}

			// Phase-7 TODO: Add structured error handling for non-accepted responses
			// Phase-7 TODO: Add callback or listener for response processing

		} catch (StatusRuntimeException e) {
			// Phase-7 TODO: Add retry logic here (with exponential backoff)
			// Phase-7 TODO: Map gRPC Status to domain exceptions
			// Phase-7 TODO: Add logging and observability
			System.err.println("OpenPage RPC failed: " + e.getStatus());
			throw e;
		}
	}

	/**
	 * Queries current page status from CEF via synchronous gRPC call.
	 *
	 * Sends PageStatusRequest to CEF and receives immediate PageStatusResponse.
	 * Used to poll the current page load state.
	 *
	 * @param commandId the command ID to query status for
	 * @throws StatusRuntimeException if the RPC fails
	 */
	public void queryPageStatus(String commandId) {
		// Build PageStatusRequest proto
		PageStatusRequest request = PageStatusRequest.newBuilder()
				.setCommandId(commandId)
				.build();

		try {
			// Invoke gRPC RPC (blocking call)
			// Phase-7 TODO: Add timeout context
			PageStatusResponse response = stub.pageStatus(request);

			// Phase-6 MVP: Log response for debugging
			System.out.println("PageStatus Response: commandId=" + response.getCommandId()
					+ ", status=" + response.getStatus()
					+ ", message=" + response.getMessage()
					+ ", progress=" + response.getProgressPercent() + "%");

			// Phase-7 TODO: Add structured response handling
			// Phase-7 TODO: Add callback or listener for response processing
			// Phase-7 TODO: Add metadata extraction and processing

		} catch (StatusRuntimeException e) {
			// Phase-7 TODO: Add retry logic here (with exponential backoff)
			// Phase-7 TODO: Map gRPC Status to domain exceptions
			// Phase-7 TODO: Add logging and observability
			System.err.println("PageStatus RPC failed: " + e.getStatus());
			throw e;
		}
	}

	/**
	 * Closes the message channel.
	 *
	 * Does NOT close the underlying gRPC channel (ownership is external).
	 * Phase-6 MVP: Channel lifecycle is managed by GrpcIpcServer.
	 *
	 * Phase-7 TODO: Add resource cleanup hooks.
	 */
	@Override
	public void close() {
		// Phase-6 MVP: No-op. Channel is managed externally.
		// Phase-7 TODO: Add cleanup logic if needed.
		System.out.println("GrpcMessageChannel closed");
	}

	/**
	 * Extracts a string field from a JSON object represented as a string.
	 *
	 * Phase-6 MVP: Simple string parsing (not robust).
	 * Phase-7 TODO: Replace with proper JSONParser from json-simple.
	 *
	 * @param json the JSON string
	 * @param fieldName the field name to extract
	 * @return the field value, or null if not found
	 */
	private String extractJsonField(String json, String fieldName) {
		// Phase-6 MVP: Simple parsing for basic JSON
		// Example: {"commandId":"cmd-1","type":"OPEN_PAGE",...}
		String searchKey = "\"" + fieldName + "\":\"";
		int startIdx = json.indexOf(searchKey);
		if (startIdx < 0) {
			return null;
		}
		startIdx += searchKey.length();
		int endIdx = json.indexOf("\"", startIdx);
		if (endIdx < 0) {
			return null;
		}
		return json.substring(startIdx, endIdx);
	}

	/**
	 * Extracts the payload object from a JSON string.
	 *
	 * Phase-6 MVP: Returns an empty JSONObject if payload is missing.
	 * Phase-7 TODO: Use proper JSONParser to extract nested payload.
	 *
	 * @param json the JSON string
	 * @return JSONObject representing the payload, or empty if missing
	 */
	private JSONObject extractJsonPayload(String json) {
		// Phase-6 MVP: Simple extraction of nested payload object
		// For now, we construct a minimal JSONObject with expected fields
		JSONObject payload = new JSONObject();

		// Extract page_url
		String pageUrl = extractJsonField(json, "page_url");
		if (pageUrl != null) {
			payload.put("page_url", pageUrl);
		}

		// Extract page_title
		String pageTitle = extractJsonField(json, "page_title");
		if (pageTitle != null) {
			payload.put("page_title", pageTitle);
		}

		// Phase-7 TODO: Parse full nested JSON payload from proto
		// Phase-7 TODO: Support arbitrary payload structure

		return payload;
	}

	// Add these methods:

	private void handlePageStatusError(StatusRuntimeException e, String commandId) {
		System.err.println("PageStatus RPC failed for command: " + commandId);
		System.err.println("Status: " + e.getStatus().getCode());
		System.err.println("Description: " + e.getStatus().getDescription());
		// Phase-7 TODO: Add exponential backoff retry
		// Phase-7 TODO: Add metrics collection
	}

	public PageStatusResponse queryPageStatusSync(String commandId, long timeoutMillis)
			throws IOException, TimeoutException
	{
		try
		{
			PageStatusRequest request = PageStatusRequest.newBuilder()
					.setCommandId(commandId)
					.build();

			// Phase-7 TODO: Add deadline (timeoutMillis) via context
			PageStatusResponse response = stub.pageStatus(request);
			return response;
		}
		catch (StatusRuntimeException e)
		{
			handlePageStatusError(e, commandId);
			throw new IOException("PageStatus RPC failed: " + e.getStatus(), e);
		}
	}

	/**
	 * Phase-6 MVP Constraints:
	 *
	 * - OPEN_PAGE only: No other message types supported.
	 *   Phase-7 will add support for PAGE_QUERY, PAGE_CLOSE, etc.
	 *
	 * - No JSON parsing library: Phase-6 uses simple string extraction.
	 *   Phase-7 will integrate org.json.simple.JSONParser.
	 *
	 * - No retry logic: Failed RPCs throw StatusRuntimeException immediately.
	 *   Phase-7 will add exponential backoff and retry policies.
	 *
	 * - No caching: Each send() is independent.
	 *   Phase-7 may add message queuing if needed.
	 *
	 * - No metadata conversion: Metadata fields are ignored.
	 *   Phase-7 will extract and convert encryption/signing metadata.
	 *
	 * - No timeout handling: All RPCs use gRPC default timeouts.
	 *   Phase-7 will add deadline extraction from message metadata.
	 *
	 * - Channel not owned: The underlying ManagedChannel is managed externally.
	 *   Close() is a no-op for Phase-6.
	 *
	 * - Thread-safe: All RPC invocations are independent and can be called
	 *   from different threads concurrently.
	 */
}
