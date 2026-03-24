#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(){
    int count = 0;
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
    addr.sin_port = htons(9000);

    if(bind(server, (struct sockaddr *)&addr, sizeof(addr))){
        printf("Bind failed\n");
        return 0;
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

    char fullstr[10000] = "";
    char buf[1024];
    int receive;

    while((receive = recv(client, buf, sizeof(buf) - 1, 0)) > 0){
        buf[receive] = '\0';
        for(int i = 0; i < receive; i++){
            if(buf[i] != '\n'){
                int len = strlen(fullstr);
                fullstr[len] = buf[i];
            }
        }

        char *pos = fullstr;
        count = 0;

        while((pos = strstr(pos, "0123456789")) != NULL){
            count++;
            pos++;
        }

        printf("So lan xuat hien trong xau goc la: %d\n", count);
        fflush(stdout);
    }

    close(client);
    close(server);

    return 0;
}