package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.Connection;
import com.example.sendingsystemclient.data.source.ConnectionDao;

import java.util.Collections;
import java.util.List;

public class ConnectionRepositoryDaoImpl implements ConnectionRepository {
    private final ConnectionDao dao;
    public ConnectionRepositoryDaoImpl(ConnectionDao dao) {
        this.dao = dao;
    }


    @Override
    public List<Connection> getAll() {
        return dao.getAll();
    }

    @Override
    public Connection getById(int id) {
        return dao.getById(id);
    }

    @Override
    public void insert(Connection connection) {
        dao.insert(connection);
    }

    @Override
    public Connection getByIpAndPortAndIsSSLEncrypted(String ip, int port, boolean isSSLEncrypted) {
        return dao.getByIpAndPortAndIsSSLEncrypted(ip, port, isSSLEncrypted);
    }
}
