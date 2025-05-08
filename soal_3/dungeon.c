#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "shop.h"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MONSTER_COUNT 3
#define BLUE    "\033[1;34m"

typedef struct {
    char name[50];
    int min_hp, max_hp;
    int min_dmg, max_dmg;
    int min_gold, max_gold;
} Monster;

typedef struct {
    int hp;
    int max_hp;
    int kills;
    PlayerStats stats;  // PlayerStats from shop.h
} FullStats;

typedef struct {
    int socket_fd;
    int player_id;
    FullStats stats;
    int in_battle;
    Monster current_enemy;
    int enemy_hp;
    char menu[32];
} Player;

Monster monsters[MONSTER_COUNT] = {
    {"Goblin", 50, 80, 5, 10, 30, 70},
    {"Skeleton Warrior", 80, 120, 10, 15, 50, 100},
    {"Dark Sorcerer", 120, 200, 15, 20, 100, 200}
};

int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

void init_default_stats(Player *player) {
    player->stats.stats.gold = 500;
    player->stats.stats.base_damage = 5;
    player->stats.kills = 0;
    player->stats.max_hp = 100;
    player->stats.hp = player->stats.max_hp;

    player->stats.stats.inventory.count = 0;
    memset(player->stats.stats.inventory.items, 0, sizeof(player->stats.stats.inventory.items));

    strcpy(player->stats.stats.equipped_weapon.name, "Fists");
    player->stats.stats.equipped_weapon.min_damage = 0;
    player->stats.stats.equipped_weapon.max_damage = 0;
    player->stats.stats.inventory.count = 0;
}

void main_menu(char *response, int player_id) {
    sprintf(response, "%s\n=== PLAYER %d MAIN MENU ===%s\n", GREEN, player_id, RESET);
    sprintf(response + strlen(response), "1. Show Player Stats\n");
    sprintf(response + strlen(response), "2. Shop (Buy Weapons)\n");
    sprintf(response + strlen(response), "3. View Inventory\n");
    sprintf(response + strlen(response), "4. Battle Mode\n");
    sprintf(response + strlen(response), "5. Exit Game\n");
    sprintf(response + strlen(response), "%sChoose an option: %s", YELLOW, RESET);
}

void show_stats(Player *player, char *response) {
    sprintf(response, "%s\n=== PLAYER STATS ===%s\n", GREEN, RESET);
    sprintf(response + strlen(response), "Gold: %d\n", player->stats.stats.gold);
    sprintf(response + strlen(response), "HP: %d/%d\n", player->stats.hp, player->stats.max_hp);
    sprintf(response + strlen(response), "Weapon: %s (Damage: %d-%d)\n",
          player->stats.stats.equipped_weapon.name,
          player->stats.stats.equipped_weapon.min_damage,
          player->stats.stats.equipped_weapon.max_damage);
    if (strlen(player->stats.stats.equipped_weapon.passive) > 0) {
        sprintf(response + strlen(response), "Passive: %s\n",
               player->stats.stats.equipped_weapon.passive);
    }
    sprintf(response + strlen(response), "Kills: %d\n", player->stats.kills);
    main_menu(response + strlen(response), player->player_id);
}

void battle_mode(Player *player, char *response) {
    srand(time(NULL));
    Monster enemy_template = monsters[rand() % MONSTER_COUNT];

    player->current_enemy = enemy_template;
    player->enemy_hp = random_range(enemy_template.min_hp, enemy_template.max_hp);
    player->in_battle = 1;

    sprintf(response, "%s\n=== BATTLE ===%s\n", GREEN, RESET);
    sprintf(response + strlen(response), "Enemy: %s (HP: %d)\n",
            enemy_template.name, player->enemy_hp);
    sprintf(response + strlen(response), "Your HP: %d/%d\n",
            player->stats.hp, player->stats.max_hp);
    sprintf(response + strlen(response), "Weapon: %s (Dmg: %d-%d)\n",
            player->stats.stats.equipped_weapon.name,
            player->stats.stats.equipped_weapon.min_damage,
            player->stats.stats.equipped_weapon.max_damage);
    sprintf(response + strlen(response), "%s1. Attack\n2. Run\nChoose action: %s",
            YELLOW, RESET);
}

