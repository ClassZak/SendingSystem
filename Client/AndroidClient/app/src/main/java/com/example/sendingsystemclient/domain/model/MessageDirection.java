package com.example.sendingsystemclient.domain.model;

/** Message directions; index() maps to the MessageDirection table ids. */
public enum MessageDirection {
    OUTBOUND(1),
    INBOUND(2);

    private final int index;

    MessageDirection(int index) {
        this.index = index;
    }

    public int index() {
        return index;
    }
}
