package com.example.sendingsystemclient.data.model;

import androidx.room.Entity;
import androidx.room.ForeignKey;
import androidx.room.PrimaryKey;

/** One send attempt: request, response, timing, status and type references. */
@Entity(
        tableName = "Message",
        foreignKeys = {
                @ForeignKey(
                        entity = Connection.class,
                        parentColumns = "id",
                        childColumns = "connectionId"
                ),
                @ForeignKey(
                        entity = MessageStatus.class,
                        parentColumns = "id",
                        childColumns = "messageStatusId"
                ),
                @ForeignKey(
                        entity = MessageType.class,
                        parentColumns = "id",
                        childColumns = "messageTypeId"
                ),
                @ForeignKey(
                        entity = MessageDirection.class,
                        parentColumns = "id",
                        childColumns = "directionId"
                )
        }
)
public class Message {
    @PrimaryKey(autoGenerate = true)
    public int id;
    public int connectionId;
    public int messageStatusId;
    public int messageTypeId;
    public int directionId;
    public String response;
    public String sendingTime;
    public String request;

    public Message(
            int connectionId,
            int messageStatusId,
            int messageTypeId,
            int directionId,
            String response,
            String sendingTime,
            String request
    ) {
        this.connectionId = connectionId;
        this.messageStatusId = messageStatusId;
        this.messageTypeId = messageTypeId;
        this.directionId = directionId;
        this.response = response;
        this.sendingTime = sendingTime;
        this.request = request;
    }
}
