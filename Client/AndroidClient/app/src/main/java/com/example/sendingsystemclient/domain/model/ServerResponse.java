package com.example.sendingsystemclient.domain.model;

import org.json.JSONException;
import org.json.JSONObject;

/**
 * Parsed server reply: kind ("message" or "error") plus the payload.
 *
 * Instances are created only through fromJson; the constructor is
 * private on purpose.
 */
public class ServerResponse {
    public ResponseType type;
    public String data;

    private ServerResponse() {}

    /**
     * Parses a ServerResponse from a JSON string.
     *
     * @param jsonString (String, Required) — JSON with "type" and "data".
     * @return (ServerResponse) — parsed response.
     * @throws JSONException if the string is not valid JSON.
     * @throws IllegalArgumentException if the "type" field is missing
     *         or cannot be mapped to ResponseType.
     */
    public static ServerResponse fromJson(String jsonString) throws JSONException {
        JSONObject json = new JSONObject(jsonString);

        String typeStr = json.optString("type", null);
        ResponseType type = parseResponseType(typeStr);
        if (type == null) {
            throw new IllegalArgumentException("Unknown response type: " + typeStr);
        }

        String data = json.optString("data", null);

        ServerResponse response = new ServerResponse();
        response.type = type;
        response.data = data;
        return response;
    }

    /**
     * Maps the JSON "type" string to a ResponseType.
     *
     * Case-insensitive; matches enum names ("error", "ERROR", "Error",
     * "message", ...).
     *
     * @param typeStr (String, Optional) — value from JSON; null/empty
     *                returns null.
     * @return (ResponseType) — matching enum, or null if unknown.
     */
    private static ResponseType parseResponseType(String typeStr) {
        if (typeStr == null || typeStr.isEmpty()) {
            return null;
        }

        for (ResponseType t : ResponseType.values()) {
            if (t.name().equalsIgnoreCase(typeStr)) {
                return t;
            }
        }
        return null;
    }

    /**
     * Serialises this response back to a JSON string.
     *
     * @return (String) — JSON with "type" and "data" (JSONObject.NULL
     *         when either field is null).
     * @throws JSONException on serialisation failure.
     */
    public String toJson() throws JSONException {
        JSONObject json = new JSONObject();
        json.put("type", type != null ? type.name() : JSONObject.NULL);
        json.put("data", data != null ? data : JSONObject.NULL);
        return json.toString();
    }
}