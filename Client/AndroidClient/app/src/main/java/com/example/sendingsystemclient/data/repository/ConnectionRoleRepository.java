package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.ConnectionRole;

import java.util.List;

/** Repository interface for ConnectionRole rows. */
public interface ConnectionRoleRepository {
    List<ConnectionRole> getAll();
    ConnectionRole getById(int id);
}
