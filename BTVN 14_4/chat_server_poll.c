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
#include <poll.h>
#include <netdb.h>
#include <time.h>

int main(){
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if(bind(server, (struct sockaddr *) &addr, sizeof(addr))){
        perror("bind() failed");
        close(server);
        return 0;
    }

    if(listen(server, 5)){
        perror("listen() failed");
        close(server);
        return 0;
    }

    printf("Server is listening on port 9000...\n");

    struct pollfd fds[64];
    char buf[2048];
    int nfds = 1;

    fds[0].fd = server;
    fds[0].events = POLLIN;

    char clients_name[FD_SETSIZE][100];

    while(1){
        int ret = poll(fds, nfds, -1);
        if(ret < 0){
            printf("poll() failed");
            return 0;
        }
        if(ret == 0){
            printf("Timed out.\n");
            continue;
        }

        if(fds[0].revents & POLLIN){
            int client = accept(server, NULL, NULL);
            if(client < 64){
                printf("New client connected %d\n", client);
                char mes[128] = "Please enter your name in the form client_id: your_name\n";
                send(client, mes, strlen(mes), 0);
                char cmd[32], id[32], temp[32];
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
            }
            else{
                close(client);
            }
        }

        for(int i = 1; i < nfds; i++){
            if(fds[i].revents & POLLIN){
                int ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                buf[strcspn(buf, "\r\n")] = 0;

                if(strlen(clients_name[i]) == 0){
                    char cmd[32], id[32], temp[32];
                    int n = sscanf(buf, "%s%s%s", cmd, id, temp);

                    if(n != 2){
                        send(fds[i].fd, "Syntax error!\n", strlen("Syntax error!\n"), 0);
                        char mes[128] = "Please enter your name in the form client_id: your_name\n";
                        send(fds[i].fd, mes, strlen(mes), 0);
                        continue;
                    }

                    if(!strcmp(cmd, "client_id:")){
                        strcpy(clients_name[i], id);
                        continue;
                    }
                    else{
                        send(fds[i].fd, "Syntax error!\n", strlen("Syntax error!\n"), 0);
                        char mes[128] = "Please enter your name in the form client_id: your_name\n";
                        send(fds[i].fd, mes, strlen(mes), 0);
                        continue;
                    }
                }
                
                time_t now = time(NULL);
                struct tm *t = localtime(&now);
                char time_str[32];
                strftime(time_str, sizeof(time_str), "%Y:%m:%d %H:%M:%S", t);

                if(ret <= 0){
                    printf("Client %d disconnected!\n", i);
                    close(fds[i].fd);
                    fds[i] = fds[nfds - 1];
                    strcpy(clients_name[i], clients_name[nfds - 1]);
                    nfds--;
                    i--;
                    continue;
                }
                else{
                    char buf2[4096];
                    buf[ret] = 0;
                    sprintf(buf2, "[%s] %s: %s", time_str, clients_name[i], buf);
                    printf("Receive from client %d: %s\n", fds[i].fd, buf);
                    for(int j = 1; j < nfds; j++){
                        if(j == i) continue;
                        send(fds[j].fd, buf2, strlen(buf2), 0);
                    }
                }
            }
            
        }
    }

    close(server);
    return 0;
}