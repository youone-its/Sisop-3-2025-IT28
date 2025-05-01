// dungeon.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT        12345
#define BACKLOG     5
#define BUF_SIZE    512
#define MAX_WEAPONS 10

// deklarasi fungsi dari shop.c
char* shop_menu();
char* buy_weapon(int);
char* view_inventory();
char* equip_weapon(int);


typedef struct {
    char name[32];
    int damage, price;
    char passive[64];
} Weapon;

typedef struct {
    int gold, kills;
    Weapon inventory[MAX_WEAPONS];
    int inventory_count;
    Weapon equipped;
} Player;

Player player = { .gold = 500, .kills = 0, .inventory_count = 0 }; // player default
Weapon fists = { "Fists", 5, 0, "" };

// daftar shop
Weapon shop_items[] = {
    {"Sword", 10, 100, ""},
    {"Axe", 15, 150, "Critical Chance"},
    {"Dagger", 8,  80,  "Double Strike"},
    {"Hammer",20, 200, ""},
    {"Bow",   12, 120, "Bleed"}
};

int shop_count = sizeof(shop_items) / sizeof(shop_items[0]);
// show player stats
char* show_stats() {
    static char buf[256];
    Weapon eq = (player.inventory_count > 0 ? player.equipped : fists);

    snprintf(buf, sizeof(buf),
        "\033[36m==== PLAYER STATS ====\033[0m\n"
        "\033[33mGold: \033[0m%d | \033[32mEquipped Weapon: \033[0m%s | \033[31mBase Damage: \033[0m%d | \033[34mKills: \033[0m%d\n",
        player.gold,
        eq.name,
        eq.damage,
        player.kills);

    if (strlen(eq.passive) > 0) {
        strcat(buf, "\033[35mPassive: \033[0m");
        strcat(buf, eq.passive);
        strcat(buf, "\n");
    }

    return buf;
}

// Draw health bar [||||||||      ]
const char* draw_health_bar(int hp, int max_hp) {
    static char bar[40];
    int total = 30;
    int filled = (hp * total) / max_hp;
    for (int i = 0; i < total; i++) {
        bar[i] = (i < filled) ? '|' : ' ';
    }
    bar[total] = '\0';
    static char result[48];
    snprintf(result, sizeof(result), "[%s]", bar);
    return result;
}

// Battle mode
char* battle(const char* action) {
    static char buf[512];
    static int enemy_hp = 0, enemy_max = 0;
    Weapon eq = (player.inventory_count > 0 ? player.equipped : fists);
    buf[0] = '\0';

    // spawn enemy
    if (enemy_hp <= 0) {
        srand(time(NULL));
        enemy_max = (rand() % 151) + 50; // 50–200
        enemy_hp = enemy_max;
        snprintf(buf, sizeof(buf),
            "\n\033[31m=== BATTLE STARTED ===\033[0m\n"
            "Enemy appeared with:\n%s %d/%d HP \n"
            "Type '\033[32mattack\033[0m' to attack or '\033[31mexit\033[0m' to leave battle.\n",
            draw_health_bar(enemy_hp, enemy_max), enemy_hp, enemy_max);
        return buf;
    }

    if (strcmp(action, "attack") == 0) {
        int damage = eq.damage + (rand() % 6); // base + 0~5

        // Critical hit (20%)
        int is_crit = rand() % 100 < 20;
        if (is_crit) damage *= 2;

        // Passive: Insta-Kill (10%)
        int is_instant = strstr(eq.passive, "Insta-Kill") && (rand() % 100 < 10);
        // Passive: Crit Bonus (+30%) → tambahkan ke is_crit
        if (strstr(eq.passive, "Crit Chance") && (rand() % 100 < 30)) is_crit = 1;

        // Insta-kill effect
        if (is_instant) {
            snprintf(buf, sizeof(buf),
                "\033[35m=== INSTANT KILL! ===\033[0m\nYour %s unleashed a beam of pure energy!\nEnemy was instantly destroyed!\n",
                eq.name);
            enemy_hp = 0;
        } else {
            if (is_crit) strcat(buf, "\n\033[33m=== CRITICAL HIT! ===\033[0m\n");
            char dmgline[64];
            snprintf(dmgline, sizeof(dmgline), "You dealt \033[31m%d damage\033[0m!\n", damage);
            strcat(buf, dmgline);
            enemy_hp -= damage;
            if (enemy_hp < 0) enemy_hp = 0;
        }

        // Enemy still alive
        if (enemy_hp > 0) {
            strcat(buf, "\n\033[36m=== ENEMY STATUS ===\033[0m\n");
            char status[128];
            snprintf(status, sizeof(status), "Enemy health: %s %d/%d HP\n",
                draw_health_bar(enemy_hp, enemy_max), enemy_hp, enemy_max);
            strcat(buf, status);
        } else {
            // Enemy defeated
            int reward = (rand() % 101) + 50;  // 50–150 gold
            player.gold += reward;
            player.kills++;

            char reward_line[128];
            snprintf(reward_line, sizeof(reward_line),
                "\n\033[35m=== REWARD ===\033[0m\nYou earned \033[33m%d gold\033[0m!\n", reward);
            strcat(buf, reward_line);

            // New enemy
            enemy_max = (rand() % 151) + 50;
            enemy_hp = enemy_max;

            char spawn_line[128];
            snprintf(spawn_line, sizeof(spawn_line),
                "\n\033[36m=== NEW ENEMY ===\033[0m\nEnemy health: %s %d/%d HP\n",
                draw_health_bar(enemy_hp, enemy_max), enemy_hp, enemy_max);
            strcat(buf, spawn_line);
        }
        strcat(buf, "\nType '\033[32mattack\033[0m' to attack or '\033[31mexit\033[0m' to leave battle.\n");

    } else if (strcmp(action, "exit") == 0) {
        enemy_hp = 0;
        return "\033[33mExited battle mode.\033[0m\n";
    } else {
        return "\033[31mInvalid battle command.\033[0m\n";
    }

    return buf;
}


