package com.example.sendingsystemclient.dto;

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

import kotlin.NotImplementedError;

public class Connector {
    private static final int MAX_RESPONSE_SIZE = 1024 * 1024;
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
    public String sendData() throws IOException, JSONException, NotImplementedError {
        if (isSSLEncryptEnabled)
            throw new NotImplementedError("SSL does not supporting yet");
        try (Socket socket = new Socket(serverIp, serverPort)) {
            socket.setTcpNoDelay(true);
            DataOutputStream dos = new DataOutputStream(socket.getOutputStream());
            DataInputStream dis = new DataInputStream(socket.getInputStream());

            // Формируем JSON
            JSONObject jsonRequest = new JSONObject();
            jsonRequest.put("type", sendingType.toString());
            jsonRequest.put("data", sendingData);
            if (saveToPath != null && !saveToPath.isEmpty())
                jsonRequest.put("saveToPath", saveToPath);

            byte[] jsonBytes = jsonRequest.toString().getBytes(StandardCharsets.UTF_8);
            int length = jsonBytes.length;

            // Отправляем длину (4 байта, little‑endian) и сами данные
            dos.writeInt(length);
            dos.write(jsonBytes);
            dos.flush();

            // Сообщаем серверу, что больше данных не будет
            socket.shutdownOutput();

            // Читаем ответ: длина (int) + JSON
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
}
