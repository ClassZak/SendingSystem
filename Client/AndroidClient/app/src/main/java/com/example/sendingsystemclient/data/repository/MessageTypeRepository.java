package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.MessageType;
import com.example.sendingsystemclient.data.source.MessageTypeDao;

import java.util.List;

public interface MessageTypeRepository {
    List<MessageType> getAll();
    MessageType getById(int id);
}
