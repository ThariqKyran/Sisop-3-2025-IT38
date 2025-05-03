#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_HUNTERS 100
#define MAX_DUNGEONS 100
#define USERNAME_LEN 50
#define DUNGEON_NAME_LEN 50
#define KEY_PATH "."
#define PROJ_ID 123

typedef struct {
    char username[USERNAME_LEN];
    int level, exp;
    int atk, hp, def;
    bool banned;
    bool notif_active;
    bool used;
} Hunter;

typedef struct {
    char name[DUNGEON_NAME_LEN];
    int min_level;
    int reward_exp, reward_atk, reward_hp, reward_def;
    bool used;
} Dungeon;

typedef struct {
    Hunter hunters[MAX_HUNTERS];
    Dungeon dungeons[MAX_DUNGEONS];
    bool shutdown;
} SharedData;

int shmid;
SharedData *data;

void detach_and_cleanup() {
    if (data != NULL) shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    printf("\n[System Menu] Shared memory cleaned up.\n");
    exit(0);
}

void handle_sigint(int sig) {
    data->shutdown = true;
    detach_and_cleanup();
}

void list_hunters() {
    printf("\n=== HUNTER INFO ===\n");
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (!data->hunters[i].used) continue;
        Hunter h = data->hunters[i];
        printf("Name: %-15s Level: %d    EXP: %d    ATK: %d    HP: %d    DEF: %d\n",
               h.username, h.level, h.exp, h.atk, h.hp, h.def);
    }
}

void list_dungeons_info() {
    printf("\n=== DUNGEON INFO ===\n");
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (!data->dungeons[i].used) continue;
        Dungeon *d = &data->dungeons[i];
        printf("[Dungeon %d]\nName: %s\nMinimum Level: %d\nEXP Reward: %d\nATK: %d\nHP: %d\nDef: %d\nKey: %d\n\n",
               i + 1, d->name, d->min_level, d->reward_exp,
               d->reward_atk, d->reward_hp, d->reward_def, (int)(intptr_t)d);
    }
}

void generate_dungeon() {
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        if (!data->dungeons[i].used) {
            Dungeon *d = &data->dungeons[i];
            snprintf(d->name, DUNGEON_NAME_LEN, "Dungeon%d", i+1);
            d->min_level = rand() % 5 + 1;
            d->reward_exp = rand() % 151 + 150;
            d->reward_atk = rand() % 51 + 100;
            d->reward_hp  = rand() % 51 + 50;
            d->reward_def = rand() % 26 + 25;
            d->used = true;
            printf("\nDungeon generated!\nName: %s\nMinimum level: %d\n", d->name, d->min_level);
            return;
        }
    }
    printf("\nDungeon list full.\n");
}

void ban_hunter(bool ban) {
    char name[USERNAME_LEN];
    printf("Masukkan nama Hunter yang ingin di%sban: ", ban ? "" : "un");
    scanf("%s", name);
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (data->hunters[i].used && strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].banned = ban;
            printf("[System Menu] Hunter %s berhasil di%sban\n", name, ban ? "" : "un");
            return;
        }
    }
    printf("[System Menu] Hunter tidak ditemukan\n");
}

void reset_hunter() {
    char name[USERNAME_LEN];
    printf("Masukkan nama Hunter yang ingin direset: ");
    scanf("%s", name);
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (data->hunters[i].used && strcmp(data->hunters[i].username, name) == 0) {
            data->hunters[i].level = 1;
            data->hunters[i].exp = 0;
            data->hunters[i].atk = 10;
            data->hunters[i].hp = 100;
            data->hunters[i].def = 5;
            printf("[System Menu] Hunter %s berhasil direset\n", name);
            return;
        }
    }
    printf("[System Menu] Hunter tidak ditemukan\n");
}

int main() {
    signal(SIGINT, handle_sigint);
    key_t key = ftok(KEY_PATH, PROJ_ID);
    shmid = shmget(key, sizeof(SharedData), IPC_CREAT | 0666);
    data = (SharedData *)shmat(shmid, NULL, 0);
    memset(data, 0, sizeof(SharedData));
    srand(time(NULL));

    int choice;
    while (1) {
        printf("\n=== SYSTEM MENU ===\n");
        printf("1. Hunter Info\n");
        printf("2. Dungeon Info\n");
        printf("3. Generate Dungeon\n");
        printf("4. Ban Hunter\n");
        printf("5. Unban Hunter\n");
        printf("6. Reset Hunter\n");
        printf("7. Exit\n");
        printf("Choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: list_hunters(); break;
            case 2: list_dungeons_info(); break;
            case 3: generate_dungeon(); break;
            case 4: ban_hunter(true); break;
            case 5: ban_hunter(false); break;
            case 6: reset_hunter(); break;
            case 7: handle_sigint(0); break;
            default: printf("[System Menu] Pilihan tidak valid\n"); break;
        }
    }
    return 0;
}
