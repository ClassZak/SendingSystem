package com.example.sendingsystemclient.data.source;

import androidx.room.Dao;
import androidx.room.Insert;
import androidx.room.Query;

import com.example.sendingsystemclient.data.model.Message;

import java.util.List;

@Dao
public interface MessageDao {
    @Insert
    void insert(Message message);
    @Insert
    void insertAll(List<Message> messages);

    @Query("SELECT * FROM Message")
    List<Message> getAll();

    @Query("SELECT * FROM Message WHERE id = :id")
    Message getById(int id);

    @Query("SELECT * FROM Message WHERE messageStatusId = 1 ORDER BY sendingTime DESC LIMIT 1")
    Message getLastSuccessful();

    @Query("SELECT * FROM Message WHERE connectionId = :connectionId")
    List<Message> getMessagesForConnectionId(int connectionId);
}
