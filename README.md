# Sisop-3-2025-IT28
# Laporan Praktikum IT28 Modul 3
## Anggota Kelompok
| No |             Nama              |     NRP     |
|----|-------------------------------|-------------|
| 1  | Yuan Banny Albyan             | 5027241027  |
| 2  | Ica Zika Hamizah              | 5027241058  |
| 3  | Nafis Faqih Allmuzaky Maolidi | 5027241095  |

## Soal_1
### Oleh: Ica Zika Hamizah
#### image_client.c
Program ini dirancang untuk berkomunikasi dengan server melalui koneksi TCP socket. Program ini dapat mengunduh file ZIP, mengekstraknya, mengirimkan permintaan dekripsi file, dan menerima file yang telah didekripsi dari server.

##### Fungsi-fungsi
- **`download_and_unzip()`**: Mendownload dan mengekstrak file ZIP jika diperlukan.
- **`run_command(char *const argv[])`**: Menjalankan perintah eksternal seperti `wget` atau `unzip`.
- **`send_decrypt_request(int sock, const char* input_file)`**: Mengirimkan permintaan dekripsi file ke server.
- **`request_download(int sock, const char* filename)`**: Mengirimkan permintaan untuk mengunduh file dari server.
- **`log_message(const char* source, const char* action, const char* info)`**: Mencatat log aktivitas.
- **`handle_menu(int sock)`**: Menampilkan menu interaktif untuk pengguna.
- **`receive_file(int sock, const char* filename)`**: Menerima file dari server setelah permintaan unduhan.

##### Header files
- **`#include <limits.h>`**: Menyediakan konstanta batasan integer.
- **`#include <stdio.h>`**: Menyediakan fungsi input/output standar.
- **`#include <stdlib.h>`**: Menyediakan fungsi umum seperti `exit()` dan `malloc()`.
- **`#include <string.h>`**: Menyediakan fungsi manipulasi string.
- **`#include <sys/stat.h>`**: Menyediakan fungsi untuk memanipulasi status file.
- **`#include <unistd.h>`**: Menyediakan fungsi-fungsi sistem operasi Unix.
- **`#include <sys/types.h>`**: Menyediakan tipe data untuk pemrograman sistem.
- **`#include <sys/wait.h>`**: Menyediakan fungsi untuk menunggu status proses anak.
- **`#include <sys/socket.h>`**: Menyediakan fungsi untuk operasi socket.
- **`#include <netinet/in.h>`**: Menyediakan struktur dan konstanta untuk alamat IPv4.
- **`#include <arpa/inet.h>`**: Menyediakan fungsi untuk pengalamatan IP.
- **`#include <ctype.h>`**: Menyediakan fungsi manipulasi karakter.
- **`#include <time.h>`**: Menyediakan fungsi untuk menangani waktu dan tanggal.

##### `int main()`
Fungsi utama dalam program ini yang berfungsi untuk:
1. Membuat socket TCP dengan menggunakan `socket()`.
2. Mengonfigurasi alamat server menggunakan struktur `sockaddr_in`.
3. Mencoba terhubung ke server melalui socket dengan `connect()`.
4. Menampilkan menu interaktif kepada pengguna menggunakan `handle_menu()`.
5. Menutup socket setelah proses selesai dengan `close()`.

##### `void download_and_unzip()`
Fungsi ini melakukan pengecekan apakah folder `client/secrets` sudah ada. Jika belum, fungsi ini akan mendownload file ZIP dari URL yang ditentukan dan mengekstraknya. Setelah ekstraksi selesai, file ZIP akan dihapus.

##### `void run_command(char *const argv[])`
Fungsi ini menjalankan perintah eksternal (misalnya `wget` atau `unzip`) dengan menggunakan `fork()` untuk membuat proses baru, kemudian mengeksekusi perintah tersebut dengan `execvp()`.

##### `void send_decrypt_request(int sock, const char* input_file)`
Fungsi ini mengirimkan permintaan dekripsi file ke server menggunakan socket yang terhubung. Fungsi ini juga menunggu balasan dari server mengenai status dekripsi.

##### `void request_download(int sock, const char* filename)`
Fungsi ini mengirimkan permintaan kepada server untuk mengunduh file tertentu. Setelah permintaan dikirim, fungsi ini akan menerima file dari server.

