#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#define MAX_CLIENT 1000

int count_client = 0;
int clients[MAX_CLIENT];
char clients_name[MAX_CLIENT][100];

void* chat_func(void* arg);

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
        clients[client] = 1;
        count_client++;
        pthread_create(&thread_id, NULL, chat_func, (void *)&client);
        pthread_detach(thread_id);
    }

    close(server);
    return 0;
}

void* chat_func(void *arg){
    int client = *(int *)arg;
    char buf[2048];
    char mes[128] = "Please enter your name in the form client_id: your_name\n";
    send(client, mes, strlen(mes), 0);
    char cmd[32], id[32], temp[32];
    while(1){
        int res = recv(client, buf, sizeof(buf), 0);
        int n = sscanf(buf, "%s%s%[^\n]", cmd, id, temp);

        if(n != 2){
            send(client, "Syntax error!\n", strlen("Syntax error!\n"), 0);
            //char mes[128] = "Please enter your name in the form client_id: your_name\n";
            send(client, mes, strlen(mes), 0);
            continue;
        }

        if(!strcmp(cmd, "client_id:")){
            strcpy(clients_name[client], id);
            break;
        }
        else{
            send(client, "Syntax error!\n", strlen("Syntax error!\n"), 0);
            //char mes[128] = "Please enter your name in the form client_id: your_name\n";
            send(client, mes, strlen(mes), 0);
            continue;
        }

    }

    while(1){
        int ret = recv(client, buf, sizeof(buf), 0);
        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        char time_str[32];
        strftime(time_str, sizeof(time_str), "%Y:%m:%d %H:%M:%S", t);

        if(ret <= 0){
            printf("Client %d disconnected!\n", client);
            clients[client] = 0;
            close(client);
            break;
        }
        else{
            char buf2[4096];
            buf[ret] = 0;
            sprintf(buf2, "[%s] %s: %s", time_str, clients_name[client], buf);
            printf("Receive from client %d: %s\n", client, buf);
            for(int j = 0; j < MAX_CLIENT; j++){
                if(j == client || clients[j] == 0) continue;
                send(j, buf2, strlen(buf2), 0);
            }
        }
    }
}