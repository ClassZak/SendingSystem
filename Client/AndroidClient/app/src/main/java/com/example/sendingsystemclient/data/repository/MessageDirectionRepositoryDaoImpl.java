package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.MessageDirection;
import com.example.sendingsystemclient.data.source.MessageDirectionDao;

import java.util.List;

/** Repository implementation backed by a Room DAO. */
public class MessageDirectionRepositoryDaoImpl implements MessageDirectionRepository{
    private final MessageDirectionDao dao;
    public MessageDirectionRepositoryDaoImpl(MessageDirectionDao dao) {
        this.dao = dao;
    }


    @Override
    public List<MessageDirection> getAll() {
        return dao.getAll();
    }

    @Override
    public MessageDirection getById(int id) {
        return dao.getById(id);
    }
}
