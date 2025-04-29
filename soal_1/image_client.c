// image_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <time.h>

void download_and_unzip();
void run_command(char *const argv[]);
void send_decrypt_request(int sock, const char* input_file);
void request_download(int sock, const char* filename);
void log_message(const char* source, const char* action, const char* info);
void handle_menu(int sock);
void receive_file(int sock, const char* filename);

int main() {
    download_and_unzip(); // Download dan unzip secrets.zip kalau belum ada

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Gagal membuat socket");
        return 1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345); // Set port server
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Server IP

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Gagal connect ke server");
        return 1;
    }

    handle_menu(sock);

    close(sock);
    return 0;
}

void download_and_unzip() {
    struct stat st = {0};
    if (stat("client/secrets", &st) == 0 && S_ISDIR(st.st_mode)) {
        printf("Folder client/secrets sudah ada. Melewati download.\n");
        return;
    }

    printf("Mendownload dan mengekstrak secrets.zip...\n");

    // Download ke client/secrets.zip
    char *wget_args[] = {
        "wget", "-q", "-O", "client/secrets.zip",
        "https://drive.usercontent.google.com/u/0/uc?id=15mnXpYUimVP1F5Df7qd_Ahbjor3o1cVw&export=download",
        NULL
    };
    run_command(wget_args);

    // UNZIP dengan target direktori client/secrets/
    char *unzip_args[] = {"unzip", "-q", "client/secrets.zip", "-d", "client/", NULL};
    run_command(unzip_args);

    // Hapus secrets.zip setelah selesai
    unlink("client/secrets.zip");
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

void send_decrypt_request(int sock, const char* input_file) {
    char message[256];
    snprintf(message, sizeof(message), "DECRYPT %s", input_file);
    send(sock, message, strlen(message), 0);

    char buffer[256] = {0};
    recv(sock, buffer, sizeof(buffer), 0);

    printf("File berhasil didekripsi dan disimpan sebagai: %s\n", buffer);
}

void request_download(int sock, const char* filename) {
    char message[256];
    snprintf(message, sizeof(message), "DOWNLOAD %s", filename);
    send(sock, message, strlen(message), 0);

    receive_file(sock, filename);
}

void log_message(const char* source, const char* action, const char* info) {
    FILE *log_file = fopen("server/server.log", "a"); // log tetap di server/server.log
    if (log_file != NULL) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char time_str[26];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(log_file, "[%s][%s]: [%s] [%s]\n", source, time_str, action, info);
        fclose(log_file);
    }
}

void receive_file(int sock, const char* filename) {
    char status[10] = {0};
    if (recv(sock, status, sizeof(status), 0) <= 0) {
        perror("Gagal menerima status");
        return;
    }

    if (strcmp(status, "FOUND") != 0) {
        printf("File tidak ditemukan di server.\n");
        return;
    }

    // Log lebih awal, setelah dipastikan file ada
    log_message("Client", "DOWNLOAD", filename);

    long filesize = 0;
    if (recv(sock, &filesize, sizeof(filesize), 0) != sizeof(filesize)) {
        fprintf(stderr, "Gagal menerima ukuran file.\n");
        return;
    }

    char fullpath[256];
    snprintf(fullpath, sizeof(fullpath), "client/%s", filename);

    FILE *file = fopen(fullpath, "wb");
    if (!file) {
        perror("Gagal membuat file");
        return;
    }

    char buffer[1024];
    long received = 0;
    while (received < filesize) {
        int n = recv(sock, buffer, sizeof(buffer), 0);
        if (n <= 0) {
            perror("recv error atau koneksi terputus");
            break;
        }
        fwrite(buffer, 1, n, file);
        received += n;
    }

    fclose(file);

    if (received == filesize) {
        printf("File berhasil diunduh dan disimpan sebagai %s (%ld bytes)\n", fullpath, received);
    } else {
        printf("Download tidak lengkap: %ld dari %ld bytes\n", received, filesize);
    }
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
        scanf("%d", &choice);

        switch (choice) {
            case 1: {
                char input_file[100];
                printf("Masukkan nama file input (contoh: input_1.txt): ");
                scanf("%s", input_file);

                // Prefix path folder secrets/
                char full_path[256];
                snprintf(full_path, sizeof(full_path), "client/secrets/%s", input_file);

                send_decrypt_request(sock, full_path);
                break;
            }
            case 2: {
                char filename[100];
                printf("Masukkan nama file untuk didownload (contoh: 1744399397.jpeg): ");
                scanf("%s", filename);
                request_download(sock, filename);
                break;
            }
            case 3:
                printf("Keluar dari program...\n");
                send(sock, "EXIT", 4, 0);
                return;
            default:
                printf("Pilihan tidak valid. Silakan coba lagi.\n");
        }
    }
}
