package com.example.sendingsystemclient;

import android.app.Application;

import com.example.sendingsystemclient.data.repository.ConnectionRepository;
import com.example.sendingsystemclient.data.repository.ConnectionRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.ConnectionRoleRepository;
import com.example.sendingsystemclient.data.repository.ConnectionRoleRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.MessageDirectionRepository;
import com.example.sendingsystemclient.data.repository.MessageDirectionRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.MessageRepository;
import com.example.sendingsystemclient.data.repository.MessageRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.MessageStatusRepository;
import com.example.sendingsystemclient.data.repository.MessageStatusRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.MessageTypeRepository;
import com.example.sendingsystemclient.data.repository.MessageTypeRepositoryDaoImpl;
import com.example.sendingsystemclient.data.repository.NamedConnectionRepository;
import com.example.sendingsystemclient.data.repository.NamedConnectionRepositoryDaoImpl;
import com.example.sendingsystemclient.data.source.AppDatabase;

/**
 * Application entry point: instantiates every repository once so
 * activities can fetch them through getApplication().
 */
public class App extends Application {
    private MessageStatusRepository messageStatusRepository;
    private MessageTypeRepository messageTypeRepository;
    private ConnectionRepository connectionRepository;
    private ConnectionRoleRepository connectionRoleRepository;
    private NamedConnectionRepository namedConnectionRepository;
    private MessageRepository messageRepository;
    private MessageDirectionRepository messageDirectionRepository;

    /**
     * @return (MessageStatusRepository) — shared instance.
     */
    public MessageStatusRepository getMessageStatusRepository() {
        return messageStatusRepository;
    }

    /**
     * @return (MessageTypeRepository) — shared instance.
     */
    public MessageTypeRepository getMessageTypeRepository() {
        return messageTypeRepository;
    }

    /**
     * @return (ConnectionRepository) — shared instance.
     */
    public ConnectionRepository getConnectionRepository() {
        return connectionRepository;
    }

    /**
     * @return (ConnectionRepository) — shared instance.
     */
    public ConnectionRoleRepository getConnectionRoleRepository() {
        return connectionRoleRepository;
    }

    /**
     * @return (NamedConnectionRepository) — shared instance.
     */
    public NamedConnectionRepository getNamedConnectionRepository() {
        return namedConnectionRepository;
    }

    /**
     * @return (MessageRepository) — shared instance.
     */
    public MessageRepository getMessageRepository() {
        return messageRepository;
    }

    /**
     * @return (MessageRepository) — shared instance.
     */
    public MessageDirectionRepository getMessageDirectionRepository() {
        return messageDirectionRepository;
    }

    @Override
    public void onCreate(){
        super.onCreate();
        AppDatabase db = AppDatabase.getInstance(this);
        messageStatusRepository = new MessageStatusRepositoryDaoImpl(db.messageStatusDao());
        messageTypeRepository = new MessageTypeRepositoryDaoImpl(db.messageTypeDao());
        connectionRepository = new ConnectionRepositoryDaoImpl(db.connectionDao());
        connectionRoleRepository = new ConnectionRoleRepositoryDaoImpl(db.connectionRoleDao());
        namedConnectionRepository = new NamedConnectionRepositoryDaoImpl(db.namedConnectionDao());
        messageRepository = new MessageRepositoryDaoImpl(db.messageDao());
        messageDirectionRepository = new MessageDirectionRepositoryDaoImpl(db.messageDirectionDao());
    }
}