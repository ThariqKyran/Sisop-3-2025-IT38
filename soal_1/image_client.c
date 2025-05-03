#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define MAX_BUF 65536

void menu() {
    printf("\n===== MENU =====\n");
    printf("1. Kirim dan decrypt file txt\n");
    printf("2. Download file jpeg dari server\n");
    printf("3. Exit\n");
    printf("Masukkan pilihan Anda: ");
}

void connect_and_send(const char *message, size_t message_len, const char *save_path, int expect_file) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Gagal connect ke server");
        close(sock);
        return;
    }

    send(sock, message, message_len, 0);

    if (expect_file) {
        FILE *f = fopen(save_path, "wb");
        if (!f) {
            perror("fopen client save_path");
            close(sock);
            return;
        }
        unsigned char buffer[4096];
        int bytes;
        while ((bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
            fwrite(buffer, 1, bytes, f);
        }
        fclose(f);
        printf("File berhasil disimpan di %s\n", save_path);
    } else {
        char reply[512] = {0};
        int len = recv(sock, reply, sizeof(reply) - 1, 0);
        if (len > 0) {
            reply[len] = '\0';
            printf("Server: %s\n", reply);
        } else {
            printf("Gagal menerima balasan dari server.\n");
        }
    }

    close(sock);
}

int main() {
    while (1) {
        menu();
        int choice;
        scanf("%d", &choice);
        getchar();

        if (choice == 1) {
            char fname[128];
            printf("Masukkan nama file txt (di folder client/secrets/): ");
            scanf("%s", fname);
            getchar();

            char path[256];
            sprintf(path, "client/secrets/%s", fname);
            FILE *f = fopen(path, "r");
            if (!f) {
                printf("Gagal membuka file.\n");
                continue;
            }

            char *hex = malloc(MAX_BUF);
            size_t len = fread(hex, 1, MAX_BUF - 1, f);
            fclose(f);
            hex[len] = '\0';

            char *message = malloc(len + 16);
            sprintf(message, "DECRYPT %s", hex);
            connect_and_send(message, strlen(message), NULL, 0);
            free(hex);
            free(message);

        } else if (choice == 2) {
            char fname[128];
            printf("Masukkan nama file jpeg: ");
            scanf("%s", fname);
            getchar();

            char message[256];
            sprintf(message, "DOWNLOAD %s", fname);

            char save_path[256];
            sprintf(save_path, "client/%s", fname);

            connect_and_send(message, strlen(message), save_path, 1);

        } else if (choice == 3) {
            char *exit_message = "EXIT";
            connect_and_send(exit_message, strlen(exit_message), NULL, 0);
            printf("Keluar dari program.\n");
            break;

        } else {
            printf("Pilihan tidak valid.\n");
        }
    }
    return 0;
}