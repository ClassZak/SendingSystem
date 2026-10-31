package com.example.sendingsystemclient.domain.model;

/** Send operation kinds; selects the endpoint in Connector. */
public enum SendingType {
    RAW_DATA (1),
    FILE (2);

    private final int index;

    SendingType(int index) {
        this.index = index;
    }

    public int index() {
        return index;
    }
}