// handle incoming commands
void handle_command(int clientfd, const char* buf) {
    char* resp;
    if(strcmp(buf, "SHOW_STATS") == 0) {
        resp = show_stats();
    }
    else if(strcmp(buf, "SHOP_MENU") == 0) {
        resp = shop_menu();  // call to shop.c
    }
    else if(strncmp(buf, "BUY:", 4) == 0) {
        int idx = atoi(buf + 4);
        resp = buy_weapon(idx);  // call to shop.c
    }
    else if(strcmp(buf, "VIEW_INV") == 0) {
        resp = view_inventory();  // call to player inventory
    }
    else if(strncmp(buf, "EQUIP:", 6) == 0) {
        int idx = atoi(buf + 6);
        resp = equip_weapon(idx);  // equip weapon logic
    }
    else if(strncmp(buf, "BATTLE:", 7) == 0) {
        resp = battle(buf + 7);
    }
    else {
        resp = "Unknown command.\n";
    }
    write(clientfd, resp, strlen(resp));
}

char* view_inventory() {
    static char buf[1024];
    buf[0] = '\0';
    strcat(buf, "\033[36m=== YOUR INVENTORY ===\033[0m\n");

    Weapon eq = (player.inventory_count > 0 ? player.equipped : fists);
    strcat(buf, "[0] Fists");
    if (strcmp(eq.name, fists.name) == 0) strcat(buf, " \033[33m(EQUIPPED)\033[0m");
    strcat(buf, "\n");

    for (int i = 0; i < player.inventory_count; i++) {
        char line[256];
        snprintf(line, sizeof(line), "[%d] %s", i + 1, player.inventory[i].name);
        strcat(buf, line);

        if (strlen(player.inventory[i].passive) > 0) {
            snprintf(line, sizeof(line), " \033[35m(Passive: %s)\033[0m", player.inventory[i].passive);
            strcat(buf, line);
        }

        if (strcmp(eq.name, player.inventory[i].name) == 0) {
            strcat(buf, " \033[33m(EQUIPPED)\033[0m");
        }
        strcat(buf, "\n");
    }

    return buf;
}

char* equip_weapon(int idx) {
    static char buf[128];
    if (idx == 0) {
        player.equipped = fists;
        return "Equipped Fists.\n";
    }
    if (idx < 1 || idx > player.inventory_count) {
        return "Invalid inventory index.\n";
    }
    player.equipped = player.inventory[idx - 1];
    snprintf(buf, sizeof(buf), "Equipped %s.\n", player.equipped.name);
    return buf;
}


int main() {
    int sockfd, clientfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    addr.sin_family = AF_INET;
    addr.sin_port   = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(sockfd, BACKLOG) < 0) {
        perror("listen"); exit(1);
    }
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        clientfd = accept(sockfd, NULL, NULL);
        if (clientfd < 0) { perror("accept"); continue; }

        char buf[BUF_SIZE];
        int len = read(clientfd, buf, BUF_SIZE-1);
        if (len <= 0) { close(clientfd); continue; }
        buf[len] = '\0';

        // handle command
        handle_command(clientfd, buf);

        close(clientfd);
    }

    close(sockfd);
    return 0;
}
