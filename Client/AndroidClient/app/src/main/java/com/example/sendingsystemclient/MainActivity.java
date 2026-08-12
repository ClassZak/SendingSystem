package com.example.sendingsystemclient;

import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
import android.util.Base64;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.example.sendingsystemclient.dto.Connector;
import com.example.sendingsystemclient.dto.IPVersion;
import com.example.sendingsystemclient.dto.SendingType;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;

import kotlin.NotImplementedError;

public class MainActivity extends AppCompatActivity {

    private static final int REQUEST_CODE_PICK_FILE = 1001;

    private EditText editTextServerIP;
    private EditText editTextServerPort;
    private EditText editTextSendingData;
    private EditText editTextSaveToPath;
    private CheckBox checkBoxIPv4;
    private CheckBox checkBoxIPv6;
    private Button buttonSend;

    private RadioGroup radioGroupSource;
    private RadioButton radioEncryptYes;
    private RadioButton radioFile;
    private LinearLayout layoutFilePicker;
    private TextView textFileName;

    private byte[] fileBytes;

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

        editTextServerIP = findViewById(R.id.editTextServerIP);
        editTextServerPort = findViewById(R.id.editTextServerPort);
        editTextSendingData = findViewById(R.id.editTextSendingData);
        editTextSaveToPath = findViewById(R.id.editTextSaveToPath);
        checkBoxIPv4 = findViewById(R.id.checkBoxIPv4);
        checkBoxIPv6 = findViewById(R.id.checkBoxIPv6);
        buttonSend = findViewById(R.id.button);

        radioGroupSource = findViewById(R.id.radioGroupSource);
        radioFile = findViewById(R.id.radioFile);
        radioEncryptYes = findViewById(R.id.radioEncryptYes);
        layoutFilePicker = findViewById(R.id.layoutFilePicker);
        textFileName = findViewById(R.id.textFileName);

        setUpListeners();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_PICK_FILE && resultCode == RESULT_OK && data != null) {
            Uri uri = data.getData();
            if (uri != null) {
                try (InputStream inputStream = getContentResolver().openInputStream(uri)) {
                    ByteArrayOutputStream buffer = new ByteArrayOutputStream();
                    byte[] chunk = new byte[4096];
                    int n;
                    while ((n = inputStream.read(chunk)) != -1) {
                        buffer.write(chunk, 0, n);
                    }
                    fileBytes = buffer.toByteArray();
                    String fileName = getFileName(uri);
                    textFileName.setText(fileName != null ? fileName : "selected file");
                    Toast.makeText(this, "File loaded (" + fileBytes.length + " bytes)", Toast.LENGTH_SHORT).show();
                } catch (IOException e) {
                    Toast.makeText(this, "Failed to read file: " + e.getMessage(), Toast.LENGTH_SHORT).show();
                }
            }
        }
    }

    private String getFileName(Uri uri) {
        String result = null;
        if (uri.getScheme().equals("content")) {
            try (Cursor cursor = getContentResolver().query(uri, null, null, null, null)) {
                if (cursor != null && cursor.moveToFirst()) {
                    int nameIndex = cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                    if (nameIndex >= 0) {
                        result = cursor.getString(nameIndex);
                    }
                }
            }
        }
        if (result == null) {
            result = uri.getLastPathSegment();
        }
        return result;
    }

    private void setUpListeners() {
        checkBoxIPv4.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) checkBoxIPv6.setChecked(false);
        });
        checkBoxIPv6.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) checkBoxIPv4.setChecked(false);
        });

        radioGroupSource.setOnCheckedChangeListener((group, checkedId) -> {
            if (checkedId == R.id.radioText) {
                editTextSendingData.setVisibility(View.VISIBLE);
                layoutFilePicker.setVisibility(View.GONE);
            } else {
                editTextSendingData.setVisibility(View.GONE);
                layoutFilePicker.setVisibility(View.VISIBLE);
            }
        });

        Button buttonBrowseFile = findViewById(R.id.buttonBrowseFile);
        buttonBrowseFile.setOnClickListener(v -> {
            Intent intent = new Intent(Intent.ACTION_OPEN_DOCUMENT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType("*/*");
            startActivityForResult(intent, REQUEST_CODE_PICK_FILE);
        });

        buttonSend.setOnClickListener(v -> onSendClicked());
    }

    private void onSendClicked() {
        IPVersion ipVersion = checkBoxIPv4.isChecked() ? IPVersion.IPv4 : IPVersion.IPv6;
        String ip = editTextServerIP.getText().toString().trim();
        String portStr = editTextServerPort.getText().toString().trim();
        int port;
        String saveToPath = editTextSaveToPath.getText().toString().trim();
        boolean isFileMode = radioFile.isChecked();
        boolean isSSLEncryptEnabled = radioEncryptYes.isChecked();


        try {
            port = Integer.parseInt(portStr);
            if (port < 1 || port > 65535) {
                Toast.makeText(this, "The port must be between 1 and 65535", Toast.LENGTH_SHORT).show();
                return;
            }
        } catch (NumberFormatException e) {
            Toast.makeText(this, "Wrong port", Toast.LENGTH_SHORT).show();
            return;
        }

        if (!Connector.isValidInetAddress(ip, ipVersion)) {
            Toast.makeText(this, "Wrong IP-address: " + ip, Toast.LENGTH_SHORT).show();
            return;
        }


        String dataToSend;
        SendingType sendingType;

        if (isFileMode) {
            if (fileBytes == null || fileBytes.length == 0) {
                Toast.makeText(this, "Choose a file first", Toast.LENGTH_SHORT).show();
                return;
            }
            dataToSend = Base64.encodeToString(fileBytes, Base64.NO_WRAP);
            sendingType = SendingType.FILE;
        } else {
            String text = editTextSendingData.getText().toString().trim();
            if (text.isEmpty()) {
                Toast.makeText(this, "Enter text", Toast.LENGTH_SHORT).show();
                return;
            }
            dataToSend = text;
            sendingType = SendingType.RAW_DATA;
        }

        Connector connector = new Connector();
        connector.serverIp = ip;
        connector.serverPort = port;
        connector.sendingType = sendingType;
        connector.sendingData = dataToSend;
        connector.ipVersion = ipVersion;
        connector.isSSLEncryptEnabled = isSSLEncryptEnabled;
        if (!saveToPath.isEmpty())
            connector.setSaveToPath(Connector.sanitizeFilename(saveToPath));
        else
            connector.setSaveToPath(null);

        new Thread(() -> {
            try {
                String response = connector.sendData();
                runOnUiThread(() ->
                        Toast.makeText(MainActivity.this, "Response: " + response, Toast.LENGTH_LONG).show()
                );
            } catch (IOException | org.json.JSONException e) {
                runOnUiThread(() ->
                        Toast.makeText(MainActivity.this, "Sending error: " + e.getMessage(), Toast.LENGTH_LONG).show()
                );
            } catch (Exception e) {
                runOnUiThread(() ->
                        Toast.makeText(MainActivity.this, "Error: " + e.getMessage(), Toast.LENGTH_LONG).show()
                );
            }
        }).start();
    }
}