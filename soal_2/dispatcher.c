#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h>

#define MAX_ORDERS 100
#define SHM_KEY 1234

typedef struct {
    char nama[64];
    char alamat[128];
    char tipe[10];    // "Express" atau "Reguler"
    char status[20];  // "Pending" atau "Delivered"
    char agen[32];    // Nama agen pengantar
} Order;

// Fungsi untuk mencatat log pengiriman
void log_delivery(const char *agent, const char *nama, const char *alamat, const char *tipe) {
    FILE *log = fopen("delivery.log", "a");
    if (!log) {
        perror("fopen log");
        return;
    }

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    fprintf(log, "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT %s] %s package delivered to %s in %s\n",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
        t->tm_hour, t->tm_min, t->tm_sec,
        agent, tipe, nama, alamat);

    fclose(log);
}

// Fungsi untuk mengirim order bertipe Reguler
void deliver_reguler_order(const char *target_nama, const char *user_agent, Order *orders) {
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strcmp(orders[i].nama, target_nama) == 0 &&
            strcmp(orders[i].tipe, "Reguler") == 0 &&
            strcmp(orders[i].status, "Pending") == 0) {

            strcpy(orders[i].status, "Delivered");
            strcpy(orders[i].agen, user_agent);

            log_delivery(user_agent, orders[i].nama, orders[i].alamat, "Reguler");

            printf("Pesanan Reguler untuk %s telah dikirim oleh AGENT %s.\n", target_nama, user_agent);
            return;
        }
    }
    printf("Pesanan Reguler untuk %s tidak ditemukan atau sudah dikirim.\n", target_nama);
}

// Fungsi untuk mengecek status pesanan
void check_status(const char *nama, Order *orders) {
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strcmp(orders[i].nama, nama) == 0) {
            if (strcmp(orders[i].status, "Delivered") == 0) {
                printf("Status for %s: Delivered by %s\n", orders[i].nama, orders[i].agen);
            } else {
                printf("Status for %s: Pending\n", orders[i].nama);
            }
            return;
        }
    }
    printf("Pesanan dengan nama %s tidak ditemukan.\n", nama);
}

// Fungsi untuk menampilkan semua pesanan
void list_orders(Order *orders) {
    printf("Daftar semua pesanan:\n");
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strlen(orders[i].nama) > 0) {
            printf("%s - %s - %s\n", orders[i].nama, orders[i].status, orders[i].tipe);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [-deliver] [-status] [-list] [nama]\n", argv[0]);
        return 1;
    }

    // Attach shared memory
    int shm_id = shmget(SHM_KEY, sizeof(Order) * MAX_ORDERS, 0666);
    if (shm_id < 0) {
        perror("shmget");
        return 1;
    }
    Order *orders = (Order *)shmat(shm_id, NULL, 0);
    if (orders == (void *)-1) {
        perror("shmat");
        return 1;
    }

    if (strcmp(argv[1], "-deliver") == 0 && argc == 3) {
        // Mengirim pesanan Reguler
        char *nama_target = argv[2];
        char *user_agent = getenv("USER");  // Mengambil nama pengguna dari environment
        if (!user_agent) user_agent = "Unknown";

        deliver_reguler_order(nama_target, user_agent, orders);
    } else if (strcmp(argv[1], "-status") == 0 && argc == 3) {
        // Mengecek status pesanan
        char *nama_target = argv[2];
        check_status(nama_target, orders);
    } else if (strcmp(argv[1], "-list") == 0) {
        // Menampilkan semua pesanan
        list_orders(orders);
    } else {
        printf("Perintah tidak valid.\n");
    }

    shmdt(orders);
    return 0;
}
