package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.ConnectionRole;
import com.example.sendingsystemclient.data.source.ConnectionRoleDao;

import java.util.List;

/** Repository implementation backed by a Room DAO. */
public class ConnectionRoleRepositoryDaoImpl implements ConnectionRoleRepository {
    private final ConnectionRoleDao dao;
    public ConnectionRoleRepositoryDaoImpl(ConnectionRoleDao dao) {
        this.dao = dao;
    }


    @Override
    public List<ConnectionRole> getAll() {
        return dao.getAll();
    }

    @Override
    public ConnectionRole getById(int id) {
        return dao.getById(id);
    }
}
