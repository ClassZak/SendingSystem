package com.example.sendingsystemclient.data.model;

import androidx.room.Entity;
import androidx.room.ForeignKey;
import androidx.room.PrimaryKey;

/** Connection endpoint: IP, port, TLS flag. Mapped to Connection table. */
@Entity(
        tableName = "Connection",
        foreignKeys = {
                @ForeignKey(
                        entity = ConnectionRole.class,
                        parentColumns = "id",
                        childColumns = "roleId"
                )
        }
)
public class Connection {
    @PrimaryKey(autoGenerate = true)
    public int id;
    public String ip;
    public int port;
    public boolean isSSLEncrypted;
    public int roleId;


    public Connection(){
    }
    public Connection(int port, String ip, boolean isSSLEncrypted, int roleId) {
        this.ip = ip;
        this.port = port;
        this.isSSLEncrypted = isSSLEncrypted;
        this.roleId = roleId;
    }
}
