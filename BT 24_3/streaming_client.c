#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(){
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    int res = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if(res == -1){
        printf("Connect fail!\n");
        return 0;
    }

    char str[1024];
    while(fgets(str, sizeof(str), stdin)){
        send(client, str, strlen(str), 0);
    }
    
    close(client);
    return 0;
}