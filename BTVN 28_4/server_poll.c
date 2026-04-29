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
    addr.sin_addr.s_addr = INADDR_ANY;
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
            printf("New client connected %d\n", client);
            char mes[64];
            sprintf(mes, "Xin chao. Hien co %d clients dang ket noi.\n", nfds);
            send(client, mes, strlen(mes), 0);
            fds[nfds].fd = client;
            fds[nfds].events = POLLIN;
            nfds++;
        }

        for(int i = 1; i < nfds; i++){
            if(fds[i].revents & POLLIN){
                int ret = recv(fds[i].fd, buf, sizeof(buf), 0);
                buf[strcspn(buf, "\r\n")] = 0;

                if(strcmp(buf, "exit") == 0 || ret <= 0){
                    printf("Client %d disconnected!\n", fds[i].fd);
                    fds[i].fd = fds[nfds - 1].fd;
                    nfds--;
                    i--;
                }
                else{
                    buf[ret] = 0;
                    printf("Received from client %d: %s\n", fds[i].fd, buf);
                    for(int j = 0; j < strlen(buf); j++){
                        if((buf[j] >= 97 && buf[j] < 122) || buf[j] >= 65 && buf[j] < 90){
                            buf[j]++;
                        }
                        else if(buf[j] == 122 || buf[j] == 90){
                            buf[j] -= 24;
                        }
                        else if(isdigit(buf[j])){
                            int n = buf[j] - '0';
                            n = 9 - n;
                            buf[j] = n + '0';
                        }
                    }

                    buf[strlen(buf)] = '\n';
                    send(fds[i].fd, buf, strlen(buf), 0);
                }
            }
        }
    }

    close(server);
    return 0;
}