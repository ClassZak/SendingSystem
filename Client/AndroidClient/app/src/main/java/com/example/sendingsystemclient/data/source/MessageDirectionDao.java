package com.example.sendingsystemclient.data.source;

import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.Query;

import com.example.sendingsystemclient.data.model.MessageDirection;

import java.util.List;

@Dao
public interface MessageDirectionDao {
    @Insert
    void insert(MessageDirection messageDirection);
    @Insert
    void insertAll(List<MessageDirection> messageDirections);

    @Query("SELECT * FROM MessageDirection")
    List<MessageDirection> getAll();

    @Query("SELECT * FROM Messagedirection WHERE id = :id")
    MessageDirection getById(int id);
}