##### `void log_message(const char* source, const char* action, const char* info)`
Fungsi ini digunakan untuk mencatat aktivitas program dalam file log. Log akan menyimpan informasi mengenai sumber aktivitas, aksi yang dilakukan, dan informasi tambahan terkait.

##### `void receive_file(int sock, const char* filename)`
Fungsi ini menerima file yang dikirim oleh server setelah permintaan unduhan diajukan. Fungsi ini pertama-tama memeriksa status file, kemudian menerima file dalam potongan-potongan kecil.

##### `void handle_menu(int sock)`
Fungsi ini menampilkan menu interaktif kepada pengguna untuk memilih opsi yang tersedia, seperti mengirim file untuk didekripsi atau mengunduh file dari server.

#### image_server.c
Program ini adalah server gambar sederhana yang berjalan sebagai daemon. Server ini mendengarkan permintaan dari klien dan menangani dua fungsi utama:
1. **DECRYPT**: Server mendekripsi file gambar yang terenkripsi berbasis teks dan menyimpannya sebagai file gambar.
2. **DOWNLOAD**: Server mengirimkan file gambar yang telah didekripsi ke klien.

Server menggunakan socket TCP untuk komunikasi dan mencatat aktivitasnya dalam log.

##### Fitur
- **Daemonisasi**: Server berjalan sebagai proses latar belakang (daemon) untuk operasi yang terus-menerus.
- **Komunikasi Socket TCP**: Server berkomunikasi dengan klien melalui socket TCP yang mendengarkan di port `12345`.
- **Dekripsi**: Server mendukung dekripsi data gambar yang terkodekan dalam format hexadecimal terbalik dan menyimpannya sebagai gambar.
- **Upload/Download File**: Server memungkinkan klien untuk mengunduh file gambar yang telah didekripsi.
- **Pencatatan Log**: Server mencatat semua tindakan seperti permintaan dekripsi, unggahan, dan kesalahan dalam file log `server.log`.

##### Fungsi-fungsi
##### 1. `reap_zombies()`

- Menangani proses zombie (anak proses yang telah selesai menjalankan tugasnya) untuk membersihkan setelah proses anak.

##### 2. `log_message(const char* source, const char* action, const char* info)`

- Mencatat pesan ke dalam file log yang terletak di direktori server (`server.log`).
- Mencatat tindakan yang dilakukan, termasuk stempel waktu.

##### 3. `reverse_string(char *str)`

- Membalikkan string yang diberikan di tempat.

##### 4. `hex_decode(const char* hex_str, unsigned char* output, int* output_len)`

- Mendekodekan string hexadecimal menjadi data biner.

##### 5. `decrypt_text(const char* input_file, char* output_filename)`

- Membuka dan membaca file terenkripsi.
- Mendekodekannya dari hexadecimal dan membalikkan stringnya.
- Menyimpan data yang terdekripsi sebagai file gambar baru di direktori database (`database/`).

##### 6. `handle_client(int client_socket)`

- Menangani permintaan dari klien, termasuk dekripsi dan pengunduhan file.
- Merespons permintaan seperti `DECRYPT`, `DOWNLOAD`, dan `EXIT`.

## Soal_2
### Oleh: Nafis Faqih Allmuzaky Maolidi
### Dispatcher.c
Program ini adalah client agent dalam sistem pengiriman: Menyambung ke shared memory. Memungkinkan agen: Mengantar pesanan, Mengecek status, Melihat semua pesanan. Mencatat log pengiriman ke delivery.log.

#### Library dan Macro
```
#include <stdio.h>     
#include <stdlib.h>    
#include <string.h>    
#include <sys/shm.h>   
#include <sys/ipc.h>   
#include <unistd.h>    
#include <time.h>      

#define MAX_ORDERS 100     
#define SHM_KEY 1234    

```
Program ini adalah persiapan awal agar program bisa menjalankan fitur utama seperti menyimpan data order, berbagi data antar proses (shared memory), mencatat waktu, dan menampilkan informasi ke layar.

### Struktur Data Order
```
typedef struct {
    char nama[64];      
    char alamat[128];    
    char tipe[10];       
    char status[20];     
    char agen[32];       
} Order;

```
Struktur ini menyimpan informasi satu pesanan pengiriman.
-`nama`: untuk Nama penerima pesanan
-`alamat`: untuk Alamat pengiriman
-`tipe`: untuk meninjau Tipe pengiriman Express atau Reguler
-`status`: untuk meninjau Status Pending atau Delivered
-`agen`: untuk Nama agen yang mengantar

