package com.example.sendingsystemclient.domain.model;

import org.json.JSONException;
import org.json.JSONObject;

public class ServerResponse {
    public ResponseType type;
    public String data;

    // Private constructor to ensure objects are only created via fromJson
    private ServerResponse() {}

    /**
     * Creates a ServerResponse from a JSON string.
     *
     * @param jsonString JSON string containing the "type" and "data" fields.
     * @return A ready ServerResponse object.
     * @throws JSONException            If the string is not valid JSON.
     * @throws IllegalArgumentException If the "type" field is missing or cannot be mapped to ResponseType.
     */
    public static ServerResponse fromJson(String jsonString) throws JSONException {
        JSONObject json = new JSONObject(jsonString);

        // Get the type string (may absent)
        String typeStr = json.optString("type", null);
        ResponseType type = parseResponseType(typeStr);
        if (type == null) {
            throw new IllegalArgumentException("Unknown response type: " + typeStr);
        }

        // Get the data (may null)
        String data = json.optString("data", null);

        ServerResponse response = new ServerResponse();
        response.type = type;
        response.data = data;
        return response;
    }

    /**
     * Maps the string representation of the type to a ResponseType enum member.
     * Supports enum names case-insensitively ("Error", "MESSAGE", "message", etc.);
     *
     * @param typeStr The string from JSON.
     * @return The corresponding ResponseType or null if no match is found.
     */
    private static ResponseType parseResponseType(String typeStr) {
        if (typeStr == null || typeStr.isEmpty()) {
            return null;
        }

        // Try to match by enum name (case-insensitive)
        for (ResponseType t : ResponseType.values()) {
            if (t.name().equalsIgnoreCase(typeStr)) {
                return t;
            }
        }

        // Could not recognize
        return null;
    }

    // Optional: method for reverse conversion to JSON (for symmetry)
    public String toJson() throws JSONException {
        JSONObject json = new JSONObject();
        json.put("type", type != null ? type.name() : JSONObject.NULL);
        json.put("data", data != null ? data : JSONObject.NULL);
        return json.toString();
    }
}


