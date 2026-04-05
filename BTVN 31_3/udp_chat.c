#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>

int main(int argc, char *argv[]){
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    unsigned long ul = 1;
    ioctl(sock, FIONBIO, &ul);
    ioctl(0, FIONBIO, &ul);

    int port_s = atoi(argv[1]);
    int port_d = atoi(argv[3]);
    char *ip_d = argv[2];

    struct sockaddr_in addr_s;
    addr_s.sin_family = AF_INET;
    addr_s.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_s.sin_port = htons(port_s);

    bind(sock, (struct sockaddr *) &addr_s, sizeof(addr_s));

    printf("Starting chat on port: %d\n", port_s);

    struct sockaddr_in addr_d;
    addr_d.sin_family = AF_INET;
    addr_d.sin_addr.s_addr = inet_addr(ip_d);
    addr_d.sin_port = htons(port_d);

    char buf[1024];
    int len;

    while(1){
        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);

        len = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &from, &from_len);
        
        if(len > 0){
            buf[len] = '\0';
            printf("\nNew message: %s\n", buf);
            fflush(stdout);
        }
        else if(len == -1){
            if(errno != EWOULDBLOCK) {
                perror("recvfrom error");
            }
        }

        len = read(0, buf, sizeof(buf) - 1);

        if(len > 0){
            buf[len] = '\0';
            buf[strcspn(buf, "\r\n")] = 0;
            sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &addr_d, sizeof(addr_d));
        }
        else if(len == -1){
            if(errno != EWOULDBLOCK) {
                perror("send error");
            }
        }

        usleep(1000);
    }
    
    close(sock);
    return 0;
}