### Fungsi log_delivery
```
void log_delivery(const char *agent, const char *nama, const char *alamat, const char *tipe) {
```
Untuk Mencatat log pengiriman ke file delivery.log.

```
    FILE *log = fopen("delivery.log", "a");
    if (!log) {
        perror("fopen log");
        return;
    }
```
Untuk Membuka file dalam mode append (a). Jika gagal, tampilkan error.

```
    time_t now = time(NULL);              
    struct tm *t = localtime(&now);       
```
-`time_t now`: Waktu saat ini
-`struct tm *t`: untukMengonversi ke format waktu lokal

```
    fprintf(log, "[%02d/%02d/%04d %02d:%02d:%02d] [AGENT %s] %s package delivered to %s in %s\n",
        t->tm_mday, t->tm_mon + 1, t->tm_year + 1900,
        t->tm_hour, t->tm_min, t->tm_sec,
        agent, tipe, nama, alamat);

```
Menulis data ke file log dalam format waktu [dd/mm/yyyy hh:mm:ss].

```
    fclose(log); 
}

```
Untuk Menutup file log

### Fungsi deliver_reguler_order
```
void deliver_reguler_order(const char *target_nama, const char *user_agent, Order *orders)

```
Untuk Mencari pesanan dengan nama tertentu dan tipe Reguler, lalu menandainya sebagai "Delivered".

```
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strcmp(orders[i].nama, target_nama) == 0 &&
            strcmp(orders[i].tipe, "Reguler") == 0 &&
            strcmp(orders[i].status, "Pending") == 0) {

```
Untuk Mengecek apakah nama cocok, tipe Reguler, dan status Pending.

```
            strcpy(orders[i].status, "Delivered");
            strcpy(orders[i].agen, user_agent);

```
Update status dan nama agen pengantar.

```
            log_delivery(user_agent, orders[i].nama, orders[i].alamat, "Reguler");
            printf("Pesanan Reguler untuk %s telah dikirim oleh AGENT %s.\n", target_nama, user_agent);
            return;

```
Panggil fungsi log_delivery dan tampilkan pesan ke pengguna.

```
    printf("Pesanan Reguler untuk %s tidak ditemukan atau sudah dikirim.\n", target_nama);

```
Cek jika tidak ditemukan.

### Fungsi check_status
```
void check_status(const char *nama, Order *orders)

```
Mengecek status pengiriman berdasarkan nama.

```
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strcmp(orders[i].nama, nama) == 0) {

```
Cek Jika nama cocok

```
            if (strcmp(orders[i].status, "Delivered") == 0) {
                printf("Status for %s: Delivered by %s\n", orders[i].nama, orders[i].agen);
            } else {
                printf("Status for %s: Pending\n", orders[i].nama);
            }
            return;

```
Untuk menampilkan status pengiriman.

```
    printf("Pesanan dengan nama %s tidak ditemukan.\n", nama);

```
Cek Jika tidak ada.

### Fungsi list_orders
```
void list_orders(Order *orders)

```
Menampilkan semua pesanan yang ada.

```
    for (int i = 0; i < MAX_ORDERS; i++) {
        if (strlen(orders[i].nama) > 0) {
            printf("%s - %s - %s\n", orders[i].nama, orders[i].status, orders[i].tipe);
        }
    }

```

### Fungsi main
```
int main(int argc, char *argv[])

```
Fungsi utama program. Menangani perintah deliver, status, dan list.

```
    if (argc < 2) {
        printf("Usage: %s [-deliver] [-status] [-list] [nama]\n", argv[0]);
        return 1;
    }

```
Validasi argumen minimal.

```
    int shm_id = shmget(SHM_KEY, sizeof(Order) * MAX_ORDERS, 0666);

```
Mengakses shared memory berdasarkan SHM_KEY.

```
    if (shm_id < 0) {
        perror("shmget");
        return 1;
    }

```
Jika gagal, tampilkan error.

```
    Order *orders = (Order *)shmat(shm_id, NULL, 0);

```
Attach ke shared memory dan meng-cast ke pointer Order.

