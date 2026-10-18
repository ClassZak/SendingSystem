package com.example.sendingsystemclient.domain.model;

import android.content.ContentResolver;
import android.database.Cursor;
import android.net.Uri;
import android.provider.OpenableColumns;
import com.example.sendingsystemclient.domain.viewmodel.SendDataViewModel;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import org.json.JSONObject;

import java.io.IOException;
import java.io.InputStream;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.security.SecureRandom;
import java.security.cert.X509Certificate;
import java.util.Objects;
import java.util.concurrent.TimeUnit;

import javax.net.ssl.SSLContext;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import okhttp3.CertificatePinner;
import okhttp3.MediaType;
import okhttp3.MultipartBody;
import okhttp3.OkHttpClient;
import okhttp3.Request;
import okhttp3.RequestBody;
import okhttp3.Response;
import okhttp3.ResponseBody;
import okio.BufferedSink;

/**
 * @brief HTTP client for the SendingSystem server.
 *
 * @details Routes the payload based on {@link #sendingType}:
 * <ul>
 *   <li>{@link SendingType#FILE} → {@code POST /api/upload}
 *       ({@code multipart/form-data}); the file is streamed from a
 *       {@link Uri} so it is never fully loaded into memory.</li>
 *   <li>All other types          → {@code POST /api/send} (JSON).</li>
 * </ul>
 *
 * Upload progress can be observed through {@link ProgressListener}.
 * TLS trust policy is selected via {@link TrustMode}.
 *
 * @warning {@link TrustMode#TRUST_ALL} disables certificate validation and
 *          should only be used with a self-signed development server.
 *
 * @see SendDataViewModel
 * @since 1.0
 */
public class Connector {

    /**
     * @brief Upload progress callback.
     *
     * @details Invoked from OkHttp's IO thread — callers that touch UI
     *          must marshal to the main thread themselves.
     *
     * @field bytesWritten Bytes handed to the socket so far.
     * @field totalBytes   Total payload size, or {@code -1} if unknown.
     */
    public interface ProgressListener {
        void onProgress(long bytesWritten, long totalBytes);
    }

    /**
     * @brief TLS trust policy.
     */
    public enum TrustMode {
        /** @brief Accept any certificate. <b>Development only.</b> */
        TRUST_ALL,
        /** @brief Use the platform CA store. Recommended for production. */
        SYSTEM_CA,
        /**
         * @brief Accept any certificate but pin the server SPKI by SHA-256.
         * @details Protects against other MITM certs; useful with
         *          self-signed certs in a closed network.
         */
        PINNED
    }

    /** @brief Connect/read/write timeout in milliseconds. */
    private static final int TIMEOUT_MS   = 30_000;
    /** @brief Chunk size used when streaming file bodies. */
    private static final int UPLOAD_CHUNK = 16 * 1024;

    // ===== Server configuration =====
    /** @brief IP protocol to validate {@link #serverIp} against. */
    public IPVersion ipVersion = IPVersion.IPv4;
    /** @brief Server address (hostname or IP literal). */
    public String    serverIp   = "127.0.0.1";
    /** @brief Server port (1–65535). */
    public int       serverPort = 5000;
    /** @brief Use HTTPS/WSS when {@code true}. */
    public boolean   isSSLEncryptEnabled = false;

    // ===== Payload =====
    /** @brief Data for {@code RAW_DATA} mode. Ignored for {@code FILE}. */
    public String      sendingData = "";
    /** @brief Selects the endpoint and body encoding. */
    public SendingType sendingType = SendingType.RAW_DATA;

    @Nullable private String          saveToPath       = null;
    @Nullable private byte[]          fileBytes        = null;
    @Nullable private Uri             fileUri          = null;
    @Nullable private ContentResolver contentResolver  = null;
    @Nullable private String          fileName         = null;
    @Nullable private ProgressListener progressListener = null;

    // ===== TLS =====
    private TrustMode trustMode     = TrustMode.TRUST_ALL;
    @Nullable private String pinnedSha256 = null;

