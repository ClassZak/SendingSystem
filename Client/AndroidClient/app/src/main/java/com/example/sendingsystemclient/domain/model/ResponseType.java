package com.example.sendingsystemclient.domain.model;

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