```
    if (orders == (void *)-1) {
        perror("shmat");
        return 1;
    }

```

### Eksekusi Perintah
```
    if (strcmp(argv[1], "-deliver") == 0 && argc == 3)

```
Untuk Menangani deliver.

```
        char *nama_target = argv[2];
        char *user_agent = getenv("USER");
        if (!user_agent) user_agent = "Unknown";
        deliver_reguler_order(nama_target, user_agent, orders);

```

```
    else if (strcmp(argv[1], "-status") == 0 && argc == 3)
        check_status(argv[2], orders);

```

```
    else if (strcmp(argv[1], "-list") == 0)
        list_orders(orders);

```

```
    else
        printf("Perintah tidak valid.\n");

```

```c
    shmdt(orders);  
    return 0;
}

```
untuk Melepas pointer dari shared memory

### delivery_agent.c


## Soal_3
### Oleh: Ica Zika Hamizah
### Dungeon.c (server)
Program ini adalah server untuk game berbasis teks yang menangani berbagai perintah seperti melihat statistik pemain, membeli senjata, melihat inventaris, serta menjalankan pertempuran dengan musuh.

#### Inklusi Library
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

```
Kode ini menyertakan berbagai pustaka standar yang digunakan untuk operasi input/output, manipulasi string, soket jaringan, serta fungsi terkait waktu.

#### Definisi Makro
```
#define PORT        12345
#define BACKLOG     5
#define BUF_SIZE    512
#define MAX_WEAPONS 10
```

- `PORT`: Port yang digunakan oleh server untuk mendengarkan koneksi.

- `BACKLOG`: Jumlah maksimal koneksi yang dapat menunggu di antrean.

- `BUF_SIZE`: Ukuran buffer untuk membaca data dari client.

- `MAX_WEAPONS`: Jumlah maksimal senjata yang dapat dimiliki oleh pemain.

#### Struktur Data
```
typedef struct {
    char name[32];
    int damage, price;
    char passive[64];
} Weapon;
```
Struktur Weapon untuk menyimpan data senjata:

- `name`: Nama senjata.

- `damage`: Damage senjata.

- `price`: Harga senjata.

- `passive`: Efek pasif senjata, seperti "Critical Chance" atau "Insta-Kill".

```
typedef struct {
    int gold, kills;
    Weapon inventory[MAX_WEAPONS];
    int inventory_count;
    Weapon equipped;
} Player;
```

Struktur Player untuk menyimpan data pemain:

- `gold`: Jumlah emas yang dimiliki pemain.

- `kills`: Jumlah musuh yang dibunuh pemain.

- `inventory`: Daftar senjata yang dimiliki pemain.

- `inventory_count`: Jumlah senjata dalam inventaris.

- `equipped`: Senjata yang sedang dipakai pemain.

#### Inisialisasi Data Pemain dan Senjata
```
Player player = { .gold = 500, .kills = 0, .inventory_count = 0 }; // pemain default
Weapon fists = { "Fists", 5, 0, "" }; // senjata default (tangan kosong)
```

Pemain dimulai dengan 500 emas, 0 pembunuhan, dan tidak ada senjata dalam inventaris. Senjata default adalah Fists dengan damage 5.

#### Dafar Senjata di Toko
```
Weapon shop_items[] = {
    {"Sword", 10, 100, ""},
    {"Axe", 15, 150, "Critical Chance"},
    {"Dagger", 8,  80,  "Double Strike"},
    {"Hammer",20, 200, ""},
    {"Bow",   12, 120, "Bleed"}
};
```

Daftar senjata yang tersedia di toko dengan atribut: nama, damage, harga, dan efek pasif.

#### Fungsi `show_stats()`
```
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
```

Fungsi ini menampilkan statistik pemain, termasuk jumlah emas, senjata yang dipakai, damage dasar, jumlah pembunuhan, dan efek pasif senjata.

#### Fungsi `draw_health_bar()`
```
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
```

Fungsi ini menggambar health bar untuk menampilkan status HP musuh dalam bentuk visual.

#### Mode Pertempuran (`battle()`)
```
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
```

Mode pertempuran dimulai dengan musuh yang muncul dan memerlukan aksi dari pemain. Pemain dapat menyerang atau keluar dari pertempuran.

Fungsi ini juga menangani serangan dengan efek seperti critical hit, insta-kill, dan bonus pasif lainnya, serta memberikan hadiah emas setelah musuh dikalahkan.

#### Fungsi `handle_command()`
```
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
```

Fungsi ini menangani perintah yang diterima dari client dan merespons sesuai dengan perintah yang diberikan, seperti `SHOW_STATS`, `SHOP_MENU`, `BUY`, `VIEW_INV`, `EQUIP`, dan `BATTLE`.

#### Fungsi `view_inventory()` dan `equip_weapon()`
```
char* view_inventory() {
    static char buf[1024];
    buf[0] = '\0';
    strcat(buf, "\033[36m=== YOUR INVENTORY ===\033[0m\n");
    ...
    return buf;
}

