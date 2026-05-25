#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

void* chat_app(void *arg);
int count_client = 0;
int clients[2];

int main(){
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int opt = 1;
    if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsockopt");
    }

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

    pthread_t thread_id;
    while(1){
        int client = accept(server, NULL, NULL);
        if(client == -1) continue;
        printf("New client connected: %d\n", client);
        clients[count_client] = client;
        count_client++;
        pthread_create(&thread_id, NULL, chat_app, (void *)&client);
        pthread_detach(thread_id);
    }

    close(server);
    return 0;
}

void* chat_app(void *arg){
    int client = *(int *)arg;
    char buf[256];
    if(count_client < 2){
        char mes[64] = "Please wait for another client to connect\n";
        send(client, mes, strlen(mes), 0);
    }
    while(1){
        int ret = recv(client, buf, sizeof(buf), 0);
        if(ret <= 0){
            printf("Client %d disconnected\n", client);
            close(clients[0]);
            close(clients[1]);
            count_client = 0;
            break;
        }
        if(count_client == 2){
            buf[ret] = '\0';
            printf("Receive from client %d: %s\n", client, buf);
            if(client == clients[0]){
                send(clients[1], buf, strlen(buf), 0);
            }
            else{
                send(clients[0], buf, strlen(buf), 0);
            }
        }
    }
}