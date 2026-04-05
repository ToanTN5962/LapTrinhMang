#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>

typedef struct {
    int num;
    int state;
    char hoten[64];
    char mssv[10];
} Client;
int nclients = 0;

void createEmail(char *hoten, char *mssv, char *email){
    char temp[64];
    strcpy(temp, hoten);

    char *tokens[10];
    int n = 0;

    char *p = strtok(temp, " ");
    while(p){
        tokens[n++] = p;
        p = strtok(NULL, " ");
    }

    char ten[20];
    strcpy(ten, tokens[n - 1]);

    char buf[10] = "";
    for(int i = 0; i < n - 1; i++){
        int len = strlen(buf);
        buf[len] = tokens[i][0];
        buf[len + 1] = 0;
    }

    char *tail = mssv + 2;

    sprintf(email, "%s.%s%s@sis.hust.edu.vn", ten, buf, tail);
    return;
}

int main(){
    Client clients[64];

    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    unsigned long ul = 1;
    ioctl(server, FIONBIO, &ul);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    bind(server, (struct sockaddr *) &addr, sizeof(addr));
    listen(server, 5);

    printf("Server is listening on port 8080...\n");

    char buf[256];
    int len;

    while(1){
        int client = accept(server, NULL, NULL);
        if(client != -1){
            printf("New client connected: %d\n", client);
            ioctl(client, FIONBIO, &ul);

            clients[nclients].num = client;
            clients[nclients].state = 0;
            send(client, "Nhap ho va ten: ", 16, 0);

            nclients++;
        }

        for(int i = 0; i < nclients; i++){
            len = recv(clients[i].num, buf, sizeof(buf) - 1, 0);

            if(len == -1){
                if(errno == EWOULDBLOCK){
                    continue;
                }
                else continue;
            }
            else if(len == 0){
                close(clients[i].num);
                clients[i] = clients[nclients - 1];
                nclients--;
                i--;
                continue;
            }

            buf[len] = '\0';
            buf[strcspn(buf, "\r\n")] = 0;

            if(clients[i].state == 0){
                strcpy(clients[i].hoten, buf);
                clients[i].state = 1;

                send(clients[i].num, "Nhap MSSV: ", 11, 0);
            }
            else if(clients[i].state == 1){
                strcpy(clients[i].mssv, buf);
                char email[64];
                createEmail(clients[i].hoten, clients[i].mssv, email);

                send(clients[i].num, "Email la: ", 10, 0);
                send(clients[i].num, email, strlen(email), 0);

                clients[i].state = 2;
            }
        }
    }

    return 0;
}