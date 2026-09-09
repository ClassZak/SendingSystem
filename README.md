# Sending System

Simple program for sending data from one computer to another.

This program can transfer data from a smartphone to a computer, but the ability to transfer data from a computer to a smartphone will also be added.

There are Linux server and Android Client

**⚠️ ATTENTION!!!**

The production of Apple products (iPhones and other devices) will not be supported but Windows 7, 10 and 11 will be.

## 🛠 Dependencies

### Linux Server Dependencies

1. OpenSSL 3.6.4-1
2. cJSON https://github.com/DaveGamble/cJSON.git v1.7.18
3. SQLite3 3.53.4-1

### Android Client Dependencies

1. androidx.room:room-runtime 2.6.1
2. androidx.room:room-compiler 2.6.1

## 🖼️ Program Appearance

### Linux Server Appearance

<p align="center">
	<img align="center" src="https://github.com/ClassZak/SendingSystem/blob/master/screenshots/screenshot1.png"/>
</p>
<p align="center">
	<img align="center" src="https://github.com/ClassZak/SendingSystem/blob/master/screenshots/screenshot2.png"/>
</p>
<p align="center">
	<img align="center" src="https://github.com/ClassZak/SendingSystem/blob/master/screenshots/screenshot3.png"/>
</p>

### Android Client Appearance

<p align="center">
	<img align="center" src="https://github.com/ClassZak/SendingSystem/blob/master/screenshots/screenshot4.png"/>
</p>

## 🏗 🚀 Building and launching

### Linux Server

1. Clone repository

2. Edit Server.c file 

```c
#define BUFFER_SIZE 4096
#define TIMEOUT_SEC 50
#define MAX_MESSAGE_SIZE 0x6400000
#define IP_SOLVER_SERVER_DOMAIN "icanhazip.com"
#define HTTP_REQUEST "GET / HTTP/1.1\r\nHost: icanhazip.com\r\nConnection: close\r\n\r\n"
#define FAILED_SIGNAL_HANDLERS_MESSAGE "Failed to set up signal handlers\n"
#define FAILED_TO_SET_THREAD_ATTRS "Failed to set up thread attributes\n"
const unsigned short CONNECTION_PORT = 5000;
const char* CONNECTION_IP = "0.0.0.0";
const int PORT_RECONNECTION_SLEEP_TIME = 5;
const bool SSL_ENCRYPT = true;
const char* PUBLIC_KEY_FILE = "server.crt";
const char* PRIVATE_KEY_FILE = "server.key";
```

3. Generate server public and private key files if SSL_ENCRYPT flag allowed.

4. Build with Cmake

Find corresponding preset
```bash
cmake --list-presets
```

Build with preset

```bash
mkdir -p build && cd build
cmake .. --list-presets
cmake .. --preset linux-debug
cd ..
cmake --build ./build --preset
```

4. Launch

```bash
./build/SendingSystem
```

### Android Client

1. Sync gradle

2. Run project
