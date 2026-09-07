package com.example.sendingsystemclient.domain.viewmodel;

import com.example.sendingsystemclient.App;
import com.example.sendingsystemclient.data.model.Connection;
import com.example.sendingsystemclient.data.model.Message;
import com.example.sendingsystemclient.domain.model.Connector;
import com.example.sendingsystemclient.domain.model.IPVersion;
import com.example.sendingsystemclient.domain.model.MessageStatus;
import com.example.sendingsystemclient.domain.model.SendingType;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;
import java.util.concurrent.atomic.AtomicReference;

public class SendDataViewModel {
    private final Connector connector = new Connector();
    private final App app;
    public SendDataViewModel(App app){
        this.app = app;
    }


    private volatile boolean isSending = false;

    public String SendData(
            IPVersion ipVersion,
            String serverIp,
            String portString,
            SendingType sendingType,
            String sendingData,
            boolean isSSLEncryptEnabled,
            String saveToPath
    ) throws Exception {
        if (getIsSending()){
            throw new Exception("Data sending right now. Please, wait.");
        }

        int serverPort;
        try {
            serverPort = Integer.parseInt(portString);
            if (serverPort < 1 || serverPort > 65535) {
                throw new Exception("The port must be between 1 and 65535");
            }
        } catch (NumberFormatException e) {
            throw new NumberFormatException("Wrong port");
        }

        if (!Connector.isValidInetAddress(serverIp, ipVersion)) {
            throw new Exception("Wrong IP-address: " + serverIp);
        }


        AtomicReference<Connection> messageConnection = new AtomicReference<Connection>(new Connection());
        Thread messsageConnectionThread = new Thread(()-> {
            Connection founded = app.getConnectionRepository().getByIpAndPortAndIsSSLEncrypted(serverIp, serverPort, isSSLEncryptEnabled);
            messageConnection.set(founded);
            if (messageConnection.get() == null) {
                app.getConnectionRepository().insert(new Connection(serverPort, serverIp, isSSLEncryptEnabled));
                founded = app.getConnectionRepository().getByIpAndPortAndIsSSLEncrypted(serverIp, serverPort, isSSLEncryptEnabled);
                messageConnection.set(founded);
            }
        });
        messsageConnectionThread.start();
        messsageConnectionThread.join();

        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss", Locale.getDefault());
        Date currDate = new Date(System.currentTimeMillis());
        AtomicReference<Message> message = new AtomicReference<Message>(new Message(messageConnection.get().id, MessageStatus.FAILED.index(), sendingType.index(), "", sdf.format(currDate), sendingData));
        connector.serverIp = serverIp;
        connector.serverPort = serverPort;
        connector.sendingType = sendingType;
        connector.sendingData = sendingData;
        connector.ipVersion = ipVersion;
        connector.isSSLEncryptEnabled = isSSLEncryptEnabled;
        if (saveToPath == null || saveToPath.isEmpty())
            connector.setSaveToPath(null);
        else
            connector.setSaveToPath(Connector.sanitizeFilename(saveToPath));

        AtomicReference<String> response = new AtomicReference<>("");
        try {
            Thread connectorThread = new Thread(() -> {
                try {
                    response.set(connector.sendData());
                } catch (Exception e) {
                    throw new RuntimeException(e);
                }
                message.get().response = response.get();
                message.get().messageStatusId = MessageStatus.SUCCESS.index();
            });
            connectorThread.start();
            connectorThread.join();
        } finally {
            Thread messageThread = new Thread(() -> {
                this.app.getMessageRepository().insert(message.get());
            });
            messageThread.start();
            messageThread.join();
            this.isSending = false;
        }

        return response.get();
    }

    public boolean getIsSending() {
        return this.isSending || connector.getIsSending();
    }
}
