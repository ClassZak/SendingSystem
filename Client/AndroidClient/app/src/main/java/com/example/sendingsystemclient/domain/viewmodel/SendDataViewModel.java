package com.example.sendingsystemclient.domain.viewmodel;

import android.content.ContentResolver;
import android.net.Uri;

import androidx.annotation.Nullable;

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

/**
 * @brief Orchestrates a single send operation.
 *
 * @details Pipeline: validate → ensure {@link Connection} row →
 * delegate to {@link Connector} → persist {@link Message} (success or
 * failure). The method is synchronous; callers must run it off the UI
 * thread.
 *
 * @see Connector
 * @see Config
 */
public class SendDataViewModel {

    /**
     * @brief Bundle of parameters for one send operation.
     *
     * @details Consolidates the former 7-argument method signature and
     * adds fields required by the streaming file path
     * ({@link #fileUri}, {@link #contentResolver}) and optional
     * progress reporting ({@link #progressListener}).
     */
    public static class Config {
        /** @brief IP protocol (used only for validation). */
        public IPVersion ipVersion          = IPVersion.IPv4;
        /** @brief Server address. */
        public String     serverIp          = "";
        /** @brief Server port as string (validated in {@link #SendData}). */
        public String     portString        = "";
        /** @brief Selects the endpoint / encoding. */
        public SendingType sendingType      = SendingType.RAW_DATA;
        /** @brief Data for {@code RAW_DATA} mode. */
        public String     sendingData       = "";
        /** @brief Use HTTPS/WSS. */
        public boolean    isSSLEncryptEnabled = false;
        /** @brief Optional server-side file name (sanitised). */
        public String     saveToPath        = "";

        /** @brief In-memory file content (small files only). */
        @Nullable public byte[]          fileBytes;
        /** @brief Suggested multipart file name. */
        @Nullable public String          fileName;
        /** @brief Streaming file source (preferred for large files). */
        @Nullable public Uri             fileUri;
        /** @brief Resolver for {@link #fileUri}. */
        @Nullable public ContentResolver contentResolver;

        /** @brief Optional progress callback. Leave {@code null} to skip. */
        @Nullable public Connector.ProgressListener progressListener;

        /** @brief TLS trust policy. */
        public Connector.TrustMode trustMode = Connector.TrustMode.TRUST_ALL;
        /** @brief SHA-256 SPKI pin, required for {@link Connector.TrustMode#PINNED}. */
        @Nullable public String    pinnedSha256;
    }

    /** @brief Single reusable connector instance. */
    private final Connector connector = new Connector();
    private final App app;
    private volatile boolean isSending = false;

    /**
     * @param app Application instance providing repositories.
     */
    public SendDataViewModel(App app) {
        this.app = app;
    }

    /** @brief @return {@code true} while a send operation is in progress. */
    public boolean getIsSending() {
        return this.isSending || connector.getIsSending();
    }

    /**
     * @brief Sends the payload described by {@code cfg} and persists the result.
     *
     * @details
     * 1. Validates the port and the IP address.
     * 2. Ensures a {@link Connection} row exists for
     *    (ip, port, TLS-flag); inserts one if not.
     * 3. Configures the shared {@link Connector}.
     * 4. Calls {@link Connector#sendData()} synchronously.
     * 5. Persists a {@link Message} with status {@code SUCCESS} or
     *    {@code FAILED} depending on the outcome.
     *
     * @param cfg Fully-populated configuration.
     * @return Raw JSON response body from the server.
     *
     * @throws Exception if the port/IP are invalid, a send is already in
     *         progress, or the transport layer throws.
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
                .getByIpAndPortAndIsSSLEncrypted(cfg.serverIp, serverPort, cfg.isSSLEncryptEnabled);
        if (conn == null) {
            app.getConnectionRepository().insert(
                    new Connection(serverPort, cfg.serverIp, cfg.isSSLEncryptEnabled));
            conn = app.getConnectionRepository()
                    .getByIpAndPortAndIsSSLEncrypted(cfg.serverIp, serverPort, cfg.isSSLEncryptEnabled);
        }

        SimpleDateFormat sdf = new SimpleDateFormat(
                "yyyy-MM-dd HH:mm:ss", Locale.getDefault());
        Message message = new Message(
                conn.id,
                MessageStatus.FAILED.index(),
                cfg.sendingType.index(),
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