char* equip_weapon(int idx) {
    static char buf[128];
    if (idx == 0) {
        player.equipped = fists;
        return "Equipped Fists.\n";
    }
    ...
    return buf;
}
```

- v`iew_inventory()`: Menampilkan inventaris pemain.

- `equip_weapon()`: Memilih senjata untuk dipakai berdasarkan indeks yang diberikan.

#### Fungsi `main()`
```
int main() {
    int sockfd, clientfd;
    struct sockaddr_in addr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    ...
    while (1) {
        clientfd = accept(sockfd, NULL, NULL);
        if (clientfd < 0) { perror("accept"); continue; }
        char buf[BUF_SIZE];
        int len = read(clientfd, buf, BUF_SIZE-1);
        if (len <= 0) { close(clientfd); continue; }
        buf[len] = '\0';
        handle_command(clientfd, buf);
        close(clientfd);
    }
    close(sockfd);
    return 0;
}
```
Fungsi utama yang membuat server mendengarkan koneksi pada port yang telah ditentukan. Ketika koneksi diterima, server membaca perintah dari client dan memprosesnya menggunakan `handle_command()`.

### Player.c 
File player.c ini memungkinkan pemain untuk berinteraksi dengan server permainan melalui soket TCP. Pemain dapat memilih untuk melihat statistik, membeli senjata, melihat inventaris dan memilih senjata yang akan dipakai, serta berpartisipasi dalam pertempuran. Semua perintah dikirimkan ke server, dan hasilnya ditampilkan pada antarmuka pemain.

#### Include dan Definisi Macro
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT     12345
#define BUF_SIZE 512
```

- `#include <stdio.h>`: Menyertakan library untuk input/output standar.

- `#include <stdlib.h>`: Menyertakan library untuk fungsi utilitas umum.

- `#include <string.h>`: Menyertakan library untuk manipulasi string.

- `#include <unistd.h>`: Menyertakan library untuk fungsi sistem operasi Unix, seperti `read` dan `write`.

- `#include <sys/socket.h>`: Menyertakan library untuk pemrograman soket.

- `#include <netinet/in.h>`: Menyertakan definisi untuk struktur alamat jaringan.

- `#include <arpa/inet.h>`: Menyertakan fungsi untuk konversi alamat IP.

```
#define PORT     12345
#define BUF_SIZE 512
```

- `PORT`: Mendefinisikan nomor port untuk komunikasi soket.

- `BUF_SIZE`: Mendefinisikan ukuran buffer untuk membaca data dari soket.

#### Fungsi `send_cmd()`
```
void send_cmd(const char *cmd) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port   = htons(PORT),
        .sin_addr.s_addr = inet_addr("127.0.0.1")
    };
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return;
    }
    write(sock, cmd, strlen(cmd));
    char buf[BUF_SIZE];
    int n = read(sock, buf, BUF_SIZE-1);
    if (n > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }
    close(sock);
}
```
- Membuka soket TCP dengan `socket()` dan menghubungkan ke server yang berjalan di `127.0.0.1` pada port yang telah ditentukan (`PORT`).

- Mengirimkan perintah `cmd` ke server dengan `write()`.

- Membaca respons dari server menggunakan `read()` dan menampilkan hasilnya ke layar.

- Menutup soket setelah komunikasi selesai dengan `close()`.

