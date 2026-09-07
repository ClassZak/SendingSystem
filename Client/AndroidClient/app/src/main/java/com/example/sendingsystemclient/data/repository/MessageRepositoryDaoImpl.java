package com.example.sendingsystemclient.data.repository;

import com.example.sendingsystemclient.data.model.Message;
import com.example.sendingsystemclient.data.source.MessageDao;

import java.util.Collections;
import java.util.List;

public class MessageRepositoryDaoImpl implements MessageRepository {
	private final MessageDao dao;
	public MessageRepositoryDaoImpl(MessageDao dao) {
		this.dao = dao;
	}


	@Override
	public void insert(Message message) {
		dao.insert(message);
	}

	@Override
	public List<Message> getAll() {
		return dao.getAll();
	}

	@Override
	public Message getById(int id) {
		return dao.getById(id);
	}

	@Override
	public Message getLastSuccessful() {
		return dao.getLastSuccessful();
	}

	@Override
	public List<Message> getMessagesForConnectionId(int connectionId) {
		return dao.getMessagesForConnectionId(connectionId);
	}
}
