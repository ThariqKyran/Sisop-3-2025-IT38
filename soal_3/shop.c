#include "shop.h"

Weapon weapon_shop[WEAPON_COUNT] = {
    {0, "Stone Spear", 5, 15, 50, "", 0, 0},
    {1, "Diamond Sword", 15, 25, 150, "", 0, 0},
    {2, "Buster Sword", 20, 30, 200, "", 0, 0},
    {3, "Staff of Light", 10, 20, 120, "10% Insta-Kill", 10, 9999},
    {4, "Excalibur", 25, 35, 300, "30% Critical Hit", 30, 2}
};

void weapon_shop_menu(char *response) {
    strcpy(response, GREEN "\n=== WEAPON SHOP ===\n" RESET);
    for (int i = 0; i < WEAPON_COUNT; i++) {
        char line[256];
        if (strlen(weapon_shop[i].passive) > 0) {
            sprintf(line, "[%d] %s - %d gold (Dmg: %d-%d%s%s)\n",
                     i+1, weapon_shop[i].name, weapon_shop[i].price,
                     weapon_shop[i].min_damage, weapon_shop[i].max_damage,
                     weapon_shop[i].passive[0] ? ", " : "",
                     weapon_shop[i].passive);
        } else {
            sprintf(line, "[%d] %s - %d gold (Dmg: %d-%d%s%s)\n",
                     i+1, weapon_shop[i].name, weapon_shop[i].price,
                     weapon_shop[i].min_damage, weapon_shop[i].max_damage,
                     weapon_shop[i].passive[0] ? ", " : "",
                     weapon_shop[i].passive);
        }
        strcat(response, line);
    }
    strcat(response, YELLOW "Enter weapon number to buy (0 to cancel): " RESET);
}

void buy_weapon(PlayerStats *stats, int weapon_index, char *response) {
    if (weapon_index < 0 || weapon_index >= WEAPON_COUNT) {
        strcpy(response, RED "Invalid weapon selection.\n" RESET);
        return;
    }

    if (stats->inventory.count >= MAX_ITEMS) {
        strcpy(response, RED "Inventory full! Cannot buy more weapons.\n" RESET);
        return;
    }

    Weapon w = weapon_shop[weapon_index];

    if (stats->gold < w.price) {
        sprintf(response, RED "Not enough gold to buy %s! Need %d, have %d\n" RESET, 
               w.name, w.price, stats->gold);
        return;
    }
 
    stats->inventory.items[stats->inventory.count].id = w.id;
    stats->inventory.items[stats->inventory.count].min_damage = w.min_damage;
    stats->inventory.items[stats->inventory.count].max_damage = w.max_damage;
    stats->inventory.items[stats->inventory.count].price = w.price;
    stats->inventory.items[stats->inventory.count].passive_chance = w.passive_chance;
    stats->inventory.items[stats->inventory.count].passive_effect = w.passive_effect;
    strncpy(stats->inventory.items[stats->inventory.count].name, w.name, 64);
    strncpy(stats->inventory.items[stats->inventory.count].passive, w.passive, 100);

    stats->gold -= w.price;
    stats->inventory.count++;

    sprintf(response, GREEN "You bought %s!\n" RESET 
           "Remaining gold: %d\n"
           "Inventory slots used: %d/%d\n",
           w.name, stats->gold, stats->inventory.count, MAX_ITEMS);

    weapon_shop_menu(response + strlen(response));
}

void show_inventory(PlayerStats *stats, char *response) {
    strcpy(response, GREEN "\n=== INVENTORY ===\n" RESET);
    for (int i = 0; i < stats->inventory.count; i++) {
        Weapon *w = &stats->inventory.items[i];
        char equipped[32] = "";
        if (strcmp(w->name, stats->equipped_weapon.name) == 0) {
            snprintf(equipped, sizeof(equipped), "%s (EQUIPPED)%s", GREEN, RESET);
        }
        sprintf(response + strlen(response), "[%d] %s (Dmg: %d-%d)%s\n",
               i+1, w->name,  w->min_damage, w->max_damage, equipped);
    }
    strcat(response, YELLOW "Choose weapon to equip (0 to cancel): " RESET);
}

void equip_weapon(PlayerStats *stats, int item_index, char *response) {
    if (item_index < 0 || item_index >= stats->inventory.count) {
        strcpy(response, RED "Invalid selection! No weapon at that slot.\n" RESET);
        return;
    }

    stats->equipped_weapon.id = stats->inventory.items[item_index].id;
    stats->equipped_weapon.min_damage = stats->inventory.items[item_index].min_damage;
    stats->equipped_weapon.max_damage = stats->inventory.items[item_index].max_damage;
    stats->equipped_weapon.price = stats->inventory.items[item_index].price;
    stats->equipped_weapon.passive_chance = stats->inventory.items[item_index].passive_chance;
    stats->equipped_weapon.passive_effect = stats->inventory.items[item_index].passive_effect;
    strncpy(stats->equipped_weapon.name, stats->inventory.items[item_index].name, 64);
    strncpy(stats->equipped_weapon.passive, stats->inventory.items[item_index].passive, 100);

    sprintf(response, GREEN "Equipped %s!\n" RESET, stats->equipped_weapon.name);
    show_inventory(stats, response + strlen(response));
}

