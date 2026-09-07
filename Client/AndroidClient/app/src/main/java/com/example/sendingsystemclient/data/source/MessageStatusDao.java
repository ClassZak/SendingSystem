package com.example.sendingsystemclient.data.source;

import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.Query;

import com.example.sendingsystemclient.data.model.MessageStatus;

import java.util.List;

@Dao
public interface MessageStatusDao {
    @Insert
    void insert(MessageStatus messageStatus);
    @Insert
    void insertAll(List<MessageStatus> list);

    @Query("SELECT * FROM MessageStatus")
    List<MessageStatus> getAll();

    @Query("SELECT * FROM MessageStatus WHERE id = :id")
    MessageStatus getById(int id);
}
