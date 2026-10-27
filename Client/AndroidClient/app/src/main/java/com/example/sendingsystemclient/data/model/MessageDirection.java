package com.example.sendingsystemclient.data.model;

import androidx.room.Entity;
import androidx.room.PrimaryKey;

/** Reference table for message directions (outbound, inbound). */
@Entity(tableName = "MessageDirection")
public class MessageDirection {
    @PrimaryKey(autoGenerate = true)
    public int id;
    public String name;

    public MessageDirection(String name) {
        this.name = name;
    }
}
