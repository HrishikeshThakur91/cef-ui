package com.ui.cef_control.grpc;

import com.ui.cef_control.grpc.gen.CefStatusCallbackServiceGrpc;
import com.ui.cef_control.grpc.gen.PageStatusNotification;
import com.ui.cef_control.grpc.gen.StatusAck;
import io.grpc.stub.StreamObserver;

import java.util.concurrent.ConcurrentHashMap;
import java.util.Map;

/**
 * Phase-6.3 Status Callback Service Implementation
 *
 * Implements CefStatusCallbackService to receive status notifications from CEF.
 * Java implements this service; CEF calls it to push status updates.
 *
 * Responsibilities:
 * - Receive PageStatusNotification from CEF
 * - Validate required fields (command_id, status)
 * - Log each status event with timestamp
 * - Store last-known status in memory (optional)
 * - Return StatusAck to CEF
 *
 * This is WIRING ONLY:
 * - NO decision-making based on status
 * - NO retry logic
 * - NO blocking operations
 * - NO new commands triggered
 *
 * Thread-safe: Uses ConcurrentHashMap for status storage.
 * Non-blocking: All operations are fast and return immediately.
 *
 * Phase-7 TODO: Add authentication and encryption.
 * Phase-7 TODO: Add structured logging framework.
 * Phase-7 TODO: Add metrics collection.
 */
public class CefStatusCallbackServiceImpl extends CefStatusCallbackServiceGrpc.CefStatusCallbackServiceImplBase {

    /**
     * In-memory storage of last-known status for each command_id.
     * Used for diagnostics and debugging only.
     * Thread-safe: ConcurrentHashMap allows concurrent reads/writes.
     */
    private final Map<String, PageStatusNotification> statusMap = new ConcurrentHashMap<>();

    /**
     * NotifyPageStatus RPC implementation.
     *
     * Called by CEF when page status changes:
     * - LOADING: Page started loading
     * - LOADED: Page finished loading successfully
     * - ERROR: Page encountered an error
     * - SHUTDOWN: Browser is closing
     *
     * This method MUST NOT block or perform long operations.
     * It logs the status and returns immediately.
     *
     * Thread-safe: Can be called concurrently from multiple CEF threads.
     * Non-blocking: All operations are fast (logging + map update).
     *
     * @param request the PageStatusNotification from CEF
     * @param responseObserver the gRPC response observer
     */
    @Override
    public void notifyPageStatus(
            PageStatusNotification request,
            StreamObserver<StatusAck> responseObserver) {

        // Extract fields
        String commandId = request.getCommandId();
        String status = request.getStatus();
        String message = request.getMessage();
        int progressPercent = request.getProgressPercent();
        long timestampMillis = request.getTimestampMillis();

        // Validate required fields
        if (commandId == null || commandId.isEmpty()) {
            // Log error but still acknowledge (non-blocking)
            System.err.println("[CefStatusCallback] ERROR: Received status notification with missing command_id");
            
            StatusAck errorAck = StatusAck.newBuilder()
                    .setCommandId("")
                    .setReceived(false)
                    .setErrorMessage("Missing command_id")
                    .build();
            
            responseObserver.onNext(errorAck);
            responseObserver.onCompleted();
            return;
        }

        if (status == null || status.isEmpty()) {
            // Log error but still acknowledge
            System.err.println("[CefStatusCallback] ERROR: Received status notification with missing status for command: " + commandId);
            
            StatusAck errorAck = StatusAck.newBuilder()
                    .setCommandId(commandId)
                    .setReceived(false)
                    .setErrorMessage("Missing status")
                    .build();
            
            responseObserver.onNext(errorAck);
            responseObserver.onCompleted();
            return;
        }

        // Log the status event (Phase 6.3 requirement: LOG status events)
        logStatusEvent(commandId, status, message, progressPercent, timestampMillis);

        // Store status in memory (Phase 6.3 optional: STORE status)
        storeStatus(commandId, request);

        // Build acknowledgement
        StatusAck ack = StatusAck.newBuilder()
                .setCommandId(commandId)
                .setReceived(true)
                .setErrorMessage("")  // No error
                .build();

        // Send response
        responseObserver.onNext(ack);
        responseObserver.onCompleted();
    }

