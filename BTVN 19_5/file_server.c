#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>

void signalHandler(int signo){
    int pid = wait(NULL);
    printf("Child process terminated: %d\n", pid);
}

long getFileSize(const char *filename) {
    FILE *fp = fopen(filename, "rb"); 
    if (fp == NULL) {
        return -1;
    }

    fseek(fp, 0, SEEK_END); 
    long size = ftell(fp); 
    
    fclose(fp);
    return size;
}

int main(){
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int opt = 1;
    if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsockopt");
    }
    signal(SIGCHLD, signalHandler);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    if(bind(server, (struct sockaddr *) &addr, sizeof(addr))){
        perror("bind() failed");
        close(server);
        return 0;
    }

    if(listen(server, 5)){
        perror("listen() failed");
        close(server);
        return 0;
    }

    printf("Server is listening on port 9000...\n");

    while(1){
        int client = accept(server, NULL, NULL);
        printf("New client connected %d\n", client);

        if(fork() == 0){ 
            close(server);
            char mes[64] = "Connected successfully!\n";
            send(client, mes, strlen(mes), 0);
            char file_list[1024] = "";
            char temp[1024] = "";
            int cnt = 0;

            DIR *dir;
            struct dirent *entry;
            dir = opendir("./");
            if (dir == NULL) {
                perror("opendir");
                return 1;
            }

            while((entry = readdir(dir)) != NULL){
                strcat(temp, entry->d_name);
                strcat(temp, "\r\n");
                cnt++;
            }

            sprintf(file_list, "OK %d\r\n", cnt);
            strcat(file_list, temp);
            strcat(file_list, "\r\n\r\n");
            send(client, file_list, strlen(file_list), 0);

            if(cnt == 0){
                char mes[64] = "ERROR No files to download \r\n";
                send(client, mes, strlen(mes), 0);
                close(client);
                exit(0);
            }
            else {
                while(1){
                    char mes[64] = "Enter file name to download: ";
                    send(client, mes, strlen(mes), 0);
                    char buf[64];
                    int n = recv(client, buf, sizeof(buf), 0);
                    if(n <= 0){
                        close(client);
                        exit(0);
                    }
                    else{
                        buf[strcspn(buf, "\r\n")] = 0;
                        int found = 0;
                        dir = opendir("./");
                        char mes[4096];
                        char temp[4000];
                        while((entry = readdir(dir)) != NULL){
                            if(!strcmp(entry->d_name, buf)){
                                found = 1;
                                long fileSize = getFileSize(entry->d_name);
                                sprintf(mes, "OK %ld\r\n", fileSize);
                                FILE *fp = fopen(entry->d_name, "rb");
                                if (fp == NULL) {
                                    perror("Error opening file");
                                    return 1;
                                }
                                fread(temp, 1, sizeof(temp) - 1, fp);
                                strcat(mes, temp);
                                send(client, mes, strlen(mes), 0);
                                close(client);
                                exit(0);
                            }
                        }
                        if(!found){
                            sprintf(mes, "No file named %s\n", buf);
                            send(client, mes, strlen(mes), 0);
                            continue;
                        }
                    }
                }
            }
        }
        close(client);
    }
    close(server);
    return 0;
}