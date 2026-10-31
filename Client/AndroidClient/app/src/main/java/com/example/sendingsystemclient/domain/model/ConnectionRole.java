package com.example.sendingsystemclient.domain.model;

/** Connection roles; index() maps the ConnectionRole table ids */
public enum ConnectionRole {
    SERVER(1),
    CLIENT(2),
    MASTER(3),
    SLAVE(4);

    private final int index;

    ConnectionRole(int index) {
        this.index = index;
    }

    public int index(){
        return this.index;
    }
}