#### Fungsi `main()`
Fungsi utama untuk menangani menu utama dan memilih opsi yang tersedia. Pemain dapat memilih berbagai tindakan melalui antarmuka menu berbasis teks.
```
int main() {
    int choice;
    while (1) {
        printf("\n\033[36m===== MAIN MENU =====\033[0m\n");
        printf("1. Show Player Stats\n");
        printf("2. Shop (Buy Weapons)\n");
        printf("3. View Inventory & Equip Weapons\n");
        printf("4. Battle Mode\n");
        printf("5. Exit Game\n");
        printf("\033[33mChoose an option: \033[0m");
        if (scanf("%d", &choice) != 1) break;
```

- Menampilkan menu utama yang memungkinkan pemain memilih berbagai opsi: menampilkan statistik pemain, berbelanja, melihat inventaris, berpartisipasi dalam mode pertempuran, atau keluar dari permainan.

- `scanf("%d", &choice)` untuk mendapatkan input dari pemain.

#### Pilihan Menu
##### 1. Shom Player Stats
Mengirim perintah `SHOW_STATS` ke server untuk menampilkan statistik pemain.
```
if (choice == 1) {
    send_cmd("SHOW_STATS");
}
```

##### 2. Shop (Buy Weapons)
- Mengirim perintah `SHOP_MENU` untuk menampilkan menu toko senjata.

- Meminta pemain untuk memilih senjata yang ingin dibeli, lalu mengirim perintah `BUY:<weapon_idx> `untuk membeli senjata tersebut.

```
else if (choice == 2) {
    send_cmd("SHOP_MENU");
    int idx;
    scanf("%d", &idx);
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "BUY:%d", idx);
    send_cmd(cmd);
}
```

##### 3. View Inventory & Equip Weapons
- Mengirim perintah `VIEW_INV` untuk menampilkan inventaris senjata.

- Meminta pemain untuk memilih senjata dari inventaris dan mengirim perintah `EQUIP:<weapon_idx> `untuk memilih senjata yang akan dipakai.

```
else if (choice == 3) {
    send_cmd("VIEW_INV");
    printf("\033[33mEnter inventory index to equip: \033[0m");
    int idx;
    scanf("%d", &idx);
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "EQUIP:%d", idx);
    send_cmd(cmd);
}
```

##### 4. Battle Mode
- Mengirim perintah `BATTLE:attack` untuk memulai pertempuran dengan musuh.

- Menangani perintah selanjutnya yang dikirim oleh pemain selama pertempuran, seperti `attack`, `defend`, atau `exit` untuk keluar dari mode pertempuran.

```
else if (choice == 4) {
    send_cmd("BATTLE:attack");
    while (1) {
        printf("\n\033[36m> \033[0m");
        char act[16];
        scanf("%s", act);
        char cmd[32];
        snprintf(cmd, sizeof(cmd), "BATTLE:%s", act);
        send_cmd(cmd);
        if (strcmp(act, "exit") == 0) break;
    }
}

```

##### 5. Exit Game
Jika pemain memilih untuk keluar, program akan menampilkan pesan perpisahan dan keluar dari loop.

```
else if (choice == 5) {
    printf("\033[34mGoodbye!\033[0m\n");
    break;
}
```

##### 6. Input Tidak Valid
Jika pemain memilih opsi yang tidak valid, program akan menampilkan pesan kesalahan.
```
else {
    printf("\033[31mInvalid choice.\033[0m\n");
}
```

#### Mengakhiri Program
Program utama akan terus berjalan dalam loop sampai pemain memilih untuk keluar dengan memilih opsi 5.

### Shop.c

File `shop.c` ini mengelola toko senjata dalam permainan dengan memungkinkan pemain untuk melihat senjata yang tersedia dan membelinya menggunakan emas yang mereka miliki. Pemain dapat membeli senjata jika mereka memiliki cukup emas dan jika inventaris mereka belum penuh.

#### Include dan Definisi Macro
```
#include <stdio.h>
#include <string.h>
```
- `#include <stdio.h>`: Menyertakan library untuk input/output standar.

- `#include <string.h>`: Menyertakan library untuk manipulasi string.

`#define MAX_WEAPONS 10`
- Mendefinisikan konstanta `MAX_WEAPONS` yang menunjukkan jumlah maksimal senjata yang dapat dimiliki pemain dalam inventaris.

#### Struktur `Weapon`
```
typedef struct {
    char name[32];
    int damage;
    int price;
    char passive[64];
} Weapon;

```
`Weapon` digunakan untuk menyimpan informasi tentang senjata.

- `name`: Nama senjata (maksimum 32 karakter).

