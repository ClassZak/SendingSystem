package com.example.sendingsystemclient.data.model;

import androidx.room.Entity;
import androidx.room.ForeignKey;
import androidx.room.Index;
import androidx.room.PrimaryKey;

@Entity(
        tableName = "NamedConnection",
        foreignKeys = @ForeignKey(
                entity = Connection.class,
                parentColumns = "id",
                childColumns = "connectionId"
        ),
        indices = @Index("connectionId")
)
public class NamedConnection {
    @PrimaryKey(autoGenerate = true)
    public int id;
    public String name;
    public int connectionId;
}
