package com.example.sendingsystemclient.data.source;

import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.Query;

import com.example.sendingsystemclient.data.model.Connection;

import java.util.List;

@Dao
public interface ConnectionDao {
    @Insert
    void insert(Connection connection);
    @Insert
    void insertAll(List<Connection> connections);

    @Query("SELECT * FROM Connection")
    List<Connection> getAll();

    @Query("SELECT * FROM Connection WHERE id = :id")
    Connection getById(int id);

    @Query("SELECT * FROM Connection WHERE ip = :ip AND port = :port AND isSSLEncrypted = :isSSLEncrypted LIMIT 1")
    Connection getByIpAndPortAndIsSSLEncrypted(String ip, int port, boolean isSSLEncrypted);
}
