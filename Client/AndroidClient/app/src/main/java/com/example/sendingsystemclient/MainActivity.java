package com.example.sendingsystemclient;

import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.OpenableColumns;
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
import com.example.sendingsystemclient.domain.model.Connector;
import com.example.sendingsystemclient.domain.model.IPVersion;
import com.example.sendingsystemclient.domain.model.ResponseType;
import com.example.sendingsystemclient.domain.model.SendingType;
import com.example.sendingsystemclient.domain.model.ServerResponse;
import com.example.sendingsystemclient.domain.viewmodel.SendDataViewModel;

/**
 * @brief Main UI: compose a message or pick a file and send it.
 *
 * @details
 * Lifecycle highlights:
 * <ul>
 *   <li>On create, restores the last successful connection.</li>
 *   <li>Handles {@code ACTION_SEND} intents both on create and on new
 *       intent (so the activity can be re-used as a share target).</li>
 *   <li>Delegates all networking to {@link SendDataViewModel} on a
 *       dedicated worker thread.</li>
 * </ul>
 *
 * @note  File selection stores a {@link Uri}, not the raw bytes, so
 *        memory usage is independent of the file size. The streaming
 *        upload is performed inside {@link Connector}.
 */
public class MainActivity extends AppCompatActivity {

    /** @brief Request code for {@link #onActivityResult} file picker. */
    private static final int REQUEST_CODE_PICK_FILE = 1001;

    private EditText     editTextServerIP;
    private EditText     editTextServerPort;
    private EditText     editTextSendingData;
    private EditText     editTextSaveToPath;
    private CheckBox     checkBoxIPv4;
    private CheckBox     checkBoxIPv6;
    private Button       buttonSend;

    private RadioGroup   radioGroupSource;
    private RadioButton  radioEncryptYes;
    private RadioButton  radioFile;
    private LinearLayout layoutFilePicker;
    private TextView     textFileName;

    private App               app;
    private SendDataViewModel sendDataViewModel;

    /** @brief Currently selected file, kept as a streamable URI. */
    @Nullable private Uri    fileUri  = null;
    /** @brief Display name of the selected file. */
    @Nullable private String fileName = null;

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

        editTextServerIP    = findViewById(R.id.editTextServerIP);
        editTextServerPort  = findViewById(R.id.editTextServerPort);
        editTextSendingData = findViewById(R.id.editTextSendingData);
        editTextSaveToPath  = findViewById(R.id.editTextSaveToPath);
        checkBoxIPv4        = findViewById(R.id.checkBoxIPv4);
        checkBoxIPv6        = findViewById(R.id.checkBoxIPv6);
        buttonSend          = findViewById(R.id.button);

        radioGroupSource    = findViewById(R.id.radioGroupSource);
        radioFile           = findViewById(R.id.radioFile);
        radioEncryptYes     = findViewById(R.id.radioEncryptYes);
        layoutFilePicker    = findViewById(R.id.layoutFilePicker);
        textFileName        = findViewById(R.id.textFileName);

