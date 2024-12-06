#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h> 
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8081
#define MAX_CLIENTS 10
#define TOTAL_TABLES 30

typedef struct {
    int status; // 0 boþ, 1 dolu
} Masa;

Masa masalar[TOTAL_TABLES];
CRITICAL_SECTION lock;

// Masalarýn durumunu döndürür
// CHECK yanýtýný hazýrlarken
void handle_check(char *response) {
	printf("Check komutu geldi");
    EnterCriticalSection(&lock);
    snprintf(response, 1024, "Tables: ");
    for (int i = 0; i < TOTAL_TABLES; i++) {
        char table_info[4];
        snprintf(table_info, sizeof(table_info), "%d ", masalar[i].status);
        strncat(response, table_info, 1024 - strlen(response) - 1);
    }
    LeaveCriticalSection(&lock);
    // Yanýtýn sonuna satýr sonlandýrýcý ekleyin
    strncat(response, "\n", 1024 - strlen(response) - 1);
}

// RESERVE yanýtýný hazýrlarken
void handle_reserve(char *response, int table_number) {
	printf("Reserve komutu geldi");
    if (table_number < 1 || table_number > TOTAL_TABLES) {
        snprintf(response, 1024, "Invalid");
        return;
    }

    EnterCriticalSection(&lock);
    if (masalar[table_number - 1].status == 0) {
        masalar[table_number - 1].status = 1;
        snprintf(response, 1024, "True", table_number);
    } else {
        snprintf(response, 1024, "False", table_number);
    }
    LeaveCriticalSection(&lock);
    // Yanýtýn sonuna satýr sonlandýrýcý ekleyin
    strncat(response, "\n", 1024 - strlen(response) - 1);
}


void handle_leave(char *response, int table_number) {
	printf("Leave komutu geldi");
    if (table_number < 1 || table_number > TOTAL_TABLES) {
        snprintf(response, 1024, "Invalid");
        return;
    }

    EnterCriticalSection(&lock);
    if (masalar[table_number - 1].status == 1) {
        masalar[table_number - 1].status = 0;
        snprintf(response, 1024, "True", table_number);
    } else {
        snprintf(response, 1024, "False", table_number);
    }
    LeaveCriticalSection(&lock);
    // Yanýtýn sonuna satýr sonlandýrýcý ekleyin
    strncat(response, "\n", 1024 - strlen(response) - 1);
}

DWORD WINAPI client_handler(void *socket_desc) {
    SOCKET sock = *(SOCKET *)socket_desc;
    free(socket_desc);
    char buffer[1024];
    int read_size;

    while ((read_size = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        buffer[read_size] = '\0';
        char response[1024];

        if (strncmp(buffer, "CHECK", 5) == 0) {
            handle_check(response);
        } else if (strncmp(buffer, "RESERVE", 7) == 0) {
            int table_number = atoi(&buffer[8]);
            handle_reserve(response, table_number);
        }else if (strncmp(buffer, "LEAVE", 5) == 0) {
            int table_number = atoi(&buffer[6]);
            handle_leave(response, table_number);
        }
		
		 else {
            snprintf(response, sizeof(response), "Unknown command. Use CHECK or RESERVE <table_number>.\n");
        }

        send(sock, response, strlen(response), 0);
    }

    closesocket(sock);
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET server_fd, new_socket, *new_sock;
    struct sockaddr_in server, client;
    int client_len = sizeof(client);

    for (int i = 0; i < TOTAL_TABLES; i++) {
        masalar[i].status = 0;
    }
    InitializeCriticalSection(&lock);

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock initialization failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Socket creation failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }

    server.sin_family = AF_INET; // IPv4 adres ailesinden olduðunu ifade eder.
    server.sin_addr.s_addr = INADDR_ANY; // Yerel adreslerden gelen baðlantýya izin verir.
    server.sin_port = htons(PORT); // Port numarasýný ayarlar.

    if (bind(server_fd, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        return 1; // Sunucu adresi ve port numarasýna baðlanýr.
    }

    if (listen(server_fd, MAX_CLIENTS) == SOCKET_ERROR) {
        printf("Listen failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_fd);
        return 1;
    }

    printf("Server is running on port %d\n", PORT);

    while ((new_socket = accept(server_fd, (struct sockaddr *)&client, &client_len)) != INVALID_SOCKET) {
        printf("New connection accepted\n");

        HANDLE thread;
        new_sock = malloc(sizeof(SOCKET));
        *new_sock = new_socket;

        thread = CreateThread(NULL, 0, client_handler, (void *)new_sock, 0, NULL);
        if (thread == NULL) {
            printf("Thread creation failed.\n");
            free(new_sock);
        } else {
            CloseHandle(thread);
        }
    }

    if (new_socket == INVALID_SOCKET) {
        printf("Accept failed. Error Code: %d\n", WSAGetLastError());
    }

    closesocket(server_fd);
    DeleteCriticalSection(&lock);
    WSACleanup();
    return 0;
}
