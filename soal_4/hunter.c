#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
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
    Hunter hunters[MAX_HUNTERS];
} HuntersShared;

typedef struct {
    Dungeon dungeons[MAX_DUNGEONS];
} DungeonsShared;

static HuntersShared *sh_h;
static DungeonsShared *sh_d;
static Hunter *me = NULL;
static int notify_mode = 0;

void clear_screen(){ printf("\033[2J"); }

void* notifier(void *arg) {
    while (me) {
        system("clear");
        printf("=== DUNGEON NOTIFICATIONS ===\n");
        for (int i = 0; i < MAX_DUNGEONS; i++) {
            Dungeon *d = &sh_d->dungeons[i];
            if (!d->used) continue;
            printf("Key=%d | %s | MinLvl=%d | EXP=%d, ATK=%d, HP=%d, DEF=%d\n",
                d->key=i, d->name, d->min_level,
                d->exp_reward, d->atk_reward, d->hp_reward, d->def_reward);
        }
        sleep(3);
    }
    return NULL;
}

void attach_shared() {
    key_t key_h = ftok(".", 'H');
    key_t key_d = ftok(".", 'D');
    int id_h = shmget(key_h, sizeof(HuntersShared), 0666);
    int id_d = shmget(key_d, sizeof(DungeonsShared), 0666);
    if (id_h < 0 || id_d < 0) {
        perror("shmget");
        exit(1);
    }
    sh_h = shmat(id_h, NULL, 0);
    sh_d = shmat(id_d, NULL, 0);
}

Hunter* find_hunter_by_username(const char *username) {
    for (int i = 0; i < MAX_HUNTERS; i++) {
        Hunter *h = &sh_h->hunters[i];
        if (h->used && strcmp(h->username, username) == 0) {
            return h;
        }
    }
    return NULL;
}

void do_register() {
    char uname[50], pwd[50];
    printf("=== REGISTER ===\n");
    printf("Masukkan username: ");
    scanf("%49s", uname);
    if (find_hunter_by_username(uname)) {
        printf("Username '%s' sudah digunakan, silakan pilih username lain.\n", uname);
        return;
    }
    printf("Masukkan password: ");
    scanf("%49s", pwd);
    for (int i = 0; i < MAX_HUNTERS; i++) {
        Hunter *h = &sh_h->hunters[i];
        if (!h->used) {
            h->used = 1;
            h->key  = i; 
            strncpy(h->username, uname, sizeof(h->username));
            strncpy(h->password, pwd, sizeof(h->password));
            h->level = 1;
            h->exp   = 0;
            h->atk   = 10;
            h->hp    = 100;
            h->def   = 5;
            h->banned = 0;
            printf("[hunter] Registrasi berhasil! Selamat datang, %s.\n", uname);
            return;
        }
    }
    printf("[hunter] Penuh, tidak bisa registrasi.\n");
}

void do_login() {
    char uname[50], pwd[50];
    printf("=== LOGIN ===\n");
    printf("Masukkan username: ");
    scanf("%49s", uname);
    printf("Masukkan password: ");
    scanf("%49s", pwd);

    Hunter *h = find_hunter_by_username(uname);
    if (!h || strcmp(h->password, pwd) != 0) {
        printf("Username atau password salah.\n");
        return;
    }
    if (h->banned) {
        printf("Akun Anda sedang banned!\n");
        return;
    }
    me = h;
    printf("[hunter] Login berhasil. Selamat datang kembali, %s!\n", me->username);

    if (notify_mode) {
        pthread_t tid;
        pthread_create(&tid, NULL, notifier, NULL);
        pthread_detach(tid);
    }
}

void list_dungeons_available() {
    printf("=== Dungeon untuk Lvl %d ===\n", me->level);

    int pilihan[MAX_DUNGEONS], cnt = 0;
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        Dungeon *d = &sh_d->dungeons[i];
        if (!d->used || me->level < d->min_level) 
            continue;
        pilihan[cnt] = i;
        printf("%2d) %s | MinLvl=%d | EXP=%d, ATK=%d, HP=%d, DEF=%d\n",
            cnt+1,
            d->name,
            d->min_level,
            d->exp_reward,
            d->atk_reward,
            d->hp_reward,
            d->def_reward
        );
        cnt++;
    }
    if (cnt == 0)
        printf("Tidak ada dungeon tersedia untuk level Anda.\n");
}

