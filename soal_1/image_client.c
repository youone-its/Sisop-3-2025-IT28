#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

#define CLIENT_DIR "client"
#define SERVER_DB_DIR "server/database"
#define MAX_MSG_SIZE (PATH_MAX + 32)
#define MAX_FILENAME 256

void download_and_unzip();
void run_command(char *const argv[]);
void send_decrypt_request(int sock);
void request_download(int sock);
void log_message(const char* source, const char* action, const char* info);
void handle_menu(int sock);
void receive_file(int sock, const char* filename);

// Buffer untuk nama file yang sudah didekripsi
static char last_filename[PATH_MAX] = {0};
#define MAX_MESSAGE_SIZE (PATH_MAX + 32)

int main() {
    download_and_unzip();

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Gagal membuat socket");
        return 1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Gagal connect ke server");
        close(sock);
        return 1;
    }

    handle_menu(sock);

    close(sock);
    return 0;
}

void download_and_unzip() {
    struct stat st = {0};
    char secrets_dir[PATH_MAX];
    snprintf(secrets_dir, sizeof(secrets_dir), "%s/secrets", CLIENT_DIR);
    
    if (stat(secrets_dir, &st) == 0 && S_ISDIR(st.st_mode)) {
        printf("Folder %s sudah ada. Melewati download.\n", secrets_dir);
        return;
    }

    printf("Mendownload dan mengekstrak secrets.zip...\n");
    
    // Pastikan direktori client ada
    if (stat(CLIENT_DIR, &st) == -1) {
        mkdir(CLIENT_DIR, 0755);
    }

    char zip_path[PATH_MAX];
    snprintf(zip_path, sizeof(zip_path), "%s/secrets.zip", CLIENT_DIR);
    
    char *wget_args[] = {
        "wget", "-q", "-O", zip_path,
        "https://drive.usercontent.google.com/u/0/uc?id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw&export=download", 
        NULL
    };
    run_command(wget_args);
    
    char *unzip_args[] = {"unzip", "-q", zip_path, "-d", CLIENT_DIR, NULL};
    run_command(unzip_args);
    unlink(zip_path);
}

void run_command(char *const argv[]) {
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        perror("exec gagal");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        perror("fork gagal");
    }
}

void send_decrypt_request(int sock) {
    char input_file[100];
    printf("Masukkan nama file input (contoh: input_1.txt): ");
    if (scanf("%99s", input_file) != 1) return;
    getchar();  // buang newline

    char full_path[PATH_MAX];
    snprintf(full_path, sizeof(full_path), "%s/secrets/%s", CLIENT_DIR, input_file);

    if (access(full_path, F_OK) != 0) {
        perror("File tidak ditemukan");
        return;
    }

    size_t msg_size = strlen("DECRYPT ") + strlen(full_path) + 1;
    char *message = malloc(msg_size);
    snprintf(message, msg_size, "DECRYPT %s", full_path);
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Gagal kirim request");
        return;
    }

    char buffer[PATH_MAX];
    int n = recv(sock, buffer, sizeof(buffer)-1, 0);
    if (n <= 0) {
        printf("Gagal menerima nama file dari server\n");
        return;
    }
    buffer[n] = '\0';
    strncpy(last_filename, buffer, sizeof(last_filename)-1);
    printf("File didekripsi dengan nama: %s (belum di-download)\n", last_filename);
}

void request_download(int sock) {
    char filename[PATH_MAX];
    
    printf("\nPilihan download:\n");
    printf("1. Download file terakhir didekripsi (%s)\n", last_filename[0] ? last_filename : "Tidak ada");
    printf("2. Download file spesifik\n");
    printf("Pilihan (1/2): ");
    
    char choice;
    scanf(" %c", &choice);
    getchar(); // Membersihkan newline
    
    if (choice == '1') {
        if (last_filename[0] == '\0') {
            printf("Belum ada file terakhir untuk didownload\n");
            return;
        }
        strncpy(filename, last_filename, sizeof(filename));
    } 
    else if (choice == '2') {
        printf("Masukkan nama file untuk didownload: ");
        if (fgets(filename, sizeof(filename), stdin) == NULL) {
            printf("Input tidak valid\n");
            return;
        }
        filename[strcspn(filename, "\n")] = '\0'; // Hapus newline
    }
    else {
        printf("Pilihan tidak valid\n");
        return;
    }

    // Validasi nama file
    if (filename[0] == '\0') {
        printf("Nama file tidak boleh kosong\n");
        return;
    }

    char message[MAX_MSG_SIZE];
    snprintf(message, sizeof(message), "DOWNLOAD %s", filename);
    
    if (send(sock, message, strlen(message), 0) < 0) {
        perror("Gagal kirim request download");
        return;
    }
    
    receive_file(sock, filename);
}

void receive_file(int sock, const char* filename) {
    char status[16];
    int n = recv(sock, status, sizeof(status)-1, 0);
    if (n <= 0) { 
        perror("Gagal menerima status"); 
        return; 
    }
    status[n] = '\0';

    if (strcmp(status, "FOUND") != 0) {
        printf("File tidak ditemukan di server.\n");
        return;
    }
    log_message("Client", "DOWNLOAD", filename);

    long filesize;
    if (recv(sock, &filesize, sizeof(filesize), 0) != sizeof(filesize)) {
        fprintf(stderr, "Gagal menerima ukuran file.\n");
        return;
    }

    // Pastikan direktori client ada
    struct stat st = {0};
    if (stat(CLIENT_DIR, &st) == -1) {
        if (mkdir(CLIENT_DIR, 0755) == -1) {
            perror("Gagal membuat direktori client");
            return;
        }
    }

    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", CLIENT_DIR, filename);
    
    FILE *file = fopen(fullpath, "wb");
    if (!file) {
        perror("Gagal membuat file");
        printf("Path: %s\n", fullpath);
        return;
    }

    char buf[1024];
    long received = 0;
    while (received < filesize) {
        n = recv(sock, buf, sizeof(buf), 0);
        if (n <= 0) { 
            perror("Gagal menerima data"); 
            break; 
        }
        size_t written = fwrite(buf, 1, n, file);
        if (written != n) {
            perror("Gagal menulis file");
            break;
        }
        received += n;
    }
    fclose(file);

    if (received == filesize) {
        printf("File berhasil diunduh dan disimpan di %s (%ld bytes)\n", fullpath, received);
    } else {
        printf("Download tidak lengkap: %ld dari %ld bytes\n", received, filesize);
        unlink(fullpath);
    }
}

void log_message(const char* source, const char* action, const char* info) {
    FILE *log_file = fopen("server/server.log", "a");
    if (!log_file) return;
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char ts[26];
    strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm_info);
    fprintf(log_file, "[%s][%s]: [%s] [%s]\n", source, ts, action, info);
    fclose(log_file);
}

void handle_menu(int sock) {
    int choice;
    while (1) {
        printf("==============================\n");
        printf("|   Image Decoder Client     |\n");
        printf("==============================\n");
        printf("1. Send input file to server\n");
        printf("2. Download file from server\n");
        printf("3. Exit\n");
        printf(">> ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }
        getchar(); // buang newline

        switch (choice) {
            case 1: 
                send_decrypt_request(sock); 
                break;
            case 2: 
                request_download(sock); 
                break;
            case 3:
                printf("Keluar dari program...\n");
                send(sock, "EXIT", 4, 0);
                return;
            default: 
                printf("Pilihan tidak valid.\n");
        }
        printf("\n");
    }
}

