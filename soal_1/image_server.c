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

#define SERVER_DB_DIR "/home/zika/Documents/sisop/shift3/coba/server/database"

void log_message(const char* source, const char* action, const char* info);
void reverse_string(char *str);
void hex_decode(const char* hex_str, unsigned char* output, int* output_len);
void decrypt_text(const char* input_file, char* output_filename);
void handle_client(int client_socket);

int main() {
    pid_t pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    if (setsid() < 0) exit(EXIT_FAILURE);
    pid = fork();
    if (pid < 0) exit(EXIT_FAILURE);
    if (pid > 0) exit(EXIT_SUCCESS);

    // tetap di project root, jangan chdir ke /
    // Tutup fd standar
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        log_message("Daemon", "ERROR", "Gagal membuat socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        log_message("Daemon", "ERROR", "Gagal bind socket");
        exit(EXIT_FAILURE);
    }

    listen(server_socket, 5);
    log_message("Daemon", "INFO", "Server listening on port 12345");

    while (1) {
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket < 0) {
            log_message("Daemon", "ERROR", "Gagal menerima koneksi");
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
    if (log_file) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char ts[26];
        strftime(ts, sizeof(ts), "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(log_file, "[%s][%s]: [%s] [%s]\n", source, ts, action, info);
        fclose(log_file);
    }
}

void reverse_string(char *str) {
    int n = strlen(str);
    for (int i = 0; i < n/2; i++) {
        char tmp = str[i]; str[i] = str[n-i-1]; str[n-i-1] = tmp;
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
    // Buat direktori client/database jika belum ada
    mkdir(SERVER_DB_DIR, 0755);

    FILE *fin = fopen(input_file, "r");
    if (!fin) {
        log_message("Server", "ERROR", "Input file not found");
        return;
    }

    fseek(fin, 0, SEEK_END);
    long fsize = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    char *hex_content = malloc(fsize + 1);
    if (!hex_content) { fclose(fin); return; }
    fread(hex_content, 1, fsize, fin);
    fclose(fin);
    hex_content[fsize] = '\0';

    reverse_string(hex_content);

    unsigned char decoded[32768]; int decoded_len = 0;
    hex_decode(hex_content, decoded, &decoded_len);
    free(hex_content);

    time_t now = time(NULL);
    snprintf(output_filename, 256, SERVER_DB_DIR "/%ld.jpeg", now);

    FILE *fout = fopen(output_filename, "wb");
    if (!fout) {
        log_message("Server", "ERROR", "Failed opening output file");
        return;
    }
    fwrite(decoded, 1, decoded_len, fout);
    fclose(fout);
    // log_message("Server", "SAVE", output_filename);
}

void handle_client(int client_socket) {
    char buf[512];
    while (1) {
        memset(buf, 0, sizeof(buf));
        int n = recv(client_socket, buf, sizeof(buf)-1, 0);
        if (n <= 0) break;
        buf[n] = '\0';

        if (strncmp(buf, "DECRYPT", 7) == 0) {
            char inp[256], outfn[256];
            sscanf(buf, "DECRYPT %255s", inp);
            decrypt_text(inp, outfn);
            // kirim basename
            char *base = strrchr(outfn, '/') + 1;
            // revisi
            log_message("Client", "DECRYPT", "Text data");
            log_message("Server", "SAVE", base);
            send(client_socket, base, strlen(base), 0);

        } else if (strncmp(buf, "DOWNLOAD", 8) == 0) {
            char fn[128]; sscanf(buf, "DOWNLOAD %127s", fn);
            char path[512]; 

            snprintf(path, sizeof(path), "%s/%s", SERVER_DB_DIR, fn);
            FILE *f = fopen(path, "rb");
            if (!f) { send(client_socket, "NOTFOUND", 8, 0); }
            else {
                send(client_socket, "FOUND", 5, 0);
                fseek(f, 0, SEEK_END); long sz = ftell(f); fseek(f, 0, SEEK_SET);

                if (send(client_socket, &sz, sizeof(sz), 0) != sizeof(sz)) {
                    log_message("Server", "ERROR", "Failed to send file size");
                    fclose(f);
                    return;
                }
                
                // Kirim file
                char tmp[1024]; 
                size_t r;
                while (r = fread(tmp, 1, sizeof(tmp), f)) {
                    if (send(client_socket, tmp, r, 0) != r) {
                        log_message("Server", "ERROR", "Failed to send file chunk");
                        break;
                    }
                }
                fclose(f);
                log_message("Server", "UPLOAD", fn);
            }

        } else if (strncmp(buf, "EXIT", 4) == 0) {
            log_message("Server", "EXIT", "Client disconnected");
            break;
        }
    }
}
