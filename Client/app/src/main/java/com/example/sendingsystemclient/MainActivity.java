package com.example.sendingsystemclient;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Toast;

import androidx.activity.EdgeToEdge;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.graphics.Insets;
import androidx.core.view.ViewCompat;
import androidx.core.view.WindowInsetsCompat;

import com.example.sendingsystemclient.dto.Connector;
import com.example.sendingsystemclient.dto.IPVersion;

import java.io.IOException;

public class MainActivity extends AppCompatActivity {

    private EditText editTextServerIP;
    private EditText editTextServerPort;
    private EditText editTextSendingData;
    private CheckBox checkBoxIPv4;
    private CheckBox checkBoxIPv6;
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
        checkBoxIPv4 = findViewById(R.id.checkBoxIPv4);
        checkBoxIPv6 = findViewById(R.id.checkBoxIPv6);
        buttonSend = findViewById(R.id.button);

        setUpListeners();
    }

    private void setUpListeners() {
        // Взаимное исключение чекбоксов
        checkBoxIPv4.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) checkBoxIPv6.setChecked(false);
        });
        checkBoxIPv6.setOnCheckedChangeListener((buttonView, isChecked) -> {
            if (isChecked) checkBoxIPv4.setChecked(false);
        });

        // Обработчик кнопки отправки
        buttonSend.setOnClickListener(v -> onSendClicked());
    }

    private void onSendClicked() {
        String ip = editTextServerIP.getText().toString().trim();
        String portStr = editTextServerPort.getText().toString().trim();
        String data = editTextSendingData.getText().toString().trim();

        // Проверка порта
        int port;
        try {
            port = Integer.parseInt(portStr);
            if (port < 1 || port > 65535) {
                Toast.makeText(this, "Порт должен быть в диапазоне 1-65535", Toast.LENGTH_SHORT).show();
                return;
            }
        } catch (NumberFormatException e) {
            Toast.makeText(this, "Некорректный порт", Toast.LENGTH_SHORT).show();
            return;
        }

        // Определяем версию IP по чекбоксам
        IPVersion ipVersion = checkBoxIPv4.isChecked() ? IPVersion.IPv4 : IPVersion.IPv6;

        // Проверка IP-адреса без regex
        if (!Connector.isValidInetAddress(ip, ipVersion)) {
            Toast.makeText(this, "Некорректный IP-адрес: " + ip, Toast.LENGTH_SHORT).show();
            return;
        }

        // Создаём объект коннектора
        Connector connector = new Connector();
        connector.serverIp = ip;
        connector.serverPort = port;
        connector.sendingData = data;
        connector.ipVersion = ipVersion;
        // saveToPath при необходимости можно задать

        // Выполняем отправку в фоновом потоке (чтобы не блокировать UI)
        new Thread(() -> {
            try {
                String response = connector.sendData();
                runOnUiThread(() ->
                        Toast.makeText(MainActivity.this, "Ответ: " + response, Toast.LENGTH_LONG).show()
                );
            } catch (IOException | org.json.JSONException e) {
                runOnUiThread(() ->
                        Toast.makeText(MainActivity.this, "Ошибка отправки: " + e.getMessage(), Toast.LENGTH_LONG).show()
                );
            }
        }).start();
    }
}