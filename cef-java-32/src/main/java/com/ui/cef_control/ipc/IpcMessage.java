package com.ui.cef_control.ipc;

import org.json.simple.JSONObject;
import org.json.simple.JSONValue;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;

public class IpcMessage {
    private final String commandId;
    private final String type;
    private final JSONObject payload;
    private final long timestamp;

    public IpcMessage(String commandId, String type, JSONObject payload) {
        this.commandId = commandId;
        this.type = type;
        this.payload = payload != null ? payload : new JSONObject();
        this.timestamp = System.currentTimeMillis();
    }

    public static IpcMessage fromJson(String json) throws ParseException {
        JSONParser parser = new JSONParser();
        JSONObject obj = (JSONObject) parser.parse(json);

        if (!obj.containsKey("commandId") || !obj.containsKey("type")) {
            throw new IllegalArgumentException("Missing required envelope fields: commandId, type");
        }

        String commandId = (String) obj.get("commandId");
        String type = (String) obj.get("type");
        JSONObject payload = (JSONObject) obj.get("payload");

        if (payload == null) {
            payload = new JSONObject();
        }

        return new IpcMessage(commandId, type, payload);
    }

    public String toJson() {
        JSONObject envelope = new JSONObject();
        envelope.put("commandId", commandId);
        envelope.put("type", type);
        envelope.put("timestamp", timestamp);
        envelope.put("payload", payload);
        return envelope.toJSONString();
    }

    public String getCommandId() {
        return commandId;
    }

    public String getType() {
        return type;
    }

    public JSONObject getPayload() {
        return payload;
    }

    public long getTimestamp() {
        return timestamp;
    }

    @Override
    public String toString() {
        return "IpcMessage{" +
                "commandId='" + commandId + '\'' +
                ", type='" + type + '\'' +
                ", timestamp=" + timestamp +
                '}';
    }
}

