package com.ui.cef_control.ipc;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * Immutable control command value object.
 * 
 * Represents a command to be sent to the CEF process.
 * All fields are final and the class is final to ensure immutability.
 * 
 * Constraints:
 * - commandId must be non-null and non-blank
 * - type must be non-null
 * - payload is nullable (optional)
 * - timestamp is opaque metadata set automatically at construction time
 * 
 * This is a pure value object with no implementation logic.
 */
public final class ControlCommand {

    private final String commandId;
    private final ControlCommandType type;
    private final Map<String, String> payload;
    private final long timestamp;

    /**
     * Creates a new immutable ControlCommand.
     * Timestamp is set automatically to the current time.
     * 
     * @param commandId unique identifier for this command (must be non-null and
     *                  non-blank)
     * @param type      the command type (must be non-null)
     * @param payload   optional key-value payload data (nullable)
     * @throws IllegalArgumentException if commandId is null or blank
     * @throws NullPointerException     if type is null
     */
    public ControlCommand(String commandId, ControlCommandType type, Map<String, String> payload) {
        this(commandId, type, payload, System.currentTimeMillis());
    }

    /**
     * Package-private constructor for testing with explicit timestamp.
     * 
     * @param commandId unique identifier for this command (must be non-null and
     *                  non-blank)
     * @param type      the command type (must be non-null)
     * @param payload   optional key-value payload data (nullable)
     * @param timestamp explicit timestamp (for testing only)
     * @throws IllegalArgumentException if commandId is null or blank
     * @throws NullPointerException     if type is null
     */
    ControlCommand(String commandId, ControlCommandType type, Map<String, String> payload, long timestamp) {
        if (commandId == null || commandId.isBlank()) {
            throw new IllegalArgumentException("commandId must be non-empty");
        }
        if (type == null) {
            throw new NullPointerException("type cannot be null");
        }

        this.commandId = commandId;
        this.type = type;
        // Defensive copy to ensure immutability
        this.payload = payload == null ? null : Collections.unmodifiableMap(new HashMap<>(payload));
        this.timestamp = timestamp;
    }

    /**
     * Gets the command ID.
     * 
     * @return the unique command identifier
     */
    public String getCommandId() {
        return commandId;
    }

    /**
     * Gets the command type.
     * 
     * @return the command type
     */
    public ControlCommandType getType() {
        return type;
    }

    /**
     * Gets the payload.
     * 
     * @return an unmodifiable map of payload data, or null if no payload
     */
    public Map<String, String> getPayload() {
        return payload;
    }

    /**
     * Gets the timestamp (opaque metadata).
     * This is set automatically at construction time and should not be used for
     * logic.
     * 
     * @return the timestamp in milliseconds since epoch
     */
    public long getTimestamp() {
        return timestamp;
    }
}
