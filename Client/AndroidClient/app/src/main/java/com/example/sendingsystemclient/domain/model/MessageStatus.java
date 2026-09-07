package com.example.sendingsystemclient.domain.model;

public enum MessageStatus {
    SUCCESS (1),
    FAILED (2);

    private final int index;

    MessageStatus(int index) {
        this.index = index;
    }

    public int index() {
        return index;
    }
}
