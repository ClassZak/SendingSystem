package com.example.sendingsystemclient.data.model;

import androidx.room.Entity;
import androidx.room.PrimaryKey;

@Entity(tableName = "MessageStatus")
public class MessageStatus {
    @PrimaryKey(autoGenerate = true)
    public int id;
    public String name;

    public MessageStatus(String name) {
        this.name = name;
    }
}
