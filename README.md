# Sending System

Simple program for sending data from one computer to another.

The current version runs an **HTTP/HTTPS server** (CivetWeb) with two REST endpoints:

- `POST /api/send` — JSON messages (text, small base64 payloads, save-to-file requests)
- `POST /api/upload` — `multipart/form-data` file uploads, streamed without
  loading the file into memory

A WebSocket endpoint for real-time audio/binary streaming (`/api/stream`) is
planned and partially wired; see `src/handlers/`.

The client is an Android app that talks to the server over HTTP or HTTPS using
OkHttp. It can send text or a file picked via the system file chooser or the
Android "Share" menu.

**⚠️ ATTENTION!!!**

Apple products (iPhone and other iOS devices) are not supported.
Windows 7, 10 and 11 builds are targeted.

---

## 🧩 Architecture

```
┌──────────────────┐        HTTP(S)         ┌────────────────────┐
│  Android client  │ ─────────────────────► │  Sending System    │
│  (OkHttp)        │  POST /api/send        │  (CivetWeb server) │
│                  │  POST /api/upload      │                    │
└──────────────────┘                        └────────────────────┘
                                                    │
                                                    ▼
                                            saves/ (file storage)
```

- The server listens on a single port (`CONNECTION_PORT`, default `5000`).
  - When `SSL_ENCRYPT = false` → plain HTTP.
  - When `SSL_ENCRYPT = true`  → HTTPS using the certificate/key in `server.pem`.
- The client switches between `http://` and `https://` via the **SSL checkbox**.
  The port stays the same; only the scheme changes.

---

## 🛠 Dependencies

### Linux Server

| Library | Purpose |
|---|---|
| **CivetWeb** (fetched via CMake `FetchContent`) | HTTP/HTTPS/WebSocket server |
| **cJSON** v1.7.18 (fetched via `FetchContent`) | JSON parsing for `/api/send` |
| **OpenSSL** 3.x | TLS, pulled in by CivetWeb |
| **libcurl** | Query an external HTTPS service for the public IP |
| **miniupnpc** | Ask the local router for the public IP via UPnP |
| **SQLite3** | Persistence of messages (planned) |

Ubuntu / Debian:

```bash
sudo apt install build-essential cmake libssl-dev \
                 libcurl4-openssl-dev libminiupnpc-dev
```

Fedora:

```bash
sudo dnf install gcc cmake openssl-devel \
                 libcurl-devel miniupnpc-devel
```

macOS:

```bash
brew install cmake openssl curl miniupnpc
```

### Android Client

```gradle
implementation("com.squareup.okhttp3:okhttp:4.12.0")
implementation("androidx.room:room-runtime:2.6.1")
annotationProcessor("androidx.room:room-compiler:2.6.1")
```

---

## 🖼️ Program Appearance

### Linux Server

<p align="center">
	<img align="center" src="https://github.com/ClassZak/SendingSystem/blob/master/screenshots/screenshot1.png"/>
</p>
<p align="center">
	<img align="center" src="https://github.com/ClassZak/SendingSystem/blob/master/screenshots/screenshot2.png"/>
</p>
<p align="center">
	<img align="center" src="https://github.com/ClassZak/SendingSystem/blob/master/screenshots/screenshot3.png"/>
</p>

### Android Client

<p align="center">
	<img align="center" src="https://github.com/ClassZak/SendingSystem/blob/master/screenshots/screenshot4.png"/>
</p>

> Screenshots may lag behind the current build. The current server prints
> a short startup banner and one line per request; the current client has a
> checkbox for SSL, two radio buttons (Text / File), and no progress bar yet.

---

## 🏗 🚀 Building and launching

### Linux server

#### 1. Clone the repository

```bash
git clone https://github.com/ClassZak/SendingSystem.git
cd SendingSystem/Server/DesktopServer
```

#### 2. Configure `src/util/config.h`

All server tunables live here. Defaults:

```c
#define CONNECTION_PORT   5000
static const char* CONNECTION_IP = "0.0.0.0";

static const bool  SSL_ENCRYPT       = true;
static const char* PUBLIC_KEY_FILE   = "server.pem";

#define MAX_MESSAGE_SIZE   0x6400000   /*  100 MB — /api/send body limit */
#define MAX_UPLOAD_SIZE    0x64000000  /*    1 GB — /api/upload file limit */
#define REQUEST_TIMEOUT_MS 30000
```

If `SSL_ENCRYPT = true`, `server.pem` **must exist** and contain **both**
the certificate and the private key, back to back. Generate it once:

```bash
# Either reuse an existing cert/key pair:
cat server.crt server.key > server.pem

# Or generate a self-signed pair from scratch:
openssl req -x509 -newkey rsa:2048 -nodes -days 3650 \
    -keyout server.key -out server.crt -subj "/CN=sending-system"
cat server.crt server.key > server.pem
chmod 600 server.pem
```

