package com.example.sendingsystemclient.data.source;

import android.content.Context;

import androidx.annotation.NonNull;
import androidx.room.Database;
import androidx.room.Room;
import androidx.room.RoomDatabase;
import androidx.sqlite.db.SupportSQLiteDatabase;

import com.example.sendingsystemclient.data.model.Connection;
import com.example.sendingsystemclient.data.model.ConnectionRole;
import com.example.sendingsystemclient.data.model.Message;
import com.example.sendingsystemclient.data.model.MessageDirection;
import com.example.sendingsystemclient.data.model.MessageStatus;
import com.example.sendingsystemclient.data.model.MessageType;
import com.example.sendingsystemclient.data.model.NamedConnection;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Executors;

/** Room database: five tables matching the C server schema. */
@Database(entities = {
        MessageType.class,
        MessageStatus.class,
        Message.class,
        MessageDirection.class,
        Connection.class,
        ConnectionRole.class,
        NamedConnection.class
    }, version = 1)
public abstract class AppDatabase extends RoomDatabase {
    public abstract MessageTypeDao messageTypeDao();
    public abstract MessageStatusDao messageStatusDao();
    public abstract MessageDao messageDao();
    public abstract MessageDirectionDao messageDirectionDao();
    public abstract ConnectionDao connectionDao();
    public abstract ConnectionRoleDao connectionRoleDao();
    public abstract NamedConnectionDao namedConnectionDao();

    private static volatile AppDatabase INSTANCE;

    public static AppDatabase getInstance(Context context) {
        if (INSTANCE == null) {
            synchronized (AppDatabase.class) {
                if (INSTANCE == null) {
                    RoomDatabase.Callback callback = new RoomDatabase.Callback() {
                        @Override
                        public void onCreate(@NonNull SupportSQLiteDatabase db) {
                            super.onCreate(db);

                            Executors.newSingleThreadExecutor().execute(() -> {
                                AppDatabase appDatabase = INSTANCE;
                                MessageDirectionDao messageDirectionDao =
                                        appDatabase.messageDirectionDao();
                                List<MessageDirection> messageDirections = Arrays.asList(
                                        new MessageDirection("OUTBOUND"),
                                        new MessageDirection("INBOUND")
                                );
                                MessageTypeDao messageTypeDao = appDatabase.messageTypeDao();
                                List<MessageType> messageTypes = Arrays.asList(
                                        new MessageType("RAW_DATA"),
                                        new MessageType("FILE")
                                );
                                MessageStatusDao messageStatusDao = appDatabase.messageStatusDao();
                                List<MessageStatus> messageStatuses = Arrays.asList(
                                        new MessageStatus("SUCCESS"),
                                        new MessageStatus("FAILED")
                                );
                                ConnectionRoleDao connectionRoleDao = appDatabase.connectionRoleDao();
                                List<ConnectionRole> connectionRoles = Arrays.asList(
                                        new ConnectionRole("SERVER"),
                                        new ConnectionRole("CLIENT"),
                                        new ConnectionRole("MASTER"),
                                        new ConnectionRole("SLAVE")
                                );
                                messageStatusDao.insertAll(messageStatuses);
                                messageTypeDao.insertAll(messageTypes);
                                messageDirectionDao.insertAll(messageDirections);
                                connectionRoleDao.insertAll(connectionRoles);
                            });
                        }
                    };
                    INSTANCE = Room.databaseBuilder(context.getApplicationContext(),
                                    AppDatabase.class, "app_database")
                            .addCallback(callback)
                            .build();
                }
            }
        }

        return INSTANCE;
    }
}