void process_battle(Player *player, char *response) {
    int base_roll = random_range(1, player->stats.stats.base_damage);
    int weapon_roll = random_range(
        player->stats.stats.equipped_weapon.min_damage,
        player->stats.stats.equipped_weapon.max_damage
        );
    int damage = base_roll + weapon_roll;
    int is_critical = (rand() % 100) < 10; // 10% base critical chance

    if (strstr(player->stats.stats.equipped_weapon.name, "Dragon Claws") &&
        (rand() % 100) < player->stats.stats.equipped_weapon.passive_chance) {
        damage *= player->stats.stats.equipped_weapon.passive_effect;
        is_critical = 1;
    }

    if (strstr(player->stats.stats.equipped_weapon.name, "Staff of Light") &&
        (rand() % 100) < player->stats.stats.equipped_weapon.passive_chance) {
        player->enemy_hp = 0;
        sprintf(response, "%sPassive Activated: %s!%s\n", BLUE,
                player->stats.stats.equipped_weapon.passive, RESET);
    } else {
        if (is_critical) {
            damage *= 2;
            sprintf(response, "%sCritical Hit! %d Damage!%s\n", YELLOW, damage, RESET);
        } else {
            sprintf(response, "You deal %d damage!\n", damage);
        }
        player->enemy_hp -= damage;
    }

    if (player->enemy_hp <= 0) {
        int reward = random_range(player->current_enemy.min_gold, player->current_enemy.max_gold);
        player->stats.stats.gold += reward;
        player->stats.kills++;

        sprintf(response, "%sYou won! Earned %d gold!%s\n",
                GREEN, reward, RESET);

        Monster enemy_template = monsters[rand() % MONSTER_COUNT];
        player->current_enemy = enemy_template;
        player->enemy_hp = random_range(enemy_template.min_hp, enemy_template.max_hp);

        sprintf(response + strlen(response), "%s\nA new %s appears!%s\n", 
               RED, player->current_enemy.name, RESET);
        sprintf(response + strlen(response), "%s\n=== BATTLE ===%s\n", GREEN, RESET);
        sprintf(response + strlen(response), "Enemy: %s (HP: %d)\n",
                player->current_enemy.name, player->enemy_hp);
        sprintf(response + strlen(response), "Your HP: %d/%d\n",
                player->stats.hp, player->stats.max_hp);
        sprintf(response + strlen(response), "Weapon: %s (Dmg: %d-%d)\n",
                player->stats.stats.equipped_weapon.name,
                player->stats.stats.equipped_weapon.min_damage,
                player->stats.stats.equipped_weapon.max_damage);
        sprintf(response + strlen(response), "%s1. Attack\n2. Run\nChoose action: %s",
                YELLOW, RESET);
        return;
    }

    int enemy_dmg = random_range(player->current_enemy.min_dmg, player->current_enemy.max_dmg);
    player->stats.hp -= enemy_dmg;
    sprintf(response + strlen(response), "%s%s hits you for %d damage!%s\n",
            RED, player->current_enemy.name, enemy_dmg, RESET);

    if (player->stats.hp <= 0) {
        player->stats.hp = player->stats.max_hp;
        player->stats.stats.gold = player->stats.stats.gold / 2;
        player->in_battle = 0;

        sprintf(response + strlen(response), "%sYou died! Lost half your gold!%s\n",
                RED, RESET);
        main_menu(response + strlen(response), player->player_id);
        strcpy(player->menu, "main");
        return;
    }

    sprintf(response + strlen(response), "%s\nEnemy HP: %d | Your HP: %d\n%s",
            GREEN, player->enemy_hp, player->stats.hp, RESET);
    sprintf(response + strlen(response), "%s1. Attack\n2. Run\nChoose action: %s",
            YELLOW, RESET);
}

