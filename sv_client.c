#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

typedef struct {
    char mssv[10];
    char hoten[50];
    char ngaysinh[20];
    float cpa;
} sinhvien;

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

    sinhvien sv;
    
    printf("Nhap MSSV: ");
    fgets(sv.mssv, sizeof(sv.mssv), stdin);

    printf("\nNhap ho va ten: ");
    fgets(sv.hoten, sizeof(sv.hoten), stdin);

    printf("\nNhap ngay thang nam sinh: ");
    fgets(sv.ngaysinh, sizeof(sv.ngaysinh), stdin);

    printf("\nNhap CPA: ");
    scanf("%f", &sv.cpa);

    send(client, &sv, sizeof(sv), 0);

    close(client);
    return 0;
}