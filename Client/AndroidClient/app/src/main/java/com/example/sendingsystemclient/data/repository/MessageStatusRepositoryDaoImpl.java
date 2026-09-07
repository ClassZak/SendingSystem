package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.MessageStatus;
import com.example.sendingsystemclient.data.source.MessageDao;
import com.example.sendingsystemclient.data.source.MessageStatusDao;

import java.util.Collections;
import java.util.List;

public class MessageStatusRepositoryDaoImpl implements MessageStatusRepository {
    private final MessageStatusDao dao;
    public MessageStatusRepositoryDaoImpl(MessageStatusDao dao) {
        this.dao = dao;
    }


    @Override
    public List<MessageStatus> getAll() {
        return dao.getAll();
    }

    @Override
    public MessageStatus getById(int id) {
        return dao.getById(id);
    }
}