void raid_dungeon() {
    int pilihan[MAX_DUNGEONS], cnt = 0;
    for (int i = 0; i < MAX_DUNGEONS; i++) {
        Dungeon *d = &sh_d->dungeons[i];
        if (!d->used || me->level < d->min_level) 
            continue;
        pilihan[cnt++] = i;
    }

    if (cnt == 0) {
        printf("Tidak ada dungeon untuk di-raid.\n");
        return;
    }

    printf("=== Pilih Dungeon untuk Raid ===\n");
    for (int j = 0; j < cnt; j++) {
        Dungeon *d = &sh_d->dungeons[pilihan[j]];
        printf("%2d) %s | MinLvl=%d | EXP=%d, ATK=%d, HP=%d, DEF=%d\n",
            j+1,
            d->name,
            d->min_level,
            d->exp_reward,
            d->atk_reward,
            d->hp_reward,
            d->def_reward
        );
    }

    printf("Masukkan pilihan (1-%d): ", cnt);
    int opt; 
    scanf("%d", &opt);
    if (opt < 1 || opt > cnt) {
        printf("Pilihan tidak valid.\n");
        return;
    }

    Dungeon *d = &sh_d->dungeons[pilihan[opt - 1]];
    me->atk += d->atk_reward;
    me->hp  += d->hp_reward;
    me->def += d->def_reward;
    me->exp += d->exp_reward;
    printf("Raid sukses! +EXP %d, +ATK %d, +HP %d, +DEF %d\n",
        d->exp_reward, d->atk_reward, d->hp_reward, d->def_reward);
    d->used = 0;
    
    if (me->exp >= 500) {
        me->level++;
        me->exp -= 500;
        printf("Level UP! Sekarang Lvl %d\n", me->level);
    }
}

void list_hunters_other() {
    printf("=== Hunters Lain ===\n");
    for (int i = 0; i < MAX_HUNTERS; i++) {
        Hunter *h = &sh_h->hunters[i];
        if (!h->used || h == me) continue;
        printf("Username= %s | Power= %d | %s\n",
            h->username,
            h->atk + h->hp + h->def,
            h->banned ? "BANNED" : "OK");
    }
}

void battle_hunter() {
    char opponent[50];
    printf("Masukkan username lawan: ");
    scanf("%49s", opponent);

    Hunter *op = find_hunter_by_username(opponent);
    if (!op || op == me) {
        printf("Hunter tidak ditemukan atau Anda memilih diri sendiri.\n");
        return;
    }
    if (op->banned) {
        printf("Lawan sedang banned.\n");
        return;
    }

    int me_power = me->atk + me->hp + me->def;
    int op_power = op->atk + op->hp + op->def;
    if (me_power >= op_power) {
        me->atk += op->atk;
        me->hp  += op->hp;
        me->def += op->def;
        printf("Menang! Mendapat semua stats lawan.\n");
        op->used = 0;
    } else {
        op->atk += me->atk;
        op->hp  += me->hp;
        op->def += me->def;
        printf("Kalah! Stats Anda diberikan ke lawan.\n");
        me->used = 0;
        me = NULL;
    }
}

int main(int argc, char *argv[]) {
    clear_screen();
    srand(time(NULL));
    if (argc == 2 && strcmp(argv[1], "-n") == 0) notify_mode = 1;
    attach_shared();

    int choice;
    while (1) {
        if (!me) {
            printf("\n=== HUNTER CLIENT ===\n");
            printf("1) Register\n");
            printf("2) Login\n");
            printf("3) Exit\n");
            printf("Pilih opsi: ");
            scanf("%d", &choice);
            switch (choice) {
                case 1: clear_screen(); do_register(); break;
                case 2: clear_screen(); do_login();    break;
                case 3:  printf("Bye!\n"); exit(0);
                default: printf("Opsi tidak valid.\n");
            }
        } else {
            printf("\n=== MENU HUNTER (%s) ===\n", me->username);
            printf("1) List Dungeon\n");
            printf("2) Raid Dungeon\n");
            printf("3) List Hunters\n");
            printf("4) Battle Hunter\n");
            printf("5) Logout\n");
            printf("6) Exit\n");
            printf("Pilih opsi: ");
            scanf("%d", &choice);
            switch (choice) {
                case 1: clear_screen(); list_dungeons_available(); break;
                case 2: clear_screen(); raid_dungeon();             break;
                case 3: clear_screen(); list_hunters_other();       break;
                case 4: clear_screen(); list_hunters_other(); battle_hunter();            break;
                case 5: clear_screen(); me = NULL; printf("Logout sukses.\n"); break;
                case 6: printf("Bye!\n"); exit(0);
                default: clear_screen(); printf("Opsi tidak valid.\n");
            }
        }
    }

    return 0;
}
