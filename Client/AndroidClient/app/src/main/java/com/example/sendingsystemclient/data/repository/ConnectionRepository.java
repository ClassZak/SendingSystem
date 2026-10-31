package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.Connection;

import java.util.List;

/** Repository interface for Connection rows. */
public interface ConnectionRepository {
    List<Connection> getAll();
    Connection getById(int id);
    void insert(Connection connection);
    Connection getByIpAndPortAndIsSSLEncryptedAndConnectionRoleId(String ip, int port, boolean isSSLEncrypted, int roleId);
}