        setUpListeners();
        setLastSuccessfulConnection();
        handleIncomingIntent(getIntent());
    }

    /**
     * @brief Handles a second {@code ACTION_SEND} while already open.
     * @see #handleIncomingIntent(Intent)
     */
    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        setIntent(intent);
        handleIncomingIntent(intent);
    }

    // ===== Incoming share intent =====

    /**
     * @brief Dispatches an incoming share intent to the right handler.
     *
     * @param intent May be {@code null}.
     */
    private void handleIncomingIntent(Intent intent) {
        if (intent == null) return;
        if (Intent.ACTION_SEND.equals(intent.getAction())) {
            handleSendIntent(intent);
        }
    }

    /**
     * @brief Fills the UI from an {@code ACTION_SEND} intent.
     *
     * @details For {@code text/*} populates the text field; for any other
     * MIME type treats the payload as a file URI and selects it.
     * Extend the MIME branch here if you add more content types.
     */
    private void handleSendIntent(Intent intent) {
        String type = intent.getType();
        if (type == null) return;

        if (type.startsWith("text/")) {
            String sharedText = intent.getStringExtra(Intent.EXTRA_TEXT);
            if (sharedText != null && !sharedText.isEmpty()) {
                radioGroupSource.check(R.id.radioText);
                editTextSendingData.setText(sharedText);
            }
        } else {
            Uri uri = intent.getParcelableExtra(Intent.EXTRA_STREAM);
            if (uri != null) selectFile(uri);
        }
    }

    // ===== File selection =====

    /**
     * @brief Records the selected URI and switches the UI to file mode.
     * @param uri File source; will be streamed on send.
     */
    private void selectFile(Uri uri) {
        fileName = queryDisplayName(uri);
        if (fileName == null) fileName = uri.getLastPathSegment();
        fileUri = uri;
        textFileName.setText(fileName != null ? fileName : "selected file");
        radioGroupSource.check(R.id.radioFile);
    }

    /**
     * @brief Best-effort lookup of {@link OpenableColumns#DISPLAY_NAME}.
     * @return Display name, or {@code null} if unavailable.
     */
    @Nullable
    private String queryDisplayName(Uri uri) {
        if (!"content".equals(uri.getScheme())) return null;
        try (Cursor c = getContentResolver().query(uri, null, null, null, null)) {
            if (c != null && c.moveToFirst()) {
                int idx = c.getColumnIndex(OpenableColumns.DISPLAY_NAME);
                if (idx >= 0) return c.getString(idx);
            }
        } catch (Exception ignored) { /* ignore */ }
        return null;
    }

    /**
     * @brief Receives the result of the {@code ACTION_OPEN_DOCUMENT} picker.
     * @see #REQUEST_CODE_PICK_FILE
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE_PICK_FILE && resultCode == RESULT_OK && data != null) {
            Uri uri = data.getData();
            if (uri != null) {
                try {
                    getContentResolver().takePersistableUriPermission(
                            uri, Intent.FLAG_GRANT_READ_URI_PERMISSION);
                } catch (SecurityException ignored) { /* provider disallowed */ }
                selectFile(uri);
            }
        }
    }

    // ===== Last successful connection =====

    /**
     * @brief Pre-fills IP / port / TLS from the most recent successful send.
     * @details Runs on a worker thread; UI is updated via
     *          {@link #runOnUiThread(Runnable)}.
     */
    private void setLastSuccessfulConnection() {
        new Thread(() -> {
            Message last = app.getMessageRepository().getLastSuccessful();
            if (last == null) return;
            Connection conn = app.getConnectionRepository().getById(last.connectionId);
            if (conn == null) return;
            runOnUiThread(() -> {
                editTextServerIP.setText(conn.ip);
                editTextServerPort.setText(String.valueOf(conn.port));
                radioEncryptYes.setChecked(conn.isSSLEncrypted);
            });
        }, "load-last-conn").start();
    }

    // ===== Listeners =====

    /**
     * @brief Wires all UI callbacks.
     */
    private void setUpListeners() {
        checkBoxIPv4.setOnCheckedChangeListener((v, c) -> { if (c) checkBoxIPv6.setChecked(false); });
        checkBoxIPv6.setOnCheckedChangeListener((v, c) -> { if (c) checkBoxIPv4.setChecked(false); });

        radioGroupSource.setOnCheckedChangeListener((group, checkedId) -> {
            boolean textMode = checkedId == R.id.radioText;
            editTextSendingData.setVisibility(textMode ? View.VISIBLE : View.GONE);
            layoutFilePicker.setVisibility(textMode ? View.GONE : View.VISIBLE);
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

    // ===== Send flow =====

    /**
     * @brief Validates the form, builds a {@link SendDataViewModel.Config},
     *        and dispatches the send on a worker thread.
     *
     * @details UI updates from the worker use {@link #runOnUiThread(Runnable)}.
     *          Progress reporting is available but intentionally not wired
     *          to any widget yet — the listener can be added later without
     *          touching the transport.
     */
    private void onSendClicked() {
        final IPVersion ipVersion = checkBoxIPv4.isChecked() ? IPVersion.IPv4 : IPVersion.IPv6;
        final String serverIp = editTextServerIP.getText().toString().trim();
        final String portStr  = editTextServerPort.getText().toString().trim();
        final String saveToPath = editTextSaveToPath.getText().toString().trim();
        final boolean fileMode = radioFile.isChecked();
        final boolean ssl      = radioEncryptYes.isChecked();

        if (fileMode && fileUri == null) {
            Toast.makeText(this, "Choose a file first", Toast.LENGTH_SHORT).show();
            return;
        }
        if (!fileMode) {
            String text = editTextSendingData.getText().toString().trim();
            if (text.isEmpty()) {
                Toast.makeText(this, "Enter text", Toast.LENGTH_SHORT).show();
                return;
            }
        }

        SendDataViewModel.Config cfg = new SendDataViewModel.Config();
        cfg.ipVersion           = ipVersion;
        cfg.serverIp            = serverIp;
        cfg.portString          = portStr;
        cfg.isSSLEncryptEnabled = ssl;
        cfg.saveToPath          = saveToPath;
        cfg.trustMode           = Connector.TrustMode.TRUST_ALL; // TODO: settings
        cfg.pinnedSha256        = null;

        if (fileMode) {
            cfg.sendingType      = SendingType.FILE;
            cfg.sendingData      = "";
            cfg.fileUri          = fileUri;
            cfg.fileName         = fileName;
            cfg.contentResolver  = getContentResolver();
        } else {
            cfg.sendingType      = SendingType.RAW_DATA;
            cfg.sendingData      = editTextSendingData.getText().toString().trim();
        }

        buttonSend.setEnabled(false);
        new Thread(() -> {
            try {
                String response = sendDataViewModel.SendData(cfg);
                ServerResponse sr = ServerResponse.fromJson(response);
                if (sr.type == ResponseType.Error) {
                    throw new Exception(sr.data);
                }
                runOnUiThread(() -> Toast.makeText(MainActivity.this,
                        "Server response: " + sr.data, Toast.LENGTH_LONG).show());

                if (fileMode) {
                    fileUri = null;
                    fileName = null;
                    runOnUiThread(() -> textFileName.setText(""));
                }
            } catch (Exception e) {
                final String msg = e.getMessage();
                runOnUiThread(() -> Toast.makeText(MainActivity.this,
                        "Error: " + msg, Toast.LENGTH_LONG).show());
            } finally {
                runOnUiThread(() -> buttonSend.setEnabled(true));
            }
        }, "send-worker").start();
    }
}