package com.example.sendingsystemclient.data.source;

import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.Query;

import com.example.sendingsystemclient.data.model.ConnectionRole;

import java.util.List;

@Dao
public interface ConnectionRoleDao {
    @Insert
    void insert(ConnectionRole connectionRole);
    @Insert
    void insertAll(List<ConnectionRole> connectionRoles);

    @Query("SELECT * FROM ConnectionRole")
    List<ConnectionRole> getAll();

    @Query("SELECT * FROM ConnectionRole WHERE id = :id")
    ConnectionRole getById(int id);
}
