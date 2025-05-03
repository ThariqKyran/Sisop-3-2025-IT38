// ========== player.c ==========
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    while (1) {
        printf("\n===== MAIN MENU =====\n");
        printf("1. Show Player Stats\n");
        printf("2. Enter Weapon Shop\n");
        printf("3. Manage Inventory & Equip Weapons\n");
        printf("4. Battle Mode\n");
        printf("5. Exit Game\n");
        printf("Choose an option: ");
        int opt;
        scanf("%d", &opt);

        send(sock, &opt, sizeof(int), 0);
        if (opt == 5) break;

        int len = read(sock, buffer, 1024);
        buffer[len] = '\0';
        printf("%s\n", buffer);
    }

    close(sock);
    return 0;
}