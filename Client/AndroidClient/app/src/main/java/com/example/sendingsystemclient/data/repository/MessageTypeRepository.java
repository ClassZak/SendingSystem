package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.MessageType;

import java.util.List;

/** Repository interface for MessageType rows. */
public interface MessageTypeRepository {
    List<MessageType> getAll();
    MessageType getById(int id);
}
