package com.example.sendingsystemclient.data.model;

import androidx.room.Entity;
import androidx.room.PrimaryKey;

@Entity(tableName = "Connection")
public class Connection {
    @PrimaryKey(autoGenerate = true)
    public int id;
    public String ip;
    public int port;
    public boolean isSSLEncrypted;


    public Connection(){
    }
    public Connection(int port, String ip, boolean isSSLEncrypted) {
        this.ip = ip;
        this.port = port;
        this.isSSLEncrypted = isSSLEncrypted;
    }
}
