package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.NamedConnection;
import com.example.sendingsystemclient.data.source.NamedConnectionDao;

import java.util.Collections;
import java.util.List;

public class NamedConnectionRepositoryDaoImpl implements NamedConnectionRepository {
    private final NamedConnectionDao dao;
    public NamedConnectionRepositoryDaoImpl(NamedConnectionDao dao) {
        this.dao = dao;
    }


    @Override
    public List<NamedConnection> getAll() {
        return dao.getAll();
    }

    @Override
    public NamedConnection getById(int id) {
        return dao.getById(id);
    }

    @Override
    public void insert(NamedConnection connection) {
        dao.insert(connection);
    }
}
