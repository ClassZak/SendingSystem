package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.MessageType;
import com.example.sendingsystemclient.data.source.MessageDao;
import com.example.sendingsystemclient.data.source.MessageTypeDao;

import java.util.Collections;
import java.util.List;

public class MessageTypeRepositoryDaoImpl implements MessageTypeRepository {
    private final MessageTypeDao dao;
    public MessageTypeRepositoryDaoImpl(MessageTypeDao dao) {
        this.dao = dao;
    }


    @Override
    public List<MessageType> getAll() {
        return dao.getAll();
    }

    @Override
    public MessageType getById(int id) {
        return dao.getById(id);
    }
}
