package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.Connection;

import java.util.List;

public interface ConnectionRepository {
    List<Connection> getAll();
    Connection getById(int id);
    void insert(Connection connection);
    Connection getByIpAndPortAndIsSSLEncrypted(String ip, int port, boolean isSSLEncrypted);
}
