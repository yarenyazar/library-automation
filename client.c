#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define PORT 8081 // TCP port numarasýný tanýmlar.

int main() {
    WSADATA wsa; // Winsock kütüphanesiyle çalýþma baþlatmak için gerekli olan yapý.
    int sock; // TCP soketini temsil eden deðiþken.
    struct sockaddr_in server_addr;
    char buffer[1024]; // buffer, sunucudan alýnan yanýtlarý tutmak için; command, kullanýcýdan alýnan komutlarý tutmak için kullanýlýr.
    char command[256];

    // Winsock baþlatýlýyor
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    // Soket oluþtur
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    server_addr.sin_family = AF_INET; // IPv4
    server_addr.sin_port = htons(PORT);

    // IP adresini inet_addr ile ayarla (inet_pton yerine)
    server_addr.sin_addr.s_addr = inet_addr("192.168.15.232");

    // Server'a baðlan
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    printf("Connected to the server.\n");
    printf("Commands:\n");
    printf("  CHECK - View all table statuses\n");
    printf("  RESERVE <table_number> - Reserve a table (e.g., RESERVE 5)\n");

    // Kullanýcýdan komut al ve server'a gönder
    while (1) {
        printf("\nEnter command: ");
        fgets(command, sizeof(command), stdin);

        // Sonundaki yeni satýrý temizle
        command[strcspn(command, "\n")] = '\0';

        // Çýkýþ komutu
        if (strcmp(command, "EXIT") == 0) {
            printf("Exiting...\n");
            break;
        }

        // Komutu server'a gönder
        send(sock, command, strlen(command), 0);

        // Server'dan yanýt al
        int bytes_received = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0'; // Yanýtý null-terminate et
            printf("Server response:\n%s\n", buffer);
        } else {
            printf("Disconnected from the server.\n");
            break;
        }
    }

    // Soketi kapat
    closesocket(sock); // Soketi kapatýr ve Winsock kütüphanesini temizler.
    WSACleanup();
    return 0;
}

