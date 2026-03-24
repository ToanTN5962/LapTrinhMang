#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(){
    int sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    char buf[256];
    while(1){
        printf("Nhap tin nhan: ");
        fgets(buf, sizeof(buf), stdin);
        printf("\n");
        sendto(sender, buf, strlen(buf), 0, (struct sockaddr *) &addr, sizeof(addr));

        int len = recvfrom(sender, buf, sizeof(buf) - 1, 0, NULL, NULL);
        buf[len] = '\0';
        printf("Response from server: %s\n", buf);
    }

    close(sender);
    return 0;
}