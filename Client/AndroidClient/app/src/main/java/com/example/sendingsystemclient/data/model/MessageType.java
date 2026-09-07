package com.example.sendingsystemclient.data.model;

import androidx.room.Entity;
import androidx.room.PrimaryKey;

@Entity(tableName = "MessageType")
public class MessageType {
    @PrimaryKey(autoGenerate = true)
    public int id;
    public String name;

    public MessageType(String name) {
        this.name = name;
    }
}
