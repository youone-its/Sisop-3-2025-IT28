#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
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

Order *orders;
int shm_id;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Fungsi untuk menulis log pengiriman
void write_log(const char *agent, const char *nama, const char *alamat) {
    FILE *f = fopen("delivery.log", "a");
    if (f) {
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        fprintf(f, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] Express package delivered to %s in %s\n",
            t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
            t->tm_hour, t->tm_min, t->tm_sec,
            agent, nama, alamat);
        fclose(f);
    }
}

// Fungsi untuk mengunduh file CSV menggunakan system call
void download_csv() {
    printf("Downloading CSV file...\n");
    system("wget -O delivery_order.csv 'https://drive.usercontent.google.com/u/0/uc?id=1OJfRuLgsBnIBWtdRXbRsD2sG6NhMKOg9&export=download'");
}

// Fungsi untuk memuat data dari CSV ke shared memory
void load_orders_from_csv(const char *csv_filename) {
    FILE *f = fopen(csv_filename, "r");
    if (!f) {
        perror("fopen");
        return;
    }

    int idx = 0;
    char line[256];
    
    fgets(line, sizeof(line), f); // Skip header

    while (fgets(line, sizeof(line), f)) {
        if (idx >= MAX_ORDERS) break;
        char *nama = strtok(line, ",");
        char *alamat = strtok(NULL, ",");
        char *tipe = strtok(NULL, "\n");

        if (nama && alamat && tipe) {
            strcpy(orders[idx].nama, nama);
            strcpy(orders[idx].alamat, alamat);
            strcpy(orders[idx].tipe, tipe);
            strcpy(orders[idx].status, "Pending");
            strcpy(orders[idx].agen, "-");
            idx++;
        }
    }

    fclose(f);
    printf("Loaded %d orders into shared memory.\n", idx);
}

// Fungsi untuk memeriksa apakah semua pesanan Express telah dikirim
int all_delivered() {
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strcmp(orders[i].tipe, "Express") == 0 &&
            strcmp(orders[i].status, "Pending") == 0) {
            return 0; // Masih ada yang belum dikirim
        }
    }
    return 1; // Semua terkirim
}

// Fungsi untuk thread agen pengiriman
void *agent_thread(void *arg) {
    char *agent_name = (char *)arg;

    while (1) {
        int found = 0;

        pthread_mutex_lock(&lock);
        for (int i = 0; i < MAX_ORDERS; i++) {
            if (strcmp(orders[i].tipe, "Express") == 0 &&
                strcmp(orders[i].status, "Pending") == 0) {

                strcpy(orders[i].status, "Delivered");
                strcpy(orders[i].agen, agent_name);
                write_log(agent_name, orders[i].nama, orders[i].alamat);
                found = 1;
                break;
            }
        }
        pthread_mutex_unlock(&lock);

        if (!found) {
            if (all_delivered()) break;
        }

        sleep(1);
    }

    return NULL;
}

int main() {
    download_csv();

    // Membuat shared memory
    shm_id = shmget(SHM_KEY, sizeof(Order) * MAX_ORDERS, 0666 | IPC_CREAT);
    if (shm_id < 0) {
        perror("shmget");
        return 1;
    }

    // Attach shared memory
    orders = (Order *)shmat(shm_id, NULL, 0);
    if (orders == (void *)-1) {
        perror("shmat");
        return 1;
    }

    load_orders_from_csv("delivery_order.csv");

    // Membuat thread agen pengiriman
    pthread_t agents[3];
    pthread_create(&agents[0], NULL, agent_thread, "AGENT A");
    pthread_create(&agents[1], NULL, agent_thread, "AGENT B");
    pthread_create(&agents[2], NULL, agent_thread, "AGENT C");

    // Menunggu semua thread agen selesai
    for (int i = 0; i < 3; i++)
        pthread_join(agents[i], NULL);

    shmdt(orders);
    // shmctl(shm_id, IPC_RMID, NULL); // Hapus shared memory setelah selesai

    printf("All deliveries completed. Exiting...\n");
    return 0;
}
