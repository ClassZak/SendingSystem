package com.example.sendingsystemclient.dto;

import androidx.annotation.Nullable;

import org.json.JSONException;
import org.json.JSONObject;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.Inet4Address;
import java.net.Inet6Address;
import java.net.InetAddress;
import java.net.Socket;

public class Connector {
    public IPVersion ipVersion = IPVersion.IPv4;
    public String serverIp = "127.0.0.1";
    public int serverPort = 5000;
    public String sendingData = "";
    public SendingType sendingType = SendingType.RAW_DATA;
    @Nullable
    public String saveToPath;


    @Nullable
    public String sendData() throws IOException, JSONException {
        try (Socket socket = new Socket(serverIp, serverPort)) {
            PrintWriter writer = new PrintWriter(socket.getOutputStream(), true);
            BufferedReader reader =
                new BufferedReader(new InputStreamReader(socket.getInputStream()));

            JSONObject jsonRequest = new JSONObject();
            jsonRequest.put("type", sendingType.toString());
            jsonRequest.put("data", sendingData);
            if (saveToPath != null)
                jsonRequest.put("saveToPath", saveToPath);

            writer.println(jsonRequest);

            socket.shutdownOutput();

            return reader.readLine();
        }
    }

    public static boolean isValidInetAddress(String ip, IPVersion version) {
        try {
            InetAddress addr = InetAddress.getByName(ip);
            if (version == IPVersion.IPv4) {
                return addr instanceof Inet4Address;
            } else if (version == IPVersion.IPv6) {
                return addr instanceof Inet6Address;
            }
        } catch (Exception e) {
            // invalid
        }
        return false;
    }
}
