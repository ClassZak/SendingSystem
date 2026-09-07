package com.example.sendingsystemclient.data.source;

import android.content.Context;

import androidx.annotation.NonNull;
import androidx.room.Database;
import androidx.room.Room;
import androidx.room.RoomDatabase;
import androidx.sqlite.db.SupportSQLiteDatabase;

import com.example.sendingsystemclient.data.model.Connection;
import com.example.sendingsystemclient.data.model.Message;
import com.example.sendingsystemclient.data.model.MessageStatus;
import com.example.sendingsystemclient.data.model.MessageType;
import com.example.sendingsystemclient.data.model.NamedConnection;

import java.util.Arrays;
import java.util.List;
import java.util.concurrent.Executors;

@Database(entities = {
        MessageType.class,
        MessageStatus.class,
        Message.class,
        Connection.class,
        NamedConnection.class
    }, version = 1)
public abstract class AppDatabase extends RoomDatabase {
    public abstract MessageTypeDao messageTypeDao();
    public abstract MessageStatusDao messageStatusDao();
    public abstract MessageDao messageDao();
    public abstract ConnectionDao connectionDao();
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
                                messageStatusDao.insertAll(messageStatuses);
                                messageTypeDao.insertALL(messageTypes);
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