void handle_command(Player *player, char *buffer, char *response) {
    strcpy(response, "");

    if (strcmp(player->menu, "main") == 0) {
        if (strcmp(buffer, "1") == 0) {
            show_stats(player, response);
        } else if (strcmp(buffer, "2") == 0) {
            weapon_shop_menu(response);
            strcpy(player->menu, "shop");
        } else if (strcmp(buffer, "3") == 0) {
            show_inventory(&player->stats.stats, response);
            strcpy(player->menu, "inventory");
        } else if (strcmp(buffer, "4") == 0) {
            battle_mode(player, response);
            strcpy(player->menu, "battle");
        } else if (strcmp(buffer, "5") == 0) {
            strcpy(response, "exit");
        } else {
            sprintf(response, "%sInvalid option!%s\n", RED, RESET);
            main_menu(response + strlen(response), player->player_id);
        }
    }
    else if (strcmp(player->menu, "shop") == 0) {
        int weapon_index = atoi(buffer) - 1;
        if (weapon_index == -1) {
            strcpy(response, YELLOW "Cancelled. Returning to Main Menu...\n" RESET);
            main_menu(response + strlen(response), player->player_id);
            strcpy(player->menu, "main");
        } else {
            buy_weapon(&player->stats.stats, weapon_index, response);
        }
    }
    else if (strcmp(player->menu, "inventory") == 0) {
        int equip_index = atoi(buffer) - 1;
        if (equip_index == -1) {
            main_menu(response, player->player_id);
            strcpy(player->menu, "main");
        } else {
            equip_weapon(&player->stats.stats, equip_index, response);
        }
    }
    else if (strcmp(player->menu, "battle") == 0) {
        if (strcmp(buffer, "1") == 0) {
            process_battle(player, response);
        } else if (strcmp(buffer, "2") == 0) {
            strcpy(response, YELLOW "You ran away from battle!\n" RESET);
            player->in_battle = 0;
            main_menu(response + strlen(response), player->player_id);
            strcpy(player->menu, "main");
        } else {
            sprintf(response, "%sInvalid action!%s\n", RED, RESET);
            sprintf(response + strlen(response), "%s\nEnemy HP: %d | Your HP: %d\n%s",
                   GREEN, player->enemy_hp, player->stats.hp, RESET);
            sprintf(response + strlen(response), "%s1. Attack\n2. Run\nChoose action: %s",
                   YELLOW, RESET);
        }
    }
}

void *handle_client(void *arg) {
    Player *player = (Player *)arg;
    char buffer[BUFFER_SIZE];
    char response[BUFFER_SIZE * 2]; 

    init_default_stats(player);
    strcpy(player->menu, "main");

    main_menu(response, player->player_id);
    send(player->socket_fd, response, strlen(response), 0);

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = read(player->socket_fd, buffer, BUFFER_SIZE);
        if (bytes_read <= 0) {
            printf("Player %d disconnected\n", player->player_id);
            break;
        }

        buffer[strcspn(buffer, "\r\n")] = 0; 
        printf("Player %d command: %s\n", player->player_id, buffer);

        memset(response, 0, sizeof(response));
        handle_command(player, buffer, response);
        send(player->socket_fd, response, strlen(response), 0);

        if (strcmp(buffer, "5") == 0 && strcmp(player->menu, "main") == 0) {
            break; // Exit game
        }
    }

    close(player->socket_fd);
    free(player);
    pthread_exit(NULL);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    int player_count = 1;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }
   
        Player *player = malloc(sizeof(Player));
        player->socket_fd = new_socket;
        player->player_id = player_count++;
 
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void*)player) != 0) {
            perror("pthread_create");
            close(new_socket);
            free(player);
            continue;
        }

        printf("Player %d connected\n", player->player_id);
    }

    return 0;
}

