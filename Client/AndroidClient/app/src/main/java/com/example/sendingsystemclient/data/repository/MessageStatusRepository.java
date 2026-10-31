package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.MessageStatus;

import java.util.List;

/** Repository interface for MessageStatus rows. */
public interface MessageStatusRepository {
    List<MessageStatus> getAll();
    MessageStatus getById(int id);
}
