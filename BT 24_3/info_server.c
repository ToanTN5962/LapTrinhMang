#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

typedef struct {
    char filename[100];
    int filesize;
} fileinfo;

int main(){
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(server != -1){
        printf("Server created: %d\n", server);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);
    
    if(bind(server, (struct sockaddr *)&addr, sizeof(addr))){
        printf("Bind failed!\n");
        return 0;
    }

    if(listen(server, 5)){
        printf("Listen failed!\n");
        return 0;
    }

    int client = accept(server, NULL, NULL);
    fileinfo fi;
    printf("Nhan thong tin cac file trong thu muc tu client\n");
    while(recv(client, &fi, sizeof(fi), 0) > 0){
        printf("%s - %d\n", fi.filename, fi.filesize);
    }

    close(client);
    close(server);
    return 0;
}