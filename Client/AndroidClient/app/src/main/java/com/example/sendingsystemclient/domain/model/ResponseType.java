package com.example.sendingsystemclient.domain.model;

/** Server response kinds: "error" or "message". */
public enum ResponseType {
    Error (1),
    Message (2);

    private final int index;

    ResponseType(int index) {
        this.index = index;
    }

    public int index() {
        return this.index;
    }
}
