#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")

#include <Windows.h>
#include <locale.h>
#include <stdbool.h>

#include "Server.h"
#include "cJSON.h"

#define BUFFER_SIZE 4096  // Увеличенный буфер
#define TIMEOUT_SEC 10    // Таймаут в секундах

// ANSI коды цветов
#define RED "\x1b[31m"
#define GREEN "\x1b[32m"
#define YELLOW "\x1b[33m"
#define RESET "\x1b[0m"

#define HTTP_REQUEST "GET / HTTP/1.1\r\nHost: icanhazip.com\r\nConnection: close\r\n\r\n"

void print_error(const char* message);
void print_success(const char* message);
void print_info(const char* message);
void send_json(SOCKET sock, const char* type, const char* data);
bool set_socket_timeout(SOCKET sock, int timeout_sec);

static inline void exit_with_error(const char* message, int code);

char* get_global_ip();



int main(int argc, char** argv) 
{
	setlocale(LC_ALL, "Ru");

	
	print_info("Инициализация сервера на 127.0.0.1:5000");

	WSADATA wsaData;
	int res = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (res)
	{
		print_error("Ошибка инициализации WinSock");
		exit(res);
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET)
	{
		print_error("Ошибка создания сокета");
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	// Определение IP шлюза
	{
		char* message="IP сервера: ";
		char* ipAddressStr=get_global_ip();
		size_t totalLength=strlen(message)+strlen(ipAddressStr);
		char* concated = malloc(totalLength + 1);
		if (!concated)
		{
			exit_with_error("Ошибка выделения памяти", 1);
			return 1;
		}
		ZeroMemory(concated, totalLength+1);
		strcat(concated, message);
		strcat(concated, ipAddressStr);
		concated[totalLength]='\0';
		print_info(concated);
		free(ipAddressStr);
	}
	// Настройка адреса сервера
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(5000);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	if (bind(serverSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		print_error("Ошибка привязки сокета");
		closesocket(serverSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		print_error("Ошибка прослушивания порта");
		closesocket(serverSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	print_success("Сервер запущен и ожидает подключений");

	// Принятие подключения
	SOCKET clientSocket;
	struct sockaddr_in clientAddr;
	int clientAddrSize = sizeof(clientAddr);

	if ((clientSocket = accept(serverSocket, (SOCKADDR*)&clientAddr, &clientAddrSize)) == INVALID_SOCKET)
	{
		print_error("Ошибка принятия подключения");
		closesocket(serverSocket);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	// Вывод IP клиента
	char clientIP[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
	printf(YELLOW "Подключен клиент: %s:%d\n" RESET, clientIP, ntohs(clientAddr.sin_port));

	// Установка таймаута
	if (!set_socket_timeout(clientSocket, TIMEOUT_SEC))
		print_error("Ошибка установки таймаута");

	// Обработка данных
	char* buffer = malloc(BUFFER_SIZE);
	if (buffer==NULL)
	{
		print_error("Ошибка выделения памяти");
		return -1; // Даунская Visual Studio не понимает void exit(int _Code)
	}

	char* bigBuffer = NULL;
	while (1)
	{
		int totalReceived = 0;
		int bytesReceived;

		// Чтение больших данных

		while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0)), bytesReceived > 0)
		{
			totalReceived += bytesReceived;
			if (totalReceived >= BUFFER_SIZE) break;
		}


		if (bytesReceived <= 0)
		{
			if (bytesReceived == 0)
				print_info("Клиент отключился");
			else if (WSAGetLastError() == WSAETIMEDOUT)
				print_error("Таймаут соединения");
			break;
		}

		// Парсинг JSON
		cJSON* root = cJSON_Parse(buffer);
		if (!root)
		{
			print_error("Получен некорректный JSON");
			continue;
		}

		// Обработка сообщения...
		cJSON_Delete(root);
	}

	free(buffer);
	closesocket(clientSocket);
	closesocket(serverSocket);
	WSACleanup();
	return EXIT_SUCCESS;
}

bool set_socket_timeout(SOCKET sock, int timeout_sec)
{
	struct timeval tv;
	tv.tv_sec = timeout_sec;
	tv.tv_usec = 0;
	return setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO,
		(const char*)&tv, sizeof(tv)) == 0;
}

void print_error(const char* message)
{
	fprintf(stderr, RED "[ОШИБКА] %s (код: %d)\n" RESET, message, WSAGetLastError());
}


void print_success(const char* message)
{
	printf(GREEN "[УСПЕХ] %s\n" RESET, message);
}

void print_info(const char* message)
{
	printf(YELLOW "[ИНФО] %s\n" RESET, message);
}




inline void exit_with_error(const char* message, int code)
{
	print_error(message);
	exit(code);
}



char* get_global_ip()
{
    WSADATA wsaData;
    SOCKET sock = INVALID_SOCKET;
    struct addrinfo hints, *result = NULL, *ptr = NULL;
    char* ip = NULL;
    char recvbuf[BUFFER_SIZE];
    int iResult;

    // 1. Инициализация Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
        fprintf(stderr, "WSAStartup failed: %d\n", WSAGetLastError());
        return NULL;
    }

    // 2. Настройка параметров для getaddrinfo
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // 3. Разрешение имени хоста
    if ((iResult = getaddrinfo("icanhazip.com", "80", &hints, &result)) != 0)
	{
        fprintf(stderr, "getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return NULL;
    }

    // 4. Перебор возможных адресов и подключение
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
        sock = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (sock == INVALID_SOCKET)
		{
            fprintf(stderr, "socket failed: %d\n", WSAGetLastError());
            continue;
        }

        if (connect(sock, ptr->ai_addr, (int)ptr->ai_addrlen) == SOCKET_ERROR)
		{
            closesocket(sock);
            sock = INVALID_SOCKET;
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    if (sock == INVALID_SOCKET)
	{
        fprintf(stderr, "Unable to connect to server\n");
        WSACleanup();
        return NULL;
    }

    // 5. Отправка HTTP-запроса
    if ((iResult = send(sock, HTTP_REQUEST, (int)strlen(HTTP_REQUEST), 0)) == SOCKET_ERROR)
	{
        fprintf(stderr, "send failed: %d\n", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
        return NULL;
    }

    // 6. Получение ответа
    int total_received = 0;
    char* response = NULL;
    do
	{
        iResult = recv(sock, recvbuf, BUFFER_SIZE - 1, 0);
        if (iResult > 0)
		{
            recvbuf[iResult] = '\0';
            char* temp = realloc(response, total_received + iResult + 1);
            if (!temp)
			{
                free(response);
                closesocket(sock);
                WSACleanup();
                return NULL;
            }
            response = temp;
            memcpy(response + total_received, recvbuf, iResult);
            total_received += iResult;
            response[total_received] = '\0';
        }
        else if (iResult == 0)
            break;
        else
		{
            fprintf(stderr, "recv failed: %d\n", WSAGetLastError());
            free(response);
            closesocket(sock);
            WSACleanup();
            return NULL;
        }
    } while (iResult > 0);

    // 7. Парсинг ответа
    if (response)
	{
        char* body = strstr(response, "\r\n\r\n");
        if (body)
		{
            body += 4;
            char* end = body + strcspn(body, "\r\n\t ");
            size_t len = end - body;
            ip = malloc(len + 1);
            if (ip)
			{
                strncpy(ip, body, len);
                ip[len] = '\0';
            }
        }
        free(response);
    }

    // 8. Очистка ресурсов
    closesocket(sock);
    WSACleanup();

    return ip;
}