#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/stat.h>
#include <ctype.h>

void log_message(const char* source, const char* action, const char* info);
void reverse_string(char *str);
void hex_decode(const char* hex_str, unsigned char* output, int* output_len);
void decrypt_text(const char* input_file, char* output_filename);
void send_file(int client_socket, const char* filepath);
void handle_client(int client_socket);

int main() {
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Gagal membuat socket");
        return 1;
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345); // Set port server
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Gagal bind socket");
        return 1;
    }

    listen(server_socket, 5);
    printf("Server listening on port 12345...\n");

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            perror("Gagal menerima koneksi");
            continue;
        }

        if (fork() == 0) {
            close(server_socket);
            handle_client(client_socket);
            close(client_socket);
            exit(0);
        }
        close(client_socket);
    }
    return 0;
}

void log_message(const char* source, const char* action, const char* info) {
    FILE *log_file = fopen("server/server.log", "a");
    if (log_file != NULL) {
        time_t t;
        time(&t);
        struct tm *tm_info = localtime(&t);
        char time_str[26];
        strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(log_file, "[%s][%s]: [%s] [%s]\n", source, time_str, action, info);
        fclose(log_file);
    }
}


void reverse_string(char *str) {
    int n = strlen(str);
    for (int i = 0; i < n/2; i++) {
        char temp = str[i];
        str[i] = str[n-i-1];
        str[n-i-1] = temp;
    }
}

void hex_decode(const char* hex_str, unsigned char* output, int* output_len) {
    int len = strlen(hex_str);
    *output_len = 0;
    for (int i = 0; i < len; i += 2) {
        if (!isxdigit(hex_str[i]) || !isxdigit(hex_str[i+1])) break;
        sscanf(hex_str + i, "%2hhx", &output[*output_len]);
        (*output_len)++;
    }
}

void decrypt_text(const char* input_file, char* output_filename) {
    mkdir("server/database", 0755);

    FILE *fin = fopen(input_file, "r");
    if (!fin) {
        perror("gagal membuka file input");
        return;
    }

    fseek(fin, 0, SEEK_END);
    long fsize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    char *hex_content = malloc(fsize + 1);
    if (!hex_content) {
        perror("malloc gagal");
        fclose(fin);
        return;
    }

    fread(hex_content, 1, fsize, fin);
    fclose(fin);

    hex_content[fsize] = '\0'; // null-terminate!

    // Reverse
    reverse_string(hex_content);

    // Decode
    unsigned char decoded[32768];
    int decoded_len = 0;
    hex_decode(hex_content, decoded, &decoded_len);

    free(hex_content);

    // Simpan ke file
    time_t now = time(NULL);
    sprintf(output_filename, "server/database/%ld.jpeg", now);

    FILE *fout = fopen(output_filename, "wb");
    if (!fout) {
        perror("gagal membuka file output");
        return;
    }
    fwrite(decoded, 1, decoded_len, fout);
    fclose(fout);
}

void send_file(int client_socket, const char* filepath) {
    FILE *file = fopen(filepath, "rb");
    if (!file) {
        send(client_socket, "NOTFOUND", 8, 0);
        return;
    }

    send(client_socket, "FOUND", 5, 0);

    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    send(client_socket, &filesize, sizeof(filesize), 0);

    char buffer[1024];
    size_t n;
    while ((n = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        send(client_socket, buffer, n, 0);
    }

    fclose(file);
}

void handle_client(int client_socket) {
    char buffer[256];
    int n;
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        n = recv(client_socket, buffer, sizeof(buffer), 0);
        if (n <= 0) break;

        if (strncmp(buffer, "DECRYPT", 7) == 0) {
            char input_file[128];
            sscanf(buffer, "DECRYPT %s", input_file);

            char output_filename[256];
            decrypt_text(input_file, output_filename);

            char *basename = strrchr(output_filename, '/') + 1;

            log_message("Client", "DECRYPT", "Text data");
            log_message("Server", "SAVE", basename);

            send(client_socket, basename, strlen(basename), 0);

        } else if (strncmp(buffer, "DOWNLOAD", 8) == 0) {
            char filename[100];
            sscanf(buffer, "DOWNLOAD %s", filename);
        
            char path[256];
            snprintf(path, sizeof(path), "server/database/%s", filename);
        
            FILE *file = fopen(path, "rb");
            if (file) {
                // Kirim "FOUND" dulu ke client
                send(client_socket, "FOUND", 5, 0);
        
                fseek(file, 0, SEEK_END);
                long filesize = ftell(file);
                fseek(file, 0, SEEK_SET);
        
                // Kirim ukuran file
                send(client_socket, &filesize, sizeof(filesize), 0);
        
                // Kirim isi file
                char buffer[1024];
                long bytes_sent = 0;
                while (bytes_sent < filesize) {
                    int n = fread(buffer, 1, sizeof(buffer), file);
                    send(client_socket, buffer, n, 0);
                    bytes_sent += n;
                }
                fclose(file);

                log_message("Server", "UPLOAD", filename);
            } else {
                // Kalau file tidak ditemukan
                send(client_socket, "NOTFOUND", 8, 0);
            }

        } else if (strncmp(buffer, "EXIT", 4) == 0) {
            log_message("Client", "EXIT", "Client requested to exit");
            break;
        }
    }
}
