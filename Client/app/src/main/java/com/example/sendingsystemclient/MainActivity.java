package com.example.sendingsystemclient;

import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.example.sendingsystemclient.dto.Connector;
import com.example.sendingsystemclient.dto.IPVersion;
import com.example.sendingsystemclient.dto.SendingType;

import java.io.IOException;
import java.lang.reflect.Array;
import java.util.Arrays;

public class MainActivity extends AppCompatActivity {

    private EditText editTextServerIP;
    private EditText editTextServerPort;
    private EditText editTextSendingData;
    private EditText editTextSaveToPath;
    private CheckBox checkBoxIPv4;
    private CheckBox checkBoxIPv6;
    private Spinner spinnerSendingType;
    private Button buttonSend;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        EdgeToEdge.enable(this);
        setContentView(R.layout.activity_main);
        ViewCompat.setOnApplyWindowInsetsListener(findViewById(R.id.main), (v, insets) -> {
            Insets systemBars = insets.getInsets(WindowInsetsCompat.Type.systemBars());
            v.setPadding(systemBars.left, systemBars.top, systemBars.right, systemBars.bottom);
            return insets;
        });

        // Инициализация View
        editTextServerIP = findViewById(R.id.editTextServerIP);
        editTextServerPort = findViewById(R.id.editTextServerPort);
        editTextSendingData = findViewById(R.id.editTextSendingData);
        editTextSaveToPath = findViewById(R.id.editTextSaveToPath);
        checkBoxIPv4 = findViewById(R.id.checkBoxIPv4);
        checkBoxIPv6 = findViewById(R.id.checkBoxIPv6);
        spinnerSendingType = findViewById(R.id.spinnerSendingType);
        buttonSend = findViewById(R.id.button);

        setUpListeners();

        String[] sendingTypes = Arrays.stream(SendingType.values())
                .map(SendingType::name)
                .toArray(String[]::new);
        ArrayAdapter<String> adapter = new ArrayAdapter(this, android.R.layout.simple_spinner_item, sendingTypes);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_item);
        spinnerSendingType.setAdapter(adapter);
        spinnerSendingType.setSelection(0);
    }

    private void setUpListeners() {
        checkBoxIPv4.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) checkBoxIPv6.setChecked(false);
        });
        checkBoxIPv6.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) checkBoxIPv4.setChecked(false);
        });

        // Sending button listener
        buttonSend.setOnClickListener(v -> onSendClicked());
    }

    private void onSendClicked() {
        String ip = editTextServerIP.getText().toString().trim();
        String portStr = editTextServerPort.getText().toString().trim();
        String data = editTextSendingData.getText().toString().trim();
        String saveToPath = editTextSaveToPath.getText().toString().trim();

        // Port validation
        int port;
        try {
            port = Integer.parseInt(portStr);
            if (port < 1 || port > 65535) {
                Toast.makeText(
                        this,
                        "The port must be between 1 and 65535",
                        Toast.LENGTH_SHORT
                ).show();
                return;
            }
        } catch (NumberFormatException e) {
            Toast.makeText(this, "Wrong port", Toast.LENGTH_SHORT).show();
            return;
        }

        // calculate IP version by checkboxes
        IPVersion ipVersion = checkBoxIPv4.isChecked() ? IPVersion.IPv4 : IPVersion.IPv6;

        // IP address verify
        if (!Connector.isValidInetAddress(ip, ipVersion)) {
            Toast.makeText(this, "Wrong IP-address: " + ip, Toast.LENGTH_SHORT).show();
            return;
        }

        Connector connector = new Connector();
        connector.serverIp = ip;
        connector.serverPort = port;
        connector.sendingType =
                this.spinnerSendingType.getSelectedItem() == SendingType.RAW_DATA.toString() ?
                        SendingType.RAW_DATA : SendingType.FILE;
        connector.sendingData = data;
        connector.ipVersion = ipVersion;
        connector.setSaveToPath(saveToPath);

        // Send in new thread (free UI thread)
        new Thread(() -> {
            try {
                String response = connector.sendData();
                runOnUiThread(() ->
                        Toast.makeText(
                            MainActivity.this,
                            "Response: " + response,
                            Toast.LENGTH_LONG
                        ).show()
                );
            } catch (IOException | org.json.JSONException e) {
                runOnUiThread(() ->
                    Toast.makeText(
                        MainActivity.this,
                        "Sending error: " + e.getMessage(),
                        Toast.LENGTH_LONG
                    ).show()
                );
            } catch (Exception e) {
                runOnUiThread(() ->
                    Toast.makeText(
                        MainActivity.this,
                        "Error: " + e.getMessage(),
                        Toast.LENGTH_LONG
                    ).show()
                );
            }
        }).start();
    }
}