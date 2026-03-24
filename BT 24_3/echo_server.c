#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(){
    int receiver = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    bind(receiver, (struct sockaddr *) &addr, sizeof(addr));

    char buf[256];
    while(1){
        int receive = recvfrom(receiver, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &client_addr, &client_len);

        if(receive == -1){
            printf("Cannot receive message!");
            break;
        }
        else{
            buf[receive] = '\0';
            printf("Receive from client: %s\n", buf);

            sendto(receiver, buf, sizeof(buf), 0, (struct sockaddr *) &client_addr, client_len);
        }
    }

    close(receiver);
    return 0;
}