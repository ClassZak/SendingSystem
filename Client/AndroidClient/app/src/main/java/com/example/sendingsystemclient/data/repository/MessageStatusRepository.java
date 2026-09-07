package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.MessageStatus;

import java.util.List;

public interface MessageStatusRepository {
    List<MessageStatus> getAll();
    MessageStatus getById(int id);
}
