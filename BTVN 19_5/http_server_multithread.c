#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

void* http_func(void* arg);

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
    int num_threads = 8;
    pthread_t thread_id;
    char buf[256];

    for(int i = 0; i < num_threads; i++){
        pthread_create(&thread_id, NULL, http_func, (void *)&server);
        pthread_join(thread_id, NULL);
    }
    
    close(server);
    return 0;
}

void* http_func(void *arg){
    int server = *(int *)arg;
    char buf[256];
    while(1){
        int client = accept(server, NULL, NULL);
        printf("Client %d accepted in thread %ld with pid %d\n", client, pthread_self(), getpid());
        int ret = recv(client, buf, sizeof(buf), 0);
        if(ret <= 0) {
            printf("Client %d disconnected\n", client);
            close(client);
            continue;
        }
        buf[ret] = 0;
        printf("Received from client %d: %s\n", client, buf);
        char *mes = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao</h1></body></html>";
        send(client, mes, strlen(mes), 0);
    }
}