- `damage`: Nilai damage dari senjata.

- `price`: Harga senjata dalam emas.

- `passive`: Efek pasif yang dimiliki senjata (maksimum 64 karakter).

#### Struktur `Player`
```
typedef struct {
    int gold, kills;
    Weapon inventory[MAX_WEAPONS];
    int inventory_count;
    Weapon equipped;
} Player;
```
`Player` digunakan untuk menyimpan data pemain.

- `gold`: Jumlah emas yang dimiliki pemain.

- `kills`: Jumlah musuh yang telah dibunuh pemain.

- `inventory`: Array yang menyimpan senjata yang dimiliki pemain.

- `inventory_count`: Jumlah senjata yang ada di inventaris pemain.

- `equipped`: Senjata yang sedang digunakan oleh pemain.

#### Deklarasi Variabel Eksternal

```
extern Player player;
extern Weapon shop_items[];
extern int shop_count;
```
Mengakses variabel yang dideklarasikan di file lain `(dungeon.c)`, yaitu:

- `player`: Data pemain.

- `shop_items[]`: Array senjata yang tersedia di toko.

- `shop_count`: Jumlah senjata yang tersedia di toko.

#### Fungsi `shop_menu()`
Fungsi ini menampilkan menu toko senjata yang tersedia dan meminta input pemain untuk memilih senjata yang ingin dibeli.

```
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
```
- Membuat string menu toko dan menampilkan daftar senjata yang tersedia.

- Menambahkan informasi harga, damage, dan efek pasif senjata ke dalam menu.

- Meminta input pemain untuk memilih senjata dengan memberikan prompt.

#### Fungsi `buy_weapon(int idx)`
Fungsi ini menangani pembelian senjata oleh pemain.
```
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
```
- Memeriksa apakah pemain memilih untuk membatalkan pembelian.

- Memeriksa apakah indeks senjata yang dipilih valid.

- Memeriksa apakah pemain memiliki cukup emas untuk membeli senjata.

- Memeriksa apakah inventaris pemain sudah penuh.

- Mengurangi jumlah emas pemain dan menambahkan senjata ke inventaris pemain.

- Mengembalikan pesan sukses pembelian senjata.

## Soal_4
### Oleh: Yuan Banny Albyan
---

## Fitur Utama

1. **Shared Memory IPC**

   * `system.c` membuat dua segmen shared memory:

     * `HuntersShared` untuk data hunter (max 100).
     * `DungeonsShared` untuk data dungeon (max 100).
2. **Manajemen Dungeon**

   * Generate dungeon acak dengan rewards EXP/ATK/HP/DEF.
   * Notification mode (`-n`) untuk hunter agar tampilan dungeon terus ter-update.
3. **Hunter Client**

   * **Register & Login** dengan username/password.
   * **List** dungeon yang sesuai level.
   * **Raid**: ambil rewards, hapus dungeon, dan level up otomatis tiap 500 EXP.
   * **Battle** hunter lain: compare power (ATK+HP+DEF), pemenang “mencuri” stat, yang kalah ter–logout.
4. **Admin Console (system.c)**

   * List semua hunters & dungeons.
   * Generate dungeon baru on-demand.
   * Toggle Ban/Unban hunter & Reset stats.

---

## Persyaratan & Instalasi

* Compiler C (gcc)
* Unix-like OS (Linux/macOS) yang support `ftok`, `shmget`, `shmat`, `pthread`, `signal`

```bash
# Clone repo atau tarball
git clone https://…/dungeon-hunter.git
cd dungeon-hunter

# Kompilasi
gcc -o system system.c
gcc -o hunter hunter.c -lpthread
```

---

## Urutan Menjalankan

1. **Jalankan `system` terlebih dulu** agar shared memory ter-init:

   ```bash
   ./system
   ```
2. **Jalankan satu atau beberapa client** hunter di terminal berbeda:

   ```bash
   ./hunter           # tanpa notifikasi
   ./hunter -n        # mode notifikasi (clear screen setiap 3 detik)
   ```

> 💡 *Tip:* Anda bisa buka banyak jendela terminal untuk simulasi banyak hunter sekaligus.

---

## Penjelasan Menu & Perintah

### system.c (Admin)

