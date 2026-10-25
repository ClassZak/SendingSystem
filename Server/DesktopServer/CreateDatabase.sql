-- Sending System schema.
-- Column names match the Android Room entities exactly.
-- Applied on every start; every statement is idempotent.




PRAGMA foreign_keys = ON;




CREATE TABLE IF NOT EXISTS MessageType(
    id   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name TEXT NOT NULL
);




CREATE TABLE IF NOT EXISTS MessageStatus(
    id   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name TEXT NOT NULL
);




CREATE TABLE IF NOT EXISTS Connection(
    id             INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    ip             TEXT,
    port           INTEGER NOT NULL,
    isSSLEncrypted INTEGER NOT NULL
);




CREATE TABLE IF NOT EXISTS Message(
    id              INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    connectionId    INTEGER NOT NULL,
    messageStatusId INTEGER NOT NULL,
    messageTypeId   INTEGER NOT NULL,
    response        TEXT,
    sendingTime     TEXT,
    request         TEXT,
    FOREIGN KEY(connectionId)    REFERENCES Connection(id),
    FOREIGN KEY(messageStatusId) REFERENCES MessageStatus(id),
    FOREIGN KEY(messageTypeId)   REFERENCES MessageType(id)
);




CREATE INDEX IF NOT EXISTS index_Message_connectionId    ON Message(connectionId);
CREATE INDEX IF NOT EXISTS index_Message_messageStatusId ON Message(messageStatusId);
CREATE INDEX IF NOT EXISTS index_Message_messageTypeId   ON Message(messageTypeId);




CREATE TABLE IF NOT EXISTS NamedConnection(
    id           INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
    name         TEXT,
    connectionId INTEGER NOT NULL,
    FOREIGN KEY(connectionId) REFERENCES Connection(id)
);




CREATE INDEX IF NOT EXISTS index_NamedConnection_connectionId ON NamedConnection(connectionId);




-- Seed values must match MessageType.index()/MessageStatus.index()
-- in the Android client.
INSERT OR IGNORE INTO MessageType(id, name)   VALUES (1, 'RAW_DATA');
INSERT OR IGNORE INTO MessageType(id, name)   VALUES (2, 'FILE');

INSERT OR IGNORE INTO MessageStatus(id, name) VALUES (1, 'FAILED');
INSERT OR IGNORE INTO MessageStatus(id, name) VALUES (2, 'SUCCESS');
