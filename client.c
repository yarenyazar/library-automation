#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8081 // TCP port numaras�n� tan�mlar.

int main() {
    WSADATA wsa; // Winsock k�t�phanesiyle �al��ma ba�latmak i�in gerekli olan yap�.
    int sock; // TCP soketini temsil eden de�i�ken.
    struct sockaddr_in server_addr;
    char buffer[1024]; // buffer, sunucudan al�nan yan�tlar� tutmak i�in; command, kullan�c�dan al�nan komutlar� tutmak i�in kullan�l�r.
    char command[256];

    // Winsock ba�lat�l�yor
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // Soket olu�tur
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(PORT);

    // IP adresini inet_addr ile ayarla (inet_pton yerine)
    server_addr.sin_addr.s_addr = inet_addr("192.168.15.232");

    // Server'a ba�lan
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to the server.\n");
    printf("Commands:\n");
    printf("  CHECK - View all table statuses\n");
    printf("  RESERVE <table_number> - Reserve a table (e.g., RESERVE 5)\n");

    // Kullan�c�dan komut al ve server'a g�nder
    while (1) {
        printf("\nEnter command: ");
        fgets(command, sizeof(command), stdin);

        // Sonundaki yeni sat�r� temizle
        command[strcspn(command, "\n")] = '\0';

        // ��k�� komutu
        if (strcmp(command, "EXIT") == 0) {
            printf("Exiting...\n");
            break;
        }

        // Komutu server'a g�nder
        send(sock, command, strlen(command), 0);

        // Server'dan yan�t al
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0'; // Yan�t� null-terminate et
            printf("Server response:\n%s\n", buffer);
        } else {
            printf("Disconnected from the server.\n");
            break;
        }
    }

    // Soketi kapat
    closesocket(sock); // Soketi kapat�r ve Winsock k�t�phanesini temizler.
    WSACleanup();
    return 0;
}

