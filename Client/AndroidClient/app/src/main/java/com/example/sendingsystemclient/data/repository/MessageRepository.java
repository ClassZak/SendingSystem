package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.Message;

import java.util.List;

public interface MessageRepository {
    void insert(Message message);
    List<Message> getAll();
    Message getById(int id);
    Message getLastSuccessful();
    List<Message> getMessagesForConnectionId(int connectionId);
}
