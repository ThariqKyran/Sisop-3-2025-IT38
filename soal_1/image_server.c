#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

#define PORT 8080
#define LOG_PATH "/home/reiziqzip/modul3/soal_1/server/server.log"
#define DB_PATH "/home/reiziqzip/modul3/soal_1/server/database/"
#define MAX_BUF 65536

void log_message(const char *source, const char *action, const char *info) {
    FILE *log = fopen(LOG_PATH, "a");
    if (!log) {
        perror("fopen log");
        return;
    }
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", t);
    fprintf(log, "[%s][%s]: [%s] [%s]\n", source, time_str, action, info);
    fclose(log);
}

void hex_to_bytes(const char *hex, unsigned char *bytes, size_t *out_len) {
    size_t len = strlen(hex) / 2;
    for (size_t i = 0; i < len; i++) {
        sscanf(hex + 2 * i, "%2hhx", &bytes[i]);
    }
    *out_len = len;
}

void reverse_string(char *str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

void handle_client(int client_sock) {
    char buffer[MAX_BUF];
    memset(buffer, 0, sizeof(buffer));
    int received = recv(client_sock, buffer, sizeof(buffer), 0);
    if (received <= 0) {
        perror("recv failed");
        close(client_sock);
        return;
    }

    if (strncmp(buffer, "DECRYPT", 7) == 0) {
        char *text = buffer + 8;
        log_message("Client", "DECRYPT", "<data diterima>");
        reverse_string(text);
        unsigned char *bytes = malloc(strlen(text) / 2);
        size_t len;
        hex_to_bytes(text, bytes, &len);

        time_t now = time(NULL);
        char filename[128];
        sprintf(filename, "%ld.jpeg", now);
        char path[512];
        sprintf(path, "%s%s", DB_PATH, filename);

        FILE *f = fopen(path, "wb");
        if (f) {
            fwrite(bytes, 1, len, f);
            fclose(f);
            log_message("Server", "SAVE", filename);
            send(client_sock, filename, strlen(filename), 0);
        } else {
            perror("fopen image");
            log_message("Server", "ERROR", "Gagal menyimpan file");
            send(client_sock, "[ERROR] Gagal menyimpan file.", 30, 0);
        }
        free(bytes);

    } else if (strncmp(buffer, "DOWNLOAD", 8) == 0) {
        char *fname = buffer + 9;
        log_message("Client", "DOWNLOAD", fname);
        char path[512];
        sprintf(path, "%s%s", DB_PATH, fname);

        FILE *f = fopen(path, "rb");
        if (f) {
            fseek(f, 0, SEEK_END);
            long size = ftell(f);
            rewind(f);
            unsigned char *filedata = malloc(size);
            fread(filedata, 1, size, f);
            fclose(f);
            send(client_sock, filedata, size, 0);
            log_message("Server", "UPLOAD", fname);
            free(filedata);
        } else {
            log_message("Server", "ERROR", "Gagal menemukan file untuk dikirim ke client");
            send(client_sock, "[ERROR] File tidak ditemukan.", 30, 0);
        }

    } else if (strncmp(buffer, "EXIT", 4) == 0) {
        log_message("Client", "EXIT", "Client requested to exit");
        send(client_sock, "[EXIT] Client disconnected.", 30, 0);

    } else {
        log_message("Server", "ERROR", "Perintah tidak dikenali");
        send(client_sock, "[ERROR] Perintah tidak dikenali.", 32, 0);
    }

    close(client_sock);
}

int main() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);
    setsid();
    umask(0);
    chdir("/home/reiziqzip/modul3/soal_1");

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    while (1) {
        int client_sock = accept(server_sock, NULL, NULL);
        if (client_sock >= 0) {
            if (fork() == 0) {
                handle_client(client_sock);
                exit(0);
            }
            close(client_sock);
        }
    }
    close(server_sock);
    return 0;
}