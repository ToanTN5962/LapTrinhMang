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
#include <sys/select.h>
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

    fd_set fdread, fdtest;
    struct timeval tv;
    char buf[2048];

    FD_ZERO(&fdread);
    FD_SET(server, &fdread);

    char clients_name[FD_SETSIZE][100];

    while(1){
        fdtest = fdread;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, &tv);
        if(ret < 0){
            printf("select() failed");
            return 0;
        }
        if(ret == 0){
            printf("Timed out.\n");
            continue;
        }

        for(int i = 0; i < FD_SETSIZE; i++){
            if(FD_ISSET(i, &fdtest)){
                if(i == server){
                    int client = accept(server, NULL, NULL);
                    if(client < FD_SETSIZE){
                        char mes[128] = "Please enter your name in the form client_id: your_name\n";
                        while(1){
                            send(client, mes, strlen(mes), 0);

                            int res = recv(client, buf, sizeof(buf), 0);

                            if(!strncmp(buf, "client_id: ", strlen("client_id: "))){
                                printf("New client connected: %d\n", client);
                                FD_SET(client, &fdread);
                                char *name = buf + strlen("client_id: ");
                                strcpy(clients_name[client], name);
                                break;
                            }

                        }
                    }
                    else{
                        close(client);
                    }
                }
                else{
                    int ret = recv(i, buf, sizeof(buf), 0);
                    time_t now = time(NULL);
                    struct tm *t = localtime(&now);
                    char time_str[32];
                    strftime(time_str, sizeof(time_str), "%Y:%m:%d %H:%M:%S", t);

                    if(ret <= 0){
                        printf("Client %d disconnected!\n", i);
                        FD_CLR(i, &fdread);
                    }
                    else{
                        char buf2[4096];
                        buf[ret] = 0;
                        sprintf(buf2, "[%s] %s: %s", time_str, clients_name[i], buf);
                        printf("Receive from client %d: %s\n", i, buf);
                        for(int j = 0; j < FD_SETSIZE; j++){
                            if(j == i || j == server) continue;
                            send(j, buf2, strlen(buf2), 0);
                        }
                    }
                }
            }
        }
    }

    close(server);
    return 0;
}