package com.example.sendingsystemclient;

import android.app.Application;

import com.example.sendingsystemclient.data.repository.ConnectionRepository;
import com.example.sendingsystemclient.data.repository.ConnectionRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.MessageRepository;
import com.example.sendingsystemclient.data.repository.MessageRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.MessageStatusRepository;
import com.example.sendingsystemclient.data.repository.MessageStatusRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.MessageTypeRepository;
import com.example.sendingsystemclient.data.repository.MessageTypeRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.NamedConnectionRepository;
import com.example.sendingsystemclient.data.repository.NamedConnectionRepositoryDaoImpl;
import com.example.sendingsystemclient.data.source.AppDatabase;

public class App extends Application {
    private MessageStatusRepository messageStatusRepository;
    private MessageTypeRepository messageTypeRepository;
    private ConnectionRepository connectionRepository;
    private NamedConnectionRepository namedConnectionRepository;
    private MessageRepository messageRepository;

    public MessageStatusRepository getMessageStatusRepository() {
        return messageStatusRepository;
    }
    public MessageTypeRepository getMessageTypeRepository() {
        return messageTypeRepository;
    }
    public ConnectionRepository getConnectionRepository() {
        return connectionRepository;
    }
    public NamedConnectionRepository getNamedConnectionRepository() {
        return namedConnectionRepository;
    }
    public MessageRepository getMessageRepository() {
        return messageRepository;
    }


    @Override
    public void onCreate(){
        super.onCreate();
        AppDatabase db = AppDatabase.getInstance(this);
        messageStatusRepository = new MessageStatusRepositoryDaoImpl(db.messageStatusDao());
        messageTypeRepository = new MessageTypeRepositoryDaoImpl(db.messageTypeDao());
        connectionRepository = new ConnectionRepositoryDaoImpl(db.connectionDao());
        namedConnectionRepository = new NamedConnectionRepositoryDaoImpl(db.namedConnectionDao());
        messageRepository = new MessageRepositoryDaoImpl(db.messageDao());
    }
}
