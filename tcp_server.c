#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server != -1){
        printf("Socket created: %d\n", server);
    }
    else{
        printf("Failed to create socket!\n");
        return 0;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if(bind(server, (struct sockaddr *)&addr, sizeof(addr))){
        printf("Bind failed\n");
    }
    else{
        printf("Bind success\n");
    }

    if(listen(server, 5)){
        printf("Listen failed\n");
        return 0;
    }

    printf("Waiting for a client...\n");

    int client = accept(server, NULL, NULL);
    if(client == -1){
        printf("Accept failed");
        return 0;
    }

    FILE *f = fopen(argv[2], "r");
    char buf[1024];
    int n;
    while((n = fread(buf, 1, 1024, f)) > 0){
        send(client, buf, n, 0);
    }

    fclose(f);

    FILE *file = fopen(argv[3], "a");
    if(file != NULL){
        char buf2[1024];
        int n;
        while((n = recv(client, buf2, 1024, 0)) > 0){
            fwrite(buf2, 1, n, file);
        }

        fclose(file);
    }

    close(client);
    close(server);

    return 0;
}