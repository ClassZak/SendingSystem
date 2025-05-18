#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

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

void print_error(const char* message);
void print_success(const char* message);
void print_info(const char* message);
void send_json(SOCKET sock, const char* type, const char* data);
bool set_socket_timeout(SOCKET sock, int timeout_sec);

inline void exit_with_error(const char* message, int code);



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