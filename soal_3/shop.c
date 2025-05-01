// shop.c
#include <stdio.h>
#include <string.h>

#define MAX_WEAPONS 10

typedef struct {
    char name[32];
    int damage;
    int price;
    char passive[64];
} Weapon;

typedef struct {
    int gold, kills;
    Weapon inventory[MAX_WEAPONS];
    int inventory_count;
    Weapon equipped;
} Player;

// deklarasi variabel dari dungeon.c
extern Player player;
extern Weapon shop_items[];
extern int shop_count;

char* shop_menu() {
    static char buf[1024];
    buf[0] = '\0';

    strcat(buf, "\033[36m===== WEAPON SHOP =====\033[0m\n");
    for (int i = 0; i < shop_count; i++) {
        Weapon *w = &shop_items[i];
        char line[256];

        snprintf(line, sizeof(line),
            "\033[32m[%d] %s\033[0m - Price: \033[33m%d gold\033[0m, Damage: \033[31m%d\033[0m",
            i + 1, w->name, w->price, w->damage);
        strcat(buf, line);

        if (strlen(w->passive) > 0) {
            snprintf(line, sizeof(line), " \033[35m(Passive: %s)\033[0m", w->passive);
            strcat(buf, line);
        }
        strcat(buf, "\n");
    }

    strcat(buf, "\n\033[33mEnter weapon number to buy (0 to cancel):\033[0m  ");
    return buf;
}

char* buy_weapon(int idx) {
    static char buf[128];

    if (idx == 0) return "\033[31mPurchase canceled.\033[0m\n";
    idx--; // karena menu 1-based
    if (idx < 0 || idx >= shop_count) return "\033[31mInvalid weapon number.\033[0m\n";

    Weapon *w = &shop_items[idx];
    if (player.gold < w->price) return "\033[33mNot enough gold.\033[0m\n";
    if (player.inventory_count >= MAX_WEAPONS) return "\033[33mInventory full.\033[0m\n";

    player.gold -= w->price;
    player.inventory[player.inventory_count++] = *w;
    snprintf(buf, sizeof(buf), "\033[0mBought \033[32m%s\033[0m for \033[33m%d\033[0m gold!\n", w->name, w->price);
    return buf;
}
