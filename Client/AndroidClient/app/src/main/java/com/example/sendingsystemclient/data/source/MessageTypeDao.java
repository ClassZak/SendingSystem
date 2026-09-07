package com.example.sendingsystemclient.data.source;

import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.Query;

import com.example.sendingsystemclient.data.model.MessageType;

import java.util.List;

@Dao
public interface MessageTypeDao {
    @Insert
    void insert(MessageType messageType);
    @Insert
    void insertALL(List<MessageType> list);

    @Query("SELECT * FROM MessageType")
    List<MessageType> getAll();

    @Query("SELECT * FROM MessageType WHERE id = :id")
    MessageType getById(int id);
}
