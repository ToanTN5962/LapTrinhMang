#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

typedef struct {
    char filename[100];
    int filesize;
} fileinfo;

int main(){
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(9000);

    int res = connect(client, (struct sockaddr *) &addr, sizeof(addr));

    if(res == -1){
        printf("Connection failed!\n");
        return 0;
    }

    fileinfo fi;
    DIR *d;
    struct dirent *dir;
    struct stat st;
    d = opendir(".");

    if(d){
        while((dir = readdir(d)) != NULL){
            strcpy(fi.filename, dir -> d_name);
            stat(dir -> d_name, &st);
            fi.filesize = st.st_size;
            send(client, &fi, sizeof(fi), 0);
        }
    }

    close(client);
    return 0;
}