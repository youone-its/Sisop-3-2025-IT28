#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>

#define MAX_HUNTERS   100
#define MAX_DUNGEONS  100

typedef struct {
    int   used;
    int   key;
    char  username[50];
    char  password[50];
    int   level;
    int   exp;
    int   atk;
    int   hp;
    int   def;
    int   banned;
} Hunter;

typedef struct {
    int used;
    int key;
    char name[50];
    int min_level;
    int atk_reward;
    int hp_reward;
    int def_reward;
    int exp_reward;
} Dungeon;

typedef struct {
    Hunter    hunters[MAX_HUNTERS];
} HuntersShared;

typedef struct {
    Dungeon   dungeons[MAX_DUNGEONS];
} DungeonsShared;

static int            shm_id_h, shm_id_d;
static HuntersShared *sh_h;
static DungeonsShared *sh_d;

void clear_screen(){ printf("\033[2J"); }

void cleanup(int signo) {
    shmdt(sh_h);
    shmdt(sh_d);
    shmctl(shm_id_h, IPC_RMID, NULL);
    shmctl(shm_id_d, IPC_RMID, NULL);
    printf("\n[system] Shared memory dihapus. Exiting.\n");
    exit(0);
}

void init_shared() {
    key_t key_h = ftok(".", 'H');
    key_t key_d = ftok(".", 'D');
    shm_id_h = shmget(key_h, sizeof(HuntersShared), IPC_CREAT | 0666);
    shm_id_d = shmget(key_d, sizeof(DungeonsShared), IPC_CREAT | 0666);
    if (shm_id_h < 0 || shm_id_d < 0) {
        perror("shmget");
        exit(1);
    }
    sh_h = shmat(shm_id_h, NULL, 0);
    sh_d = shmat(shm_id_d, NULL, 0);
    memset(sh_h, 0, sizeof(*sh_h));
    memset(sh_d, 0, sizeof(*sh_d));
}

void list_hunters() {
    printf("=== List Hunters ===\n");
    for (int i = 0; i < MAX_HUNTERS; i++) {
        Hunter *h = &sh_h->hunters[i];
        if (!h->used) continue;
        printf("Key=%d | %s | Lvl=%d, EXP=%d, ATK=%d, HP=%d, DEF=%d | %s\n",
            h->key, h->username, h->level, h->exp, h->atk, h->hp, h->def,
            h->banned ? "BANNED" : "OK");
    }
}

void list_dungeons() {
    printf("=== List Dungeons ===\n");
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        Dungeon *d = &sh_d->dungeons[i];
        if (!d->used) continue;
        printf("Key=%d | %s | MinLvl=%d | Rewards: EXP=%d, ATK=%d, HP=%d, DEF=%d\n",
            d->key, d->name, d->min_level,
            d->exp_reward, d->atk_reward, d->hp_reward, d->def_reward);
    }
}

void generate_dungeon() {
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        Dungeon *d = &sh_d->dungeons[i];
        if (d->used) continue;
        d->used = 1;
        d->key = i;
        snprintf(d->name, sizeof(d->name), "Dungeon_%d", d->key);
        d->min_level   = rand()%5 + 1;
        d->atk_reward  = rand()%51 + 100;
        d->hp_reward   = rand()%51 + 50;
        d->def_reward  = rand()%26 + 25;
        d->exp_reward  = rand()%151 + 150;
        printf("[system] Dungeon baru: Key=%d, MinLvl=%d, EXP=%d, ATK=%d, HP=%d, DEF=%d\n",
            d->key, d->min_level,
            d->exp_reward, d->atk_reward, d->hp_reward, d->def_reward);
        return;
    }
    printf("[system] Kapasitas dungeon penuh!\n");
}

Hunter* find_hunter_by_key(int key) {
    for (int i = 0; i < MAX_HUNTERS; i++) {
        if (sh_h->hunters[i].used && sh_h->hunters[i].key == key)
            return &sh_h->hunters[i];
    }
    return NULL;
}

void toggle_ban_hunter() {
    int key;
    printf("Masukkan Hunter key: ");
    scanf("%d", &key);
    Hunter *h = find_hunter_by_key(key);
    if (!h) {
        printf("Hunter tidak ditemukan.\n");
        return;
    }
    h->banned = !h->banned;
    printf("Hunter %d sekarang %s.\n",
        key, h->banned ? "BANNED" : "UNBANNED");
}

void reset_hunter() {
    int key;
    printf("Masukkan Hunter key: ");
    scanf("%d", &key);
    Hunter *h = find_hunter_by_key(key);
    if (!h) {
        printf("Hunter tidak ditemukan.\n");
        return;
    }
    h->level  = 1;
    h->exp    = 0;
    h->atk    = 10;
    h->hp     = 100;
    h->def    = 5;
    h->banned = 0;
    printf("Hunter %d stats dikembalikan ke awal dan unbanned.\n", key);
}

int main() {
    srand(time(NULL));
    signal(SIGINT, cleanup);
    init_shared();

    int cmd;
    clear_screen();
    while (1) {
        printf("\nPilihan:\n");
        printf("1) List Hunters\n");
        printf("2) List Dungeons\n");
        printf("3) Generate Dungeon\n");
        printf("4) Toggle Ban/Unban Hunter\n");
        printf("5) Reset Hunter Stats\n");
        printf("6) Exit\n");
        printf("Masukkan pilihan: ");
        scanf("%d", &cmd);
        switch (cmd) {
            case 1: clear_screen(); list_hunters();             break;
            case 2: clear_screen(); list_dungeons();            break;
            case 3: clear_screen(); generate_dungeon();         break;
            case 4: clear_screen(); list_hunters(); toggle_ban_hunter();        break;
            case 5: clear_screen(); list_hunters(); reset_hunter();             break;
            case 6: clear_screen(); cleanup(0);clear_screen();  break;
            default: clear_screen();printf("Pilihan tidak valid.\n");
        }
    }
    return 0;
}
