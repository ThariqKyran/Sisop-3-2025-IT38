// ========== dungeon.c ==========
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "shop.c"

#define MAX_INVENTORY 10
#define PORT 8080

typedef struct {
    int gold;
    int base_damage;
    Weapon equipped;
    Weapon inventory[MAX_INVENTORY];
    int inventory_count;
    int kills;
} Player;

Player player;

char output[1024];

void show_stats() {
    sprintf(output, "\n=== PLAYER STATS ===\nGold: %d | Equipped Weapon: %s | Base Damage: %d | Kills: %d",
        player.gold, player.equipped.name, player.base_damage, player.kills);
    if (strlen(player.equipped.passive) > 0)
        sprintf(output + strlen(output), " | Passive: %s", player.equipped.passive);
}

void shop_menu() {
    char temp[256];
    strcpy(output, "=== WEAPON SHOP ===\n");
    for (int i = 0; i < MAX_WEAPONS; i++) {
        sprintf(temp, "[%d] %s - Price: %d gold, Damage: %d",
            i + 1, weapon_list[i].name, weapon_list[i].price, weapon_list[i].damage);
        strcat(output, temp);
        if (strlen(weapon_list[i].passive) > 0) {
            sprintf(temp, " (Passive: %s)", weapon_list[i].passive);
            strcat(output, temp);
        }
        strcat(output, "\n");
    }
    strcat(output, "You bought the first item for demo.\n");
    Weapon w = get_weapon(0);
    if (player.gold >= w.price) {
        player.inventory[player.inventory_count++] = w;
        player.gold -= w.price;
        strcat(output, "You bought Terra Blade!\n");
    } else {
        strcat(output, "Not enough gold.\n");
    }
}

void inventory_menu() {
    strcpy(output, "\n=== YOUR INVENTORY ===\n");
    for (int i = 0; i < player.inventory_count; i++) {
        sprintf(output + strlen(output), "[%d] %s\n", i, player.inventory[i].name);
    }
    strcpy(player.equipped.name, player.inventory[0].name);
    player.base_damage = player.inventory[0].damage;
    strcat(output, "Equipped first weapon for demo.\n");
}

void battle_mode() {
    int enemy_hp = 50 + rand() % 151;
    int dmg = player.base_damage + (rand() % 6);
    sprintf(output, "Enemy appeared with %d HP.\nYou dealt %d damage.\n", enemy_hp, dmg);
    enemy_hp -= dmg;
    if (enemy_hp <= 0) {
        int reward = 50 + rand() % 101;
        sprintf(output + strlen(output), "Enemy defeated! You earned %d gold!\n", reward);
        player.gold += reward;
        player.kills++;
    } else {
        sprintf(output + strlen(output), "Enemy HP remaining: %d\n", enemy_hp);
    }
}

int main() {
    srand(time(NULL));
    init_shop();
    player.gold = 500;
    strcpy(player.equipped.name, "Fists");
    player.base_damage = 5;
    player.kills = 0;
    player.inventory_count = 1;
    player.inventory[0] = player.equipped;

    int server_fd, new_socket, opt;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Server started on port %d\n", PORT);

    while ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
        while (1) {
            int valread = read(new_socket, &opt, sizeof(int));
            if (opt == 5 || valread <= 0) break;

            switch (opt) {
                case 1: show_stats(); break;
                case 2: shop_menu(); break;
                case 3: inventory_menu(); break;
                case 4: battle_mode(); break;
                default: strcpy(output, "Invalid option.\n"); break;
            }

            send(new_socket, output, strlen(output), 0);
        }
        close(new_socket);
    }
    return 0;
}
