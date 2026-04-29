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

int topic_count;

typedef struct {
    char topic[100];
    int clients[100];
    int client_count;
} Topic;

void unsub(char *topic, int client, Topic *topics){
    for(int i = 0; i < topic_count; i++){
        if(strcmp(topic, topics[i].topic) == 0){
            for(int j = 0; j < topics[i].client_count; j++){
                if(topics[i].clients[j] == client){
                    topics[i].clients[j] = topics[i].clients[topics[i].client_count - 1];
                    topics[i].client_count--;
                    printf("Client %d has unsubcribed from topic %s\n", client, topic);
                    return;
                }
            }
        }
    }

    return;
}

void disconnetedClient(int client, Topic *topics){
    for(int i = 0; i < topic_count; i++){
        unsub(topics[i].topic, client, topics);
    }

    return;
}

int main(){
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int opt = 1;
    if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsockopt");
    }

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

    Topic topics[100];
    topic_count = 0;

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
            fds[nfds].fd = client;
            fds[nfds].events = POLLIN;
            nfds++;
        }

        for(int i = 1; i < nfds; i++){
            if(fds[i].revents & POLLIN){
                int ret = recv(fds[i].fd, buf, sizeof(buf), 0);

                if(ret <= 0){
                    printf("Client %d disconneted!\n", fds[i].fd);
                    disconnetedClient(fds[i].fd, topics);
                    fds[i].fd = fds[nfds - 1].fd;
                    nfds--;
                    i--;
                    continue;
                }
                buf[strcspn(buf, "\r\n")] = 0;

                char key[10];
                char topic[100];
                char mes[256];

                memset(key, 0, sizeof(key));
                memset(topic, 0, sizeof(topic));
                memset(mes, 0, sizeof(mes));

                int n = sscanf(buf, "%s %s %[^\n]", key, topic, mes);
                // int x = strcmp(key, "UNSUB");
                // printf("%d\n", x);

                if(strcmp(key, "SUB") == 0){
                    int found = 0;
                    for(int j = 0; j < topic_count; j++){
                        if(strcmp(topics[j].topic, topic) == 0){
                            found = 1;
                            topics[j].clients[topics[j].client_count] = fds[i].fd;
                            topics[j].client_count++;
                            printf("Client %d has subcribed to topic %s successfully!\n", fds[i].fd, topic);
                            break;
                        }
                    }
                    if(!found){
                        strcpy(topics[topic_count].topic, topic);
                        topics[topic_count].clients[0] = fds[i].fd;
                        topics[topic_count].client_count = 1;
                        topic_count++;
                        printf("Client %d has subcribed to topic %s successfully!\n", fds[i].fd, topic);
                    }
                }
                else if(strcmp(key, "UNSUB") == 0){
                    // printf("UNSUB request!\n");
                    unsub(topic, fds[i].fd, topics);
                }
                else if(strcmp(key, "PUB") == 0 && n == 3){
                    strcat(mes, "\n");
                    for(int j = 0; j < topic_count; j++){
                        if(strcmp(topics[j].topic, topic) == 0){
                            for(int k = 0; k < topics[j].client_count; k++){
                                if(topics[j].clients[k] == fds[i].fd) continue;
                                send(topics[j].clients[k], mes, strlen(mes), 0);
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    close(server);
    return 0;
}