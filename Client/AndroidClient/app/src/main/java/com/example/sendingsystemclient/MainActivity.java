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

import com.example.sendingsystemclient.data.model.Connection;
import com.example.sendingsystemclient.data.model.Message;
import com.example.sendingsystemclient.data.model.MessageStatus;
import com.example.sendingsystemclient.data.model.MessageType;
import com.example.sendingsystemclient.domain.model.Connector;
import com.example.sendingsystemclient.domain.model.IPVersion;
import com.example.sendingsystemclient.domain.model.ResponseType;
import com.example.sendingsystemclient.domain.model.SendingType;
import com.example.sendingsystemclient.domain.model.ServerResponse;
import com.example.sendingsystemclient.domain.viewmodel.SendDataViewModel;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.Objects;


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

    private App app;
    private SendDataViewModel sendDataViewModel;

    private byte[] fileBytes;

    Connector connector = new Connector();

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

        app = (App) getApplication();
        sendDataViewModel = new SendDataViewModel(app);

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

        setLastSuccessfulConnection();
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
                    while (true) {
                        assert inputStream != null;
                        if ((n = inputStream.read(chunk)) == -1) break;
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
        if (Objects.equals(uri.getScheme(), "content")) {
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

    private void setLastSuccessfulConnection() {
        Thread setLastSuccessfulConnectionThread = new Thread(()->{
            Message lastSuccessfulMessage = app.getMessageRepository().getLastSuccessful();
            if (lastSuccessfulMessage == null)
                return;
            Connection lastSuccessfulConnection = app.getConnectionRepository().getById(lastSuccessfulMessage.connectionId);
            if (lastSuccessfulConnection == null)
                return;
            runOnUiThread(()->{
                editTextServerIP.setText(lastSuccessfulConnection.ip);
                editTextServerPort.setText(String.valueOf(lastSuccessfulConnection.port));
                radioEncryptYes.setChecked(lastSuccessfulConnection.isSSLEncrypted);
            });
        });
        setLastSuccessfulConnectionThread.start();
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

        buttonSend.setOnClickListener(v -> onSendClickedNew());
    }

    private void onSendClickedNew() {
        IPVersion ipVersion = checkBoxIPv4.isChecked() ? IPVersion.IPv4 : IPVersion.IPv6;
        String serverIp = editTextServerIP.getText().toString().trim();
        String portStr = editTextServerPort.getText().toString().trim();
        String saveToPath = editTextSaveToPath.getText().toString().trim();
        boolean isFileMode = radioFile.isChecked();
        boolean isSSLEncryptEnabled = radioEncryptYes.isChecked();
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

        try {
            String response = sendDataViewModel.SendData(ipVersion, serverIp, portStr, sendingType, dataToSend, isSSLEncryptEnabled, saveToPath);
            ServerResponse serverResponse = ServerResponse.fromJson(response);
            if (serverResponse.type == ResponseType.Error) {
                throw new Exception(serverResponse.data);
            } else {
                runOnUiThread(() ->
                        Toast.makeText(MainActivity.this, "Server response: " + serverResponse.data, Toast.LENGTH_LONG).show()
                );
            }
        } catch (Exception e) {
            runOnUiThread(() ->
                    Toast.makeText(MainActivity.this, "Error: " + e.getMessage(), Toast.LENGTH_LONG).show()
            );
        } finally {
            fileBytes = null;
        }
    }
}