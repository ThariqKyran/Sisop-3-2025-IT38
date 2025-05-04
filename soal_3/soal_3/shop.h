#ifndef SHOP_H
#define SHOP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WEAPON_COUNT 5
#define MAX_ITEMS 10

#define RED     "\033[1;31m"
#define GREEN   "\033[0;32m"
#define YELLOW  "\033[1;33m"
#define RESET   "\033[0m"

typedef struct {
    int id;
    char name[64];
    int damage;
    int price;
    char passive[100];
    int passive_chance;
    int passive_effect;
} Weapon;

typedef struct {
    Weapon items[MAX_ITEMS];
    int count;
} Inventory;

typedef struct {
    int gold;
    Inventory inventory;
    Weapon equipped_weapon;
    int base_damage;
} PlayerStats;


extern Weapon weapon_shop[WEAPON_COUNT];

void weapon_shop_menu(char *response);
void buy_weapon(PlayerStats *stats, int weapon_index, char *response);
void show_inventory(PlayerStats *stats, char *response);
void equip_weapon(PlayerStats *stats, int item_index, char *response);

#endif

