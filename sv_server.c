#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    char mssv[10];
    char hoten[50];
    char ngaysinh[20];
    float cpa;
} sinhvien;

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

    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int client = accept(server, (struct sockaddr *)&client_addr, &addr_len);
    if(client == -1){
        printf("Accept failed");
        return 0;
    }

    // FILE *f = fopen(argv[2], "r");
    // char buf[1024];
    // int n;
    // while((n = fread(buf, 1, 1024, f)) > 0){
    //     send(client, buf, n, 0);
    // }

    // fclose(f);

    sinhvien sv;

    int n = recv(client, &sv, sizeof(sv), 0);
    char *ip = inet_ntoa(client_addr.sin_addr);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);

    char curtime[64];
    strftime(curtime, sizeof(curtime), "%Y-%m-%d %H:%M:%S", t);

    printf("%s %s %s %s %s %.2f\n", ip, curtime, sv.mssv, sv.hoten, sv.ngaysinh, sv.cpa);

    FILE *file = fopen(argv[2], "a");
    if(file != NULL){
        fprintf(file, "%s %s %s %s %s %.2f\n", ip, curtime, sv.mssv, sv.hoten, sv.ngaysinh, sv.cpa);

        fclose(file);
    }

    close(client);
    close(server);

    return 0;
}