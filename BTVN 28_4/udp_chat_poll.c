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
#include <poll.h>
#include <netdb.h>
#include <time.h>

int main(int argc, char* argv[]){
    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

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

    struct pollfd fds[64];
    fds[0].fd = 0;
    fds[0].events = POLLIN;
    fds[1].fd = sock;
    fds[1].events = POLLIN;


    char buf[1024];
    int len;

    while(1){
        int ret = poll(fds, 2, -1);
        if(ret < 0){
            printf("poll() failed");
            return 0;
        }
        if(ret == 0){
            printf("Timed out.\n");
            continue;
        }

        struct sockaddr_in from;
        socklen_t from_len = sizeof(from);

        if(fds[1].revents & POLLIN){
            len = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *) &from, &from_len);
            
            if(len > 0){
                buf[len] = '\0';
                printf("\nNew message: %s\n", buf);
                fflush(stdout);
            }
        }

        if(fds[0].revents & POLLIN){
            len = read(0, buf, sizeof(buf) - 1);

            if(len > 0){
                buf[len] = '\0';
                buf[strcspn(buf, "\r\n")] = 0;
                sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &addr_d, sizeof(addr_d));
            }
        }
    }

    close(sock);
    return 0;
}