    // ===== State / client cache =====
    private volatile boolean isSending = false;
    private boolean          cachedSsl;
    private TrustMode        cachedTrust;
    @Nullable private String cachedPin;
    @Nullable private OkHttpClient cachedClient;

    // ===== Setters =====

    /**
     * @brief Sets the file name under which the server should save the payload.
     * @param saveToPath Raw name; passed through {@link #sanitizeFilename}.
     */
    public void setSaveToPath(String saveToPath) {
        this.saveToPath = sanitizeFilename(saveToPath);
    }

    /** @brief @return The sanitised file name, or {@code null}. */
    @Nullable
    public String getSaveToPath() { return saveToPath; }

    /**
     * @brief Supplies the file content as an in-memory array.
     * @note  Prefer {@link #setFileUri} for anything above a few MB.
     */
    public void setFileBytes(@Nullable byte[] data, @Nullable String name) {
        this.fileBytes = data;
        this.fileUri = null;
        this.fileName = name;
    }

    /**
     * @brief Supplies the file as a {@link Uri} to be streamed.
     * @param cr   Resolver used to open the stream.
     * @param uri  Content URI; must grant read access for the lifetime
     *             of the upload.
     * @param name Suggested file name; used as multipart filename.
     */
    public void setFileUri(@NonNull ContentResolver cr,
                           @NonNull Uri uri,
                           @Nullable String name) {
        this.contentResolver = cr;
        this.fileUri = uri;
        this.fileBytes = null;
        this.fileName = name;
    }

    /** @brief Clears any previously set file source. */
    public void clearFile() {
        this.fileBytes = null;
        this.fileUri = null;
        this.fileName = null;
    }

    /** @brief Sets (or clears) the upload progress listener. */
    public void setProgressListener(@Nullable ProgressListener l) {
        this.progressListener = l;
    }

    /** @brief Selects the TLS trust policy. */
    public void setTrustMode(@NonNull TrustMode mode) { this.trustMode = mode; }

    /** @brief SHA-256 (base64) of the server SPKI; required for {@link TrustMode#PINNED}. */
    public void setPinnedSha256(@Nullable String pin) { this.pinnedSha256 = pin; }

    /** @brief @return {@code true} while a request is in flight. */
    public boolean getIsSending() { return isSending; }

    // ===== Entry point =====

    /**
     * @brief Performs one send operation.
     *
     * @details Synchronous — callers must run this off the UI thread.
     *
     * @return Raw JSON body of the server response, or {@code null}.
     *         Parse with {@code ServerResponse.fromJson}.
     *
     * @throws Exception on validation, network or TLS failure.
     */
    @Nullable
    public String sendData() throws Exception {
        this.isSending = true;
        try {
            if (sendingType == SendingType.FILE) return uploadFile();
            return sendJsonMessage();
        } finally {
            this.isSending = false;
            this.progressListener = null;
        }
    }

    // ===== JSON path =====

    /**
     * @brief Builds and posts a JSON body to {@code /api/send}.
     */
    private String sendJsonMessage() throws Exception {
        JSONObject json = new JSONObject();
        json.put("type", sendingType.toString());
        json.put("data", sendingData);
        if (saveToPath != null && !saveToPath.isEmpty()) {
            json.put("saveToPath", saveToPath);
        }

        RequestBody body = RequestBody.create(
                json.toString(),
                MediaType.get("application/json; charset=utf-8"));

        Request req = new Request.Builder()
                .url(baseUrl("/api/send"))
                .post(body)
                .build();

        return execute(req);
    }

    // ===== File upload path =====

    /**
     * @brief Builds a multipart body and posts it to {@code /api/upload}.
     */
    private String uploadFile() throws Exception {
        if (fileBytes == null && fileUri == null) {
            throw new IOException("No file to send");
        }

        String multipartName = (saveToPath != null && !saveToPath.isEmpty())
                ? saveToPath
                : (fileName != null ? fileName : "unnamed.dat");

        MultipartBody multipart = new MultipartBody.Builder()
                .setType(MultipartBody.FORM)
                .addFormDataPart("file", multipartName, buildFileBody())
                .build();

        Request req = new Request.Builder()
                .url(baseUrl("/api/upload"))
                .post(multipart)
                .build();

        return execute(req);
    }