`server.pem` contains a private key — keep it out of git.

#### 3. Build

```bash
cmake --list-presets
cmake --preset linux-debug
cmake --build build --preset linux-debug
```

The binary is at `build/SendingSystem`.

CMake fetches CivetWeb and cJSON automatically. `libcurl` and `miniupnpc`
must be installed on the system (see **Dependencies** above).

#### 4. Run

```bash
./build/SendingSystem
```

Expected startup log:

```
[INFO] Local IP addresses (3):
[INFO]   192.168.1.42/24
[INFO]   127.0.0.1/8
[INFO]   fe80::1c2b:3fff:fe4a:1234/64
***  ... mg_start2: ... [listening_ports] -> [5000s]
***  ... mg_start2: ... [ssl_certificate] -> [server.pem]
[SUCCESS] Server started
[INFO]   POST http://0.0.0.0:5000/api/send
[INFO]   POST http://0.0.0.0:5000/api/upload
[INFO]   HTTPS/WSS on port 5000 (cert: server.pem)
[INFO] Public IP (UPnP): 203.0.113.42       # or (HTTP): … or "unknown"
```

The local-IP and public-IP lines are produced by a **background thread** —
server startup is not blocked by network lookups.

#### 5. Smoke-test with curl

Text message:

```bash
# plain HTTP
curl -v -X POST http://127.0.0.1:5000/api/send \
     -H "Content-Type: application/json" \
     -d '{"type":"RAW_DATA","data":"hello"}'

# HTTPS (self-signed)
curl -kv -X POST https://127.0.0.1:5000/api/send \
     -H "Content-Type: application/json" \
     -d '{"type":"RAW_DATA","data":"hello"}'
```

Expected body: `{"type":"message","data":"OK"}`.

File upload:

```bash
curl -kv -F "file=@record.aac" https://127.0.0.1:5000/api/upload
```

Expected body: `{"type":"message","data":"File(s) received"}` and the file
appears under `saves/`.

### Android client

#### 1. Open the project in Android Studio

The client source lives under `app/src/main/java/com/example/sendingsystemclient/`.

#### 2. Sync Gradle

Android Studio will download OkHttp and Room automatically.

#### 3. Run

```bash
adb uninstall com.example.sendingsystemclient    # ensure a clean install
./gradlew installDebug
```

#### 4. First run

1. Enter the server's **IP** (the one printed by `get_server_ip_addresses`,
   e.g. `192.168.1.42`) and **port** (`5000`).
2. Tick or untick **SSL** to match the server's `SSL_ENCRYPT` setting.
3. Pick **Text** or **File**, fill in the field(s), press **Send**.

### HTTP cleartext on Android 9+

If you set `SSL_ENCRYPT = false` and the client tries to use `http://`, Android
will refuse the connection with

```
CLEARTEXT communication to <ip> not permitted by network security policy
```

Either enable HTTPS on the server (`SSL_ENCRYPT = true`) or add the following
to `AndroidManifest.xml`:

```xml
<application
    android:usesCleartextTraffic="true"
    ...>
```

(For development only. Do not ship a production build with this flag on.)

---

## 🔌 API

### `POST /api/send`

JSON body:

```json
{
  "type":       "RAW_DATA" | "FILE" | "<any>",
  "data":       "<plain text, or base64-encoded file>",
  "saveToPath": "<optional file name>"
}
```

- Without `saveToPath`: message is logged and the server replies `OK`.
- With `saveToPath` and `type = "FILE"`: `data` is base64-decoded and saved.
- With `saveToPath` and any other type: `data` is written verbatim.

Response: `{"type": "message", "data": "..."}` on success,
`{"type": "error", "data": "..."}` on failure.

### `POST /api/upload`

`multipart/form-data` with one or more file parts. Each part is streamed to
`saves/<sanitised-name>`; memory usage is independent of file size.

Response: `{"type": "message", "data": "File(s) received"}`.

### `WS /api/stream` (planned)

Bidirectional WebSocket for real-time audio chunks. Not part of the stable
API yet.

---

## 🧪 Status

| Feature | Status |
|---|---|
| HTTP server (`/api/send`) | ✅ |
| HTTPS server (`/api/send`) | ✅ |
| Multipart upload (`/api/upload`) | ✅ |
| Public-IP detection (UPnP + HTTP) | ✅ |
| Local-IP enumeration | ✅ |
| WebSocket (`/api/stream`) | 🚧 in progress |
| Android client (OkHttp) | ✅ |
| Android client progress bar | 🚧 not wired to UI yet |
| Authentication | ❌ (planned) |
| Windows build | 🚧 untested |

---

## 📄 License

See `LICENSE.txt`.
