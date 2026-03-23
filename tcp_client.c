#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if(res == -1){
        printf("Connect fail!\n");
        return 0;
    }
    
    char msg[1024];
    fgets(msg, sizeof(msg), stdin);
    send(client, msg, strlen(msg), 0);

    close(client);
    return 0;
}