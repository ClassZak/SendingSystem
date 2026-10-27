package com.example.sendingsystemclient.domain.viewmodel;

import android.content.ContentResolver;
import android.net.Uri;

import androidx.annotation.Nullable;

import com.example.sendingsystemclient.App;
import com.example.sendingsystemclient.data.model.Connection;
import com.example.sendingsystemclient.data.model.Message;
import com.example.sendingsystemclient.domain.model.ConnectionRole;
import com.example.sendingsystemclient.domain.model.Connector;
import com.example.sendingsystemclient.domain.model.IPVersion;
import com.example.sendingsystemclient.domain.model.MessageDirection;
import com.example.sendingsystemclient.domain.model.MessageStatus;
import com.example.sendingsystemclient.domain.model.SendingType;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Locale;

/**
 * Orchestrates a single send operation.
 *
 * Pipeline: validate → ensure Connection row → delegate to Connector
 * → persist Message (success or failure). The method is synchronous;
 * callers must run it off the UI thread.
 *
 * @see Connector
 * @see Config
 */
public class SendDataViewModel {

    /**
     * Bundle of parameters for one send operation.
     *
     * Consolidates the former 7-argument method signature and adds
     * fields required by the streaming file path (fileUri,
     * contentResolver) and optional progress reporting
     * (progressListener).
     */
    public static class Config {
        /** IP protocol (used only for validation). */
        public IPVersion ipVersion          = IPVersion.IPv4;
        /** Server address. */
        public String     serverIp          = "";
        /** Server port as string (validated in SendData). */
        public String     portString        = "";
        /** Selects the endpoint / encoding. */
        public SendingType sendingType      = SendingType.RAW_DATA;
        /** Data for RAW_DATA mode. */
        public String     sendingData       = "";
        /** Use HTTPS/WSS. */
        public boolean    isSSLEncryptEnabled = false;
        /** Optional server-side file name (sanitised). */
        public String     saveToPath        = "";

        /** In-memory file content (small files only). */
        @Nullable public byte[]          fileBytes;
        /** Suggested multipart file name. */
        @Nullable public String          fileName;
        /** Streaming file source (preferred for large files). */
        @Nullable public Uri             fileUri;
        /** Resolver for fileUri. */
        @Nullable public ContentResolver contentResolver;

        /** Optional progress callback. Leave null to skip. */
        @Nullable public Connector.ProgressListener progressListener;

        /** TLS trust policy. */
        public Connector.TrustMode trustMode = Connector.TrustMode.TRUST_ALL;
        /** SHA-256 SPKI pin, required for TrustMode.PINNED. */
        @Nullable public String    pinnedSha256;
    }

    /** Single reusable connector instance. */
    private final Connector connector = new Connector();
    private final App app;
    private volatile boolean isSending = false;

    /**
     * @param app (App, Required) — application instance providing repositories.
     */
    public SendDataViewModel(App app) {
        this.app = app;
    }

    /**
     * @return (boolean) — true while a send operation is in progress.
     */
    public boolean getIsSending() {
        return this.isSending || connector.getIsSending();
    }

    /**
     * Sends the payload described by cfg and persists the result.
     *
     * Steps:
     *   1. Validate the port and the IP address.
     *   2. Ensure a Connection row exists for (ip, port, TLS-flag);
     *      insert one if not.
     *   3. Configure the shared Connector.
     *   4. Call Connector.sendData() synchronously.
     *   5. Persist a Message with status SUCCESS or FAILED depending on
     *      the outcome.
     *
     * @param cfg (Config, Required) — fully-populated configuration.
     * @return (String) — raw JSON response body from the server.
     * @throws Exception if the port/IP are invalid, send is already
     *         in progress, or the transport layer throws.
     */
    public String SendData(Config cfg) throws Exception {
        if (getIsSending()) {
            throw new Exception("Data sending right now. Please, wait.");
        }

        int serverPort;
        try {
            serverPort = Integer.parseInt(cfg.portString);
            if (serverPort < 1 || serverPort > 65535) {
                throw new Exception("The port must be between 1 and 65535");
            }
        } catch (NumberFormatException e) {
            throw new NumberFormatException("Wrong port");
        }

        if (!Connector.isValidInetAddress(cfg.serverIp, cfg.ipVersion)) {
            throw new Exception("Wrong IP-address: " + cfg.serverIp);
        }

        // Ensure a Connection row exists (upsert by IP+port+TLS flag).
        Connection conn = app.getConnectionRepository()
                .getByIpAndPortAndIsSSLEncryptedAndConnectionRoleId(cfg.serverIp, serverPort, cfg.isSSLEncryptEnabled, ConnectionRole.SERVER.index());
        if (conn == null) {
            app.getConnectionRepository().insert(
                    new Connection(serverPort, cfg.serverIp, cfg.isSSLEncryptEnabled, ConnectionRole.SERVER.index()));
            conn = app.getConnectionRepository()
                    .getByIpAndPortAndIsSSLEncryptedAndConnectionRoleId(cfg.serverIp, serverPort, cfg.isSSLEncryptEnabled, ConnectionRole.SERVER.index());
        }

        SimpleDateFormat sdf = new SimpleDateFormat(
                "yyyy-MM-dd HH:mm:ss", Locale.getDefault());
        Message message = new Message(
                conn.id,
                MessageStatus.FAILED.index(),
                cfg.sendingType.index(),
                MessageDirection.OUTBOUND.index(),
                "",
                sdf.format(new Date(System.currentTimeMillis())),
                cfg.sendingType == SendingType.FILE ? "" : cfg.sendingData);

        // Configure the connector for this call.
        connector.serverIp            = cfg.serverIp;
        connector.serverPort          = serverPort;
        connector.sendingType         = cfg.sendingType;
        connector.sendingData         = cfg.sendingData;
        connector.ipVersion           = cfg.ipVersion;
        connector.isSSLEncryptEnabled = cfg.isSSLEncryptEnabled;
        connector.setTrustMode(cfg.trustMode);
        connector.setPinnedSha256(cfg.pinnedSha256);
        connector.setProgressListener(cfg.progressListener);

        if (cfg.saveToPath == null || cfg.saveToPath.isEmpty()) {
            connector.setSaveToPath(null);
        } else {
            connector.setSaveToPath(Connector.sanitizeFilename(cfg.saveToPath));
        }

        if (cfg.fileUri != null && cfg.contentResolver != null) {
            connector.setFileUri(cfg.contentResolver, cfg.fileUri, cfg.fileName);
        } else if (cfg.fileBytes != null) {
            connector.setFileBytes(cfg.fileBytes, cfg.fileName);
        } else {
            connector.clearFile();
        }

        this.isSending = true;
        String response;
        try {
            response = connector.sendData();
            message.response = response != null ? response : "";
            message.messageStatusId = MessageStatus.SUCCESS.index();
        } finally {
            this.isSending = false;
            // Persist regardless of success/failure — history stays complete.
            app.getMessageRepository().insert(message);
        }
        return response;
    }
}