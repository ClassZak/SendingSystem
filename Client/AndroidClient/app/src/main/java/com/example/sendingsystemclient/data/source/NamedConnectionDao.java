package com.example.sendingsystemclient.data.source;

import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.Query;

import com.example.sendingsystemclient.data.model.NamedConnection;

import java.util.List;

@Dao
public interface NamedConnectionDao {
    @Insert
    void insert(NamedConnection namedConnection);
    @Insert
    void insertALL(List<NamedConnection> namedConnections);

    @Query("SELECT * FROM NamedConnection")
    List<NamedConnection> getAll();

    @Query("SELECT * FROM NamedConnection WHERE id = :id")
    NamedConnection getById(int id);

    /**
     * Search every query entry from table
     * Necessary to escape argument
     * @param query Search query
     * @return List of found rows
     */
    @Query("SELECT * FROM NamedConnection WHERE LOWER(name) LIKE '%' || LOWER(:query) || '%' ESCAPE '\\'")
    List<NamedConnection> searchNamedConnection(String query);
}
