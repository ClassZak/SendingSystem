package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.NamedConnection;

import java.util.List;

public interface NamedConnectionRepository {
    List<NamedConnection> getAll();
    NamedConnection getById(int id);
    void insert(NamedConnection connection);
}