    /**
     * Logs a status event to console.
     *
     * Phase-6.3: Simple console logging.
     * Phase-7 TODO: Use structured logging framework (SLF4J).
     *
     * @param commandId the command ID
     * @param status the status string (LOADING, LOADED, ERROR, SHUTDOWN)
     * @param message optional detail message
     * @param progressPercent progress 0-100, or -1 if not applicable
     * @param timestampMillis timestamp when status was generated
     */
    private void logStatusEvent(String commandId, String status, String message, 
                                 int progressPercent, long timestampMillis) {
        // Format timestamp
        String timestamp = String.format("%tF %<tT", timestampMillis);
        
        // Build log message
        StringBuilder logMsg = new StringBuilder();
        logMsg.append("[CefStatusCallback] ");
        logMsg.append(timestamp);
        logMsg.append(" | command_id=").append(commandId);
        logMsg.append(" | status=").append(status);
        
        if (progressPercent >= 0) {
            logMsg.append(" | progress=").append(progressPercent).append("%");
        }
        
        if (message != null && !message.isEmpty()) {
            logMsg.append(" | message=").append(message);
        }
        
        // Log to console
        System.out.println(logMsg.toString());
    }

    /**
     * Stores status in memory for diagnostics.
     *
     * Phase-6.3: Simple in-memory map.
     * Phase-7 TODO: Add TTL and cleanup for old entries.
     * Phase-7 TODO: Add metrics on status transitions.
     *
     * @param commandId the command ID
     * @param notification the full notification object
     */
    private void storeStatus(String commandId, PageStatusNotification notification) {
        // Store in concurrent map (thread-safe)
        statusMap.put(commandId, notification);
        
        // Phase-7 TODO: Add cleanup logic for old entries
        // Phase-7 TODO: Add size limit to prevent memory leak
    }

    /**
     * Gets the last-known status for a command ID.
     *
     * Used for diagnostics and debugging only.
     * Phase-6.3: Simple getter.
     *
     * @param commandId the command ID to query
     * @return the last PageStatusNotification, or null if not found
     */
    public PageStatusNotification getLastStatus(String commandId) {
        return statusMap.get(commandId);
    }

    /**
     * Gets all stored statuses.
     *
     * Used for diagnostics and debugging only.
     * Phase-6.3: Simple getter.
     *
     * @return unmodifiable view of the status map
     */
    public Map<String, PageStatusNotification> getAllStatuses() {
        return Map.copyOf(statusMap);
    }

    /**
     * Clears all stored statuses.
     *
     * Used for testing and cleanup.
     * Phase-6.3: Simple clear.
     */
    public void clearStatuses() {
        statusMap.clear();
        System.out.println("[CefStatusCallback] Cleared all stored statuses");
    }

    /**
     * Phase-6.3 Constraints:
     *
     * - WIRING ONLY: This service only logs and stores status.
     *   NO decision-making or retry logic based on status.
     *
     * - NON-BLOCKING: All operations are fast and return immediately.
     *   No blocking waits, no long computations.
     *
     * - THREAD-SAFE: Uses ConcurrentHashMap for status storage.
     *   Can be called concurrently from multiple threads.
     *
     * - TOLERATES DUPLICATES: If CEF sends the same status twice,
     *   we log it twice and update the map. This is intentional.
     *
     * - TOLERATES OUT-OF-ORDER: No ordering guarantees.
     *   We store whatever we receive.
     *
     * - NO AUTHENTICATION: Phase-7 feature.
     *   We accept all status notifications without validation.
     *
     * - NO ENCRYPTION: Phase-7 feature.
     *   Status messages are sent in plain text.
     *
     * - SIMPLE LOGGING: Console output only.
     *   Phase-7 will add structured logging framework.
     */
}
