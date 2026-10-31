package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.MessageDirection;

import java.util.List;

/** Repository interface for MessageDirection rows. */
public interface MessageDirectionRepository {
    List<MessageDirection> getAll();
    MessageDirection getById(int id);
}
