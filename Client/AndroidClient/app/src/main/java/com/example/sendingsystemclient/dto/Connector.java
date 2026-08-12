package com.example.sendingsystemclient.dto;

import android.annotation.SuppressLint;

import androidx.annotation.Nullable;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.Socket;
import java.nio.charset.StandardCharsets;
import java.security.KeyManagementException;
import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;

import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLException;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;
import javax.net.ssl.TrustManager;
import javax.net.ssl.X509TrustManager;

import kotlin.NotImplementedError;

public class Connector {
    private static final int MAX_RESPONSE_SIZE = 1024 * 1024;
    private static final int TIMEOUT_TIME_MILLISECONDS = 5000;
    public IPVersion ipVersion = IPVersion.IPv4;
    public String serverIp = "127.0.0.1";
    public int serverPort = 5000;
    public String sendingData = "";
    public SendingType sendingType = SendingType.RAW_DATA;
    public boolean isSSLEncryptEnabled = false;
    @Nullable
    private String saveToPath = null;
    public void setSaveToPath(String saveToPath) {
        this.saveToPath = sanitizeFilename(saveToPath);
    }
    @Nullable
    public String getSaveToPath() {
        return this.saveToPath;
    }




    @Nullable
    public String sendData() throws IOException, JSONException, Exception {
        if (!isSSLEncryptEnabled) {
            return this.sendDataRaw();
        } else {
            return this.sendDataSSL();
        }
    }

    @Nullable
    public String sendDataRaw() throws IOException, JSONException {
        try (Socket socket = new Socket(serverIp, serverPort)) {
            socket.setTcpNoDelay(false);
            socket.setSoTimeout(TIMEOUT_TIME_MILLISECONDS);
            DataOutputStream dos = new DataOutputStream(socket.getOutputStream());
            DataInputStream dis = new DataInputStream(socket.getInputStream());

            // create JSON
            JSONObject jsonRequest = new JSONObject();
            jsonRequest.put("type", sendingType.toString());
            jsonRequest.put("data", sendingData);
            if (saveToPath != null && !saveToPath.isEmpty())
                jsonRequest.put("saveToPath", saveToPath);

            byte[] jsonBytes = jsonRequest.toString().getBytes(StandardCharsets.UTF_8);
            int length = jsonBytes.length;

            // Send message length (4 байта, little‑endian)
            dos.writeInt(length);
            dos.write(jsonBytes);
            dos.flush();

            // Stop data sending
            socket.shutdownOutput();

            // Receive the response (int) + JSON
            int responseLength = dis.readInt();
            if (responseLength <= 0 || responseLength > MAX_RESPONSE_SIZE) {
                throw new IOException("Invalid response length: " + responseLength);
            }
            byte[] responseBytes = new byte[responseLength];
            dis.readFully(responseBytes);
            return new String(responseBytes, StandardCharsets.UTF_8);
        }
    }


    @Nullable
    public String sendDataSSL() throws IOException, JSONException, Exception {
        // Set up the SSL socket
        SSLSocketFactory socketFactory;
        // Throws Exception
        socketFactory = this.createTrustAllSocketFactory();

        DataInputStream dis;
        try (SSLSocket socket = (SSLSocket) socketFactory.createSocket(serverIp, serverPort)) {
            socket.startHandshake();

            socket.setTcpNoDelay(false);
            socket.setSoTimeout(TIMEOUT_TIME_MILLISECONDS);
            DataOutputStream dos = new DataOutputStream(socket.getOutputStream());
            dis = new DataInputStream(socket.getInputStream());

            // Create JSON
            JSONObject jsonRequest = new JSONObject();
            jsonRequest.put("type", sendingType.toString());
            jsonRequest.put("data", sendingData);
            if (saveToPath != null && !saveToPath.isEmpty())
                jsonRequest.put("saveToPath", saveToPath);

            byte[] jsonBytes = jsonRequest.toString().getBytes(StandardCharsets.UTF_8);
            int length = jsonBytes.length;

            // Send message length (4 байта, little‑endian)
            dos.writeInt(length);
            dos.write(jsonBytes);
            dos.flush();

            // Stop data sending
            socket.shutdownOutput();

            // Receive the response (int) + JSON
            int responseLength = dis.readInt();
            if (responseLength <= 0 || responseLength > MAX_RESPONSE_SIZE) {
                throw new IOException("Invalid response length: " + responseLength);
            }
            byte[] responseBytes = new byte[responseLength];
            dis.readFully(responseBytes);
            return new String(responseBytes, StandardCharsets.UTF_8);
        }
    }




    public static String sanitizeFilename(String input) {
        if (input == null || input.isEmpty()) return "";
        String safe = input.replaceAll("[^a-zA-Z0-9._-]", "_");
        safe = safe.replaceAll("\\.{2,}", "_");
        safe = safe.replaceFirst("^\\.", "");
        return safe.isEmpty() ? "unnamed.dat" : safe;
    }
    public static boolean isValidInetAddress(String ip, IPVersion version) {
        try {
            InetAddress address = InetAddress.getByName(ip);
            if (version == IPVersion.IPv4) {
                return address instanceof Inet4Address;
            } else if (version == IPVersion.IPv6) {
                return address instanceof Inet6Address;
            }
        } catch (Exception e) {
            // invalid
        }
        return false;
    }



    private SSLSocketFactory createTrustAllSocketFactory() throws KeyManagementException, NoSuchAlgorithmException {
        @SuppressLint("CustomX509TrustManager") TrustManager[] trustAllManager = new TrustManager[] {
                new X509TrustManager() {
                    @SuppressLint("TrustAllX509TrustManager")
                    @Override
                    public void checkClientTrusted(X509Certificate[] x509Certificates, String s) throws CertificateException {

                    }

                    @SuppressLint("TrustAllX509TrustManager")
                    @Override
                    public void checkServerTrusted(X509Certificate[] x509Certificates, String s) throws CertificateException {

                    }

                    @Override
                    public X509Certificate[] getAcceptedIssuers() {
                        return new X509Certificate[0];
                    }
                }
        };
        SSLContext sslContext = SSLContext.getInstance("TLS");
        sslContext.init(null, trustAllManager, new SecureRandom());

        return sslContext.getSocketFactory();
    }
}
