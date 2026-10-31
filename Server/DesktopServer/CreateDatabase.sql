-- Sending System schema.
-- Column names match the Android Room entities exactly.
-- Applied on every start; every statement is idempotent.




PRAGMA foreign_keys = ON;




-- ============================================================
-- Reference tables
-- ============================================================
CREATE TABLE IF NOT EXISTS MessageType(
	id   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	name TEXT NOT NULL
);




CREATE TABLE IF NOT EXISTS MessageStatus(
	id   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	name TEXT NOT NULL
);




CREATE TABLE IF NOT EXISTS MessageDirection(
	id   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	name TEXT NOT NULL
);




CREATE TABLE IF NOT EXISTS ConnectionRole(
	id   INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	name TEXT NOT NULL
);




-- ============================================================
-- Data tables
-- ============================================================
CREATE TABLE IF NOT EXISTS Connection(
	id				INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	ip				TEXT,
	-- Port of the *listening* side of this link. Never the ephemeral
	-- source port of an inbound connection: that one is random per
	-- TCP handshake and carries no identity. For outbound rows it is
	-- the server's port we connected to; for inbound rows it is our
	-- own listening port.
	port			INTEGER NOT NULL,
	isSSLEncrypted	INTEGER NOT NULL,
	roleId			INTEGER NOT NULL,
	FOREIGN KEY(roleId)		 REFERENCES ConnectionRole(id)
);
CREATE UNIQUE INDEX IF NOT EXISTS ux_Connection_peer
	ON Connection(roleId, IFNULL(ip, ''), port, isSSLEncrypted);




CREATE TABLE IF NOT EXISTS Message(
	id				INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	connectionId	INTEGER NOT NULL,
	messageStatusId INTEGER NOT NULL,
	messageTypeId   INTEGER NOT NULL,
	directionId		INTEGER NOT NULL,
	response		TEXT,
	sendingTime		TEXT,
	request			TEXT,
	FOREIGN KEY(connectionId)	 REFERENCES Connection(id),
	FOREIGN KEY(messageStatusId) REFERENCES MessageStatus(id),
	FOREIGN KEY(messageTypeId)	 REFERENCES MessageType(id),
	FOREIGN KEY(directionId)	 REFERENCES MessageDirection(id)
);




CREATE INDEX IF NOT EXISTS index_Message_connectionId	 ON Message(connectionId);
CREATE INDEX IF NOT EXISTS index_Message_messageStatusId ON Message(messageStatusId);
CREATE INDEX IF NOT EXISTS index_Message_messageTypeId	 ON Message(messageTypeId);
CREATE INDEX IF NOT EXISTS index_Message_directionId	 ON Message(directionId);
CREATE INDEX IF NOT EXISTS index_Message_sendingTime	 ON Message(sendingTime);




CREATE TABLE IF NOT EXISTS NamedConnection(
	id			 INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,
	name		 TEXT,
	connectionId INTEGER NOT NULL,
	FOREIGN KEY(connectionId) REFERENCES Connection(id)
);




CREATE INDEX IF NOT EXISTS index_NamedConnection_connectionId ON NamedConnection(connectionId);




-- ============================================================
-- Seed values
-- ============================================================
-- Message type
INSERT OR IGNORE INTO MessageType(id, name) VALUES (1, 'RAW_DATA');
INSERT OR IGNORE INTO MessageType(id, name) VALUES (2, 'FILE');
-- Message status
INSERT OR IGNORE INTO MessageStatus(id, name) VALUES (1, 'SUCCESS');
INSERT OR IGNORE INTO MessageStatus(id, name) VALUES (2, 'FAILED');
-- Message direction
insert or ignore into messagedirection(id, name) values (1, 'OUTBOUND');
insert or ignore into mESSAGEdIRECTION(ID, NAME) values (2, 'INBOUND');
-- Connection role
INSERT OR IGNORE INTO ConnectionRole(id, name) VALUES (1, 'SERVER');
INSERT OR IGNORE INTO ConnectionRole(id, name) VALUES (2, 'CLIENT');
INSERT OR IGNORE INTO ConnectionRole(id, name) VALUES (3, 'MASTER');
INSERT OR IGNORE INTO ConnectionRole(id, name) VALUES (4, 'SLAVE');