    /**
     * @brief Returns a {@link RequestBody} that streams the file chunk by chunk.
     *
     * @details Uses {@link #fileBytes} if set, otherwise opens {@link #fileUri}
     *          through {@link #contentResolver}. Memory usage is O(chunk size),
     *          independent of the file size. Calls {@link ProgressListener}
     *          after every chunk.
     */
    private RequestBody buildFileBody() {
        return new RequestBody() {
            @Override public MediaType contentType() {
                return MediaType.get("application/octet-stream");
            }

            @Override public long contentLength() {
                if (fileBytes != null) return fileBytes.length;
                Long size = queryUriSize(fileUri);
                return size != null ? size : -1L;
            }

            @Override public void writeTo(@NonNull BufferedSink sink) throws IOException {
                final long total = contentLength();
                long written = 0;

                if (fileBytes != null) {
                    for (int i = 0; i < fileBytes.length; i += UPLOAD_CHUNK) {
                        int n = Math.min(UPLOAD_CHUNK, fileBytes.length - i);
                        sink.write(fileBytes, i, n);
                        written += n;
                        notifyProgress(written, total);
                    }
                    return;
                }

                if (fileUri == null || contentResolver == null) {
                    throw new IOException("No source to upload");
                }

                try (InputStream in = contentResolver.openInputStream(fileUri)) {
                    if (in == null) throw new IOException("Cannot open " + fileUri);
                    byte[] buf = new byte[UPLOAD_CHUNK];
                    int n;
                    while ((n = in.read(buf)) != -1) {
                        sink.write(buf, 0, n);
                        written += n;
                        notifyProgress(written, total);
                    }
                }
            }
        };
    }

    private void notifyProgress(long written, long total) {
        ProgressListener l = progressListener;
        if (l != null) l.onProgress(written, total);
    }

    @Nullable
    private Long queryUriSize(@Nullable Uri uri) {
        if (uri == null || contentResolver == null) return null;
        try (Cursor c = contentResolver.query(uri,
                new String[]{OpenableColumns.SIZE}, null, null, null)) {
            if (c != null && c.moveToFirst()) {
                int idx = c.getColumnIndex(OpenableColumns.SIZE);
                if (idx >= 0 && !c.isNull(idx)) return c.getLong(idx);
            }
        } catch (Exception ignored) { /* size unknown */ }
        return null;
    }

    // ===== HTTP execution =====

    /**
     * @brief Executes the request and returns the raw response body.
     *
     * @details On non-2xx responses without a body, throws {@link IOException}
     *          carrying the status code. If the server sent a JSON error
     *          body (as CivetWeb does), it is returned unchanged so the
     *          caller can parse it with {@code ServerResponse}.
     */
    private String execute(Request request) throws Exception {
        try (Response resp = client().newCall(request).execute()) {
            ResponseBody body = resp.body();
            String text = body != null ? body.string() : "";
            if (!resp.isSuccessful() && text.isEmpty()) {
                throw new IOException("HTTP " + resp.code() + " " + resp.message());
            }
            return text;
        }
    }

    private String baseUrl(String path) {
        String scheme = isSSLEncryptEnabled ? "https" : "http";
        return scheme + "://" + serverIp + ":" + serverPort + path;
    }

    // ===== OkHttp client (lazy, invalidated on config change) =====

