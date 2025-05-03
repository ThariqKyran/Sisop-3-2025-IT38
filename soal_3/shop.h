#ifndef SHOP_H
#define SHOP_H

#define MAX_WEAPONS 5
#define MAX_NAME_LEN 50
#define MAX_PASSIVE_LEN 100

typedef struct {
    char name[MAX_NAME_LEN];
    int price;
    int damage;
    int hasPassive;
    char passive[MAX_PASSIVE_LEN];
} Weapon;

extern Weapon weaponList[MAX_WEAPONS];

void initShop();
void showWeapons();
int buyWeapon(int index, int* playerGold, Weapon* inventory, int* invSize);
void showInventory(Weapon* inventory, int invSize);
int equipWeapon(int index, Weapon* inventory, int invSize, Weapon* equipped);

#endif
