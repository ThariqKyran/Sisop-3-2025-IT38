// ========== shop.c ==========
#ifndef SHOP_H
#define SHOP_H

#include <stdio.h>
#include <string.h>

#define MAX_WEAPONS 5

typedef struct {
    char name[64];
    int price;
    int damage;
    char passive[128];
} Weapon;

Weapon weapon_list[MAX_WEAPONS];

void init_shop() {
    strcpy(weapon_list[0].name, "Terra Blade");
    weapon_list[0].price = 50;
    weapon_list[0].damage = 10;
    strcpy(weapon_list[0].passive, "");

    strcpy(weapon_list[1].name, "Flint & Steel");
    weapon_list[1].price = 150;
    weapon_list[1].damage = 25;
    strcpy(weapon_list[1].passive, "");

    strcpy(weapon_list[2].name, "Kitchen Knife");
    weapon_list[2].price = 200;
    weapon_list[2].damage = 35;
    strcpy(weapon_list[2].passive, "");

    strcpy(weapon_list[3].name, "Staff of Light");
    weapon_list[3].price = 120;
    weapon_list[3].damage = 20;
    strcpy(weapon_list[3].passive, "10%% Insta-Kill Chance");

    strcpy(weapon_list[4].name, "Dragon Claws");
    weapon_list[4].price = 300;
    weapon_list[4].damage = 50;
    strcpy(weapon_list[4].passive, "+30%% Crit Chance");
}

void list_weapons() {
    printf("=== WEAPON SHOP ===\n");
    for (int i = 0; i < MAX_WEAPONS; i++) {
        printf("[%d] %s - Price: %d gold, Damage: %d",
               i + 1, weapon_list[i].name, weapon_list[i].price, weapon_list[i].damage);
        if (strlen(weapon_list[i].passive) > 0) {
            printf(" (Passive: %s)", weapon_list[i].passive);
        }
        printf("\n");
    }
    printf("Enter weapon number to buy (0 to cancel): ");
}

Weapon get_weapon(int index) {
    if (index >= 0 && index < MAX_WEAPONS) return weapon_list[index];
    Weapon empty = {"", 0, 0, ""};
    return empty;
}

#endif