```
1) List Hunters
2) List Dungeons
3) Generate Dungeon
4) Toggle Ban/Unban Hunter
5) Reset Hunter Stats
6) Exit (CTRL+C juga cleanup otomatis)
```

* **Generate Dungeon**: buat “Dungeon\_N” dengan level & reward acak.
* **Toggle Ban/Unban**: blokir hunter bandel.
* **Reset**: kembalikan hunter ke level 1 & unban.

### hunter.c (Client)

#### Saat Belum Login

```
1) Register
2) Login
3) Exit
```

* **Register**: pilih username unik, password.
* **Login**: masuk jika tidak banned.

#### Setelah Login

```
1) List Dungeon       – dungeon sesuai levelmu
2) Raid Dungeon       – lakukan raid & dapat reward
3) List Hunters       – lihat hunter lain + status banned
4) Battle Hunter      – tantang hunter lain
5) Logout
6) Exit
```

---

## Penjelasan Kode

### Struct Definitions

```c
// Data per hunter
typedef struct {
    int   used;           // Apakah slot ini terpakai
    int   key;            // ID unik hunter
    char  username[50];   // Username hunter
    char  password[50];   // Password hunter
    int   level;          // Level saat ini
    int   exp;            // Experience points
    int   atk;            // Attack stat
    int   hp;             // Health points
    int   def;            // Defense stat
    int   banned;         // Flag banned (1 = banned)
} Hunter;

// Data per dungeon
typedef struct {
    int used;             // Apakah dungeon tersedia
    int key;              // ID unik dungeon
    char name[50];        // Nama dungeon ("Dungeon_n")
    int min_level;        // Min level hunter untuk masuk
    int atk_reward;       // Reward attack
    int hp_reward;        // Reward HP
    int def_reward;       // Reward defense
    int exp_reward;       // Reward EXP
} Dungeon;

// Shared memory layout
typedef struct { Hunter    hunters[MAX_HUNTERS]; } HuntersShared;
typedef struct { Dungeon   dungeons[MAX_DUNGEONS]; } DungeonsShared;
```

### Fungsi Penting di `system.c`

```c
void init_shared();
```

*Menginisialisasi shared memory:*

* `shmget` untuk alokasi
* `shmat` untuk attach
* `memset` untuk bersih-bersih

```c
void generate_dungeon();
```

*Generate satu dungeon baru secara acak:*

* Cari slot kosong
* Set `used`, `key`, randomize `min_level`, rewards
* Cetak info dungeon

```c
void toggle_ban_hunter();       // Ban/Unban hunter
void reset_hunter();            // Reset stats hunter ke default
```

```c
void cleanup(int signo);
```

\_Handler `SIGINT` (CTRL+C): detach + hapus IPC + exit.

### Fungsi Penting di `hunter.c`

```c
void attach_shared();
```

*Attach ke segmen yang dibuat `system.c`.*

```c
void do_register();            // Register hunter baru
void do_login();               // Login & optional notifier thread
```

```c
void list_dungeons_available(); // Tampilkan dungeon sesuai level
void raid_dungeon();            // Jalankan raid: dapat rewards & hapus dungeon
```

```c
void list_hunters_other();     // Lihat hunter lain + status
void battle_hunter();          // Battle dua hunter: transfer stats
```

```c
void* notifier(void *arg);     // Thread notifikasi dungeon setiap 3 detik
```

*Jika `notify_mode == 1`, thread ini clear screen dan print semua dungeon.*

---

## Arsitektur Shared Memory

```
+-------------------+       +-------------------+
|   system.c        |       |   hunter.c        |
|                   |<----->|                   |
| HuntersShared     |       | HuntersShared     |
| ┌─ hunters[100]─┐ |       | ┌─ hunters[100]─┐ |
| └───────────────┘ |       | └───────────────┘ |
| DungeonsShared    |       | DungeonsShared    |
| ┌─ dungeons[100]┐ |       | ┌─ dungeons[100]┐ |
| └───────────────┘ |       | └───────────────┘ |
+-------------------+       +-------------------+
```

---

## Kontribusi & Lisensi

* **Silakan fork & PR** untuk perbaikan bug, fitur baru, atau dokumentasi.
* Dilisensi di bawah MIT License — bebas gunakan & modifikasi.

---

*Selamat bertualang di dunia Dungeon Hunter! Semoga monster-nya tidak nge-lag…* 😎
