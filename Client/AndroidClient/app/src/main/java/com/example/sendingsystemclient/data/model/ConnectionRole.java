package com.example.sendingsystemclient.data.model;

import androidx.room.Entity;
import androidx.room.PrimaryKey;

/** Reference table for message directions (server, client, master, slave). */
@Entity(tableName = "ConnectionRole")
public class ConnectionRole {
    @PrimaryKey(autoGenerate = true)
    public int id;
    public String name;

    public ConnectionRole(String name) {
        this.name = name;
    }
}
