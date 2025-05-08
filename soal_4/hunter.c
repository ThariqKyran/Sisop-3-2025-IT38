#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_HUNTERS 100
#define MAX_DUNGEONS 100
#define USERNAME_LEN 50
#define DUNGEON_NAME_LEN 50
#define KEY_PATH "."
#define PROJ_ID 123

// Hunter struct
typedef struct {
    char username[USERNAME_LEN];
    int level, exp;
    int atk, hp, def;
    bool banned;
    bool notif_active;
    bool used;
} Hunter;

// Dungeon struct
typedef struct {
    char name[DUNGEON_NAME_LEN];
    int min_level;
    int reward_exp, reward_atk, reward_hp, reward_def;
    bool used;
} Dungeon;

// Shared memory struct
typedef struct {
    Hunter hunters[MAX_HUNTERS];
    Dungeon dungeons[MAX_DUNGEONS];
    bool shutdown;
} SharedData;

SharedData *data;
int shmid;
int current_id = -1;

void toggle_notification() {
    data->hunters[current_id].notif_active = !data->hunters[current_id].notif_active;
    if (data->hunters[current_id].notif_active) {
        printf("Notification activated!\n");
        while (data->hunters[current_id].notif_active) {
            system("clear");
            for (int i = 0; i < MAX_DUNGEONS; i++) {
                if (data->dungeons[i].used && data->dungeons[i].min_level <= data->hunters[current_id].level) {
                    printf("D-Rank Dungeon for minimum level %d opened!\n", data->dungeons[i].min_level);
                    break;
                }
            }
            sleep(3);
        }
    } else {
        printf("Notification deactivated!\n");
    }
}

void list_dungeons() {
    printf("\n=== AVAILABLE DUNGEON ===\n");
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        Dungeon d = data->dungeons[i];
        if (d.used && d.min_level <= data->hunters[current_id].level) {
            printf("%d. %s (Level %d+)\n", i + 1, d.name, d.min_level);
        }
    }
    printf("Press enter to continue..."); getchar(); getchar();
}

void raid_dungeon() {
    list_dungeons();
    int choice;
    printf("Choose dungeon: "); scanf("%d", &choice); choice--;
    if (choice < 0 || choice >= MAX_DUNGEONS || !data->dungeons[choice].used || data->dungeons[choice].min_level > data->hunters[current_id].level) {
        printf("Invalid dungeon.\n"); return;
    }
    Dungeon d = data->dungeons[choice];
    data->hunters[current_id].atk += d.reward_atk;
    data->hunters[current_id].hp  += d.reward_hp;
    data->hunters[current_id].def += d.reward_def;
    data->hunters[current_id].exp += d.reward_exp;
    if (data->hunters[current_id].exp >= 500) {
        data->hunters[current_id].level++;
        data->hunters[current_id].exp = 0;
    }
    data->dungeons[choice].used = false;
    printf("Raid success! Gained: ATK: %d, HP: %d, DEF: %d, EXP: %d\n",
           d.reward_atk, d.reward_hp, d.reward_def, d.reward_exp);
    printf("Press enter to continue..."); getchar(); getchar();
}

void battle_hunter() {
    printf("=== PVP LIST ===\n");
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (i != current_id && data->hunters[i].used) {
            int power = data->hunters[i].atk + data->hunters[i].hp + data->hunters[i].def;
            printf("%s - Total Power: %d\n", data->hunters[i].username, power);
        }
    }
    char target[USERNAME_LEN];
    printf("Target: "); scanf("%s", target);
    int id = -1;
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (data->hunters[i].used && strcmp(data->hunters[i].username, target) == 0) {
            id = i; break;
        }
    }
    if (id == -1 || id == current_id) return;
    int my_power = data->hunters[current_id].atk + data->hunters[current_id].hp + data->hunters[current_id].def;
    int op_power = data->hunters[id].atk + data->hunters[id].hp + data->hunters[id].def;
    printf("You chose to battle %s\n", target);
    printf("Your Power: %d\nOpponent's Power: %d\n", my_power, op_power);
    if (my_power >= op_power) {
        data->hunters[current_id].atk += data->hunters[id].atk;
        data->hunters[current_id].hp  += data->hunters[id].hp;
        data->hunters[current_id].def += data->hunters[id].def;
        data->hunters[id].used = false;
        printf("Battle won! You acquired %s's stats\n", target);
    } else {
        data->hunters[id].atk += data->hunters[current_id].atk;
        data->hunters[id].hp  += data->hunters[current_id].hp;
        data->hunters[id].def += data->hunters[current_id].def;
        data->hunters[current_id].used = false;
        printf("You lost and your data is deleted.\n");
        exit(0);
    }
    printf("Press enter to continue..."); getchar(); getchar();
}

void hunter_menu() {
    while (1) {
        if (data->hunters[current_id].banned) {
            printf("You are banned from raiding or battling.\n");
        }
        printf("\n=== %s's MENU ===\n", data->hunters[current_id].username);
        printf("1. List dungeon\n2. Raid\n3. Battle\n4. Toggle Notification\n5. Exit\nChoice: ");
        int ch; scanf("%d", &ch);
        if (ch == 1) list_dungeons();
        else if (ch == 2 && !data->hunters[current_id].banned) raid_dungeon();
        else if (ch == 3 && !data->hunters[current_id].banned) battle_hunter();
        else if (ch == 4) toggle_notification();
        else if (ch == 5) break;
    }
}

void register_hunter() {
    char name[USERNAME_LEN];
    printf("Username: "); scanf("%s", name);

    // Cek apakah username sudah ada di hunter yang aktif (used = true)
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (data->hunters[i].used && strcmp(data->hunters[i].username, name) == 0) {
            printf("Hunter already registered\n");
            printf("Registration failed.\n");
            return; // Langsung keluar jika username sudah ada
        }
    }

    // Jika username belum ada, cari slot kosong
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (!data->hunters[i].used) {
            strcpy(data->hunters[i].username, name);
            data->hunters[i].level = 1;
            data->hunters[i].exp = 0;
            data->hunters[i].atk = 10;
            data->hunters[i].hp = 100;
            data->hunters[i].def = 5;
            data->hunters[i].banned = false;
            data->hunters[i].notif_active = false;
            data->hunters[i].used = true;
            printf("Registration success!\n");
            return;
        }
    }

    // Jika tidak ada slot kosong
    printf("Registration failed.\n");
}
void login_hunter() {
    char name[USERNAME_LEN];
    printf("Username: "); scanf("%s", name);
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (data->hunters[i].used && strcmp(data->hunters[i].username, name) == 0) {
            current_id = i;
            printf("Login success!\n");
            hunter_menu();
            return;
        }
    }
    printf("Login failed.\n");
}

int main() {
    key_t key = ftok(KEY_PATH, PROJ_ID);
    shmid = shmget(key, sizeof(SharedData), 0666);
    if (shmid < 0) {
        printf("System not running. Start system.c first.\n");
        exit(1);
    }
    data = (SharedData *)shmat(shmid, NULL, 0);

    while (!data->shutdown) {
        printf("\n=== HUNTER MENU ===\n1. Register\n2. Login\n3. Exit\nChoice: ");
        int ch; scanf("%d", &ch);
        if (ch == 1) register_hunter();
        else if (ch == 2) login_hunter();
        else if (ch == 3) break;
    }

    shmdt(data);
    return 0;
}
