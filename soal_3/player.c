// player.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT     12345
#define BUF_SIZE 512

// kirim cmd dan tampilkan response
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

        if (choice == 1) {
            send_cmd("SHOW_STATS");
        }
        else if (choice == 2) {
            send_cmd("SHOP_MENU");

            int idx;
            // printf("Choose weapon number to buy: ");
            scanf("%d", &idx);

            char cmd[32];
            snprintf(cmd, sizeof(cmd), "BUY:%d", idx);
            send_cmd(cmd);
        }
        else if (choice == 3) {
            send_cmd("VIEW_INV");

            printf("\033[33mEnter inventory index to equip: \033[0m");
            int idx;
            scanf("%d", &idx);
            char cmd[32];
            snprintf(cmd, sizeof(cmd), "EQUIP:%d", idx);
            send_cmd(cmd);
        }
        else if (choice == 4) {
            // Langsung trigger BATTLE:attack pertama untuk spawn musuh
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
        else if (choice == 5) {
            printf("\033[34mGoodbye!\033[0m\n");
            break;
        }
        else {
            printf("\033[31mInvalid choice.\033[0m\n");
        }
    }

    return 0;
}