    /**
     * @brief Returns an {@link OkHttpClient} matching the current TLS config.
     *
     * @details The client is cached and rebuilt only when any of
     *          {@link #isSSLEncryptEnabled}, {@link #trustMode} or
     *          {@link #pinnedSha256} changes.
     *
     * @throws IllegalArgumentException if {@link TrustMode#PINNED} is used
     *         without a pin.
     */
    private OkHttpClient client() throws Exception {
        if (cachedClient != null
                && cachedSsl   == isSSLEncryptEnabled
                && cachedTrust == trustMode
                && Objects.equals(cachedPin, pinnedSha256)) {
            return cachedClient;
        }

        OkHttpClient.Builder b = new OkHttpClient.Builder()
                .connectTimeout(TIMEOUT_MS, TimeUnit.MILLISECONDS)
                .readTimeout   (TIMEOUT_MS, TimeUnit.MILLISECONDS)
                .writeTimeout  (TIMEOUT_MS, TimeUnit.MILLISECONDS);

        if (isSSLEncryptEnabled) {
            switch (trustMode) {
                case TRUST_ALL: {
                    X509TrustManager tm = trustAllManager();
                    SSLContext ctx = SSLContext.getInstance("TLS");
                    ctx.init(null, new TrustManager[]{tm}, new SecureRandom());
                    b.sslSocketFactory(ctx.getSocketFactory(), tm);
                    b.hostnameVerifier((h, s) -> true);
                    break;
                }
                case PINNED: {
                    if (pinnedSha256 == null || pinnedSha256.isEmpty()) {
                        throw new IllegalArgumentException(
                                "PINNED requires a SHA-256 pin via setPinnedSha256()");
                    }
                    X509TrustManager tm = trustAllManager();
                    SSLContext ctx = SSLContext.getInstance("TLS");
                    ctx.init(null, new TrustManager[]{tm}, new SecureRandom());
                    b.sslSocketFactory(ctx.getSocketFactory(), tm);
                    b.hostnameVerifier((h, s) -> true);
                    b.certificatePinner(new CertificatePinner.Builder()
                            .add(serverIp, "sha256/" + pinnedSha256)
                            .build());
                    break;
                }
                case SYSTEM_CA:
                default:
                    /* OkHttp defaults: platform trust + hostname check. */
                    break;
            }
        }

        cachedClient = b.build();
        cachedSsl    = isSSLEncryptEnabled;
        cachedTrust  = trustMode;
        cachedPin    = pinnedSha256;
        return cachedClient;
    }

    private static X509TrustManager trustAllManager() {
        return new X509TrustManager() {
            @Override public void checkClientTrusted(X509Certificate[] c, String a) { }
            @Override public void checkServerTrusted(X509Certificate[] c, String a) { }
            @Override public X509Certificate[] getAcceptedIssuers() { return new X509Certificate[0]; }
        };
    }

    // ===== Validation helpers =====

    /**
     * @brief Sanitises a user-supplied filename.
     *
     * @details Replaces every character outside `[A-Za-z0-9._-]` with `_`,
     *          collapses `..` sequences and strips a leading dot.
     *          Mirrors the server-side `sanitize_filename` so the name
     *          arriving at the server matches what the user expects.
     *
     * @param input Raw name. May be {@code null}.
     * @return Safe name, or `"unnamed.dat"` if the result is empty.
     */
    public static String sanitizeFilename(String input) {
        if (input == null || input.isEmpty()) return "";
        String safe = input.replaceAll("[^a-zA-Z0-9._-]", "_");
        safe = safe.replaceAll("\\.{2,}", "_");
        safe = safe.replaceFirst("^\\.", "");
        return safe.isEmpty() ? "unnamed.dat" : safe;
    }

    /**
     * @brief Checks that {@code ip} parses and matches the requested family.
     *
     * @param ip      Hostname or IP literal.
     * @param version Expected {@link IPVersion}.
     * @return {@code true} if {@code ip} resolves to an address of the
     *         requested family, {@code false} otherwise.
     */
    public static boolean isValidInetAddress(String ip, IPVersion version) {
        try {
            InetAddress address = InetAddress.getByName(ip);
            if (version == IPVersion.IPv4) return address instanceof Inet4Address;
            if (version == IPVersion.IPv6) return address instanceof Inet6Address;
        } catch (Exception ignored) { /* invalid */ }
        return false;
    }
}