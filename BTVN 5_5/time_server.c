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
#include<time.h>

void signalHandler(int signo){
    int pid = wait(NULL);
    printf("Child process terminated: %d\n", pid);
}

int check_format(char *command, char *format){
    if(strcmp(command, "GET_TIME") != 0) return 0;

    if(strcmp(format, "dd/mm/yyyy") == 0) return 1;
    else if(strcmp(format, "dd/mm/yy") == 0) return 2;
    else if(strcmp(format, "mm/dd/yyyy") == 0) return 3;
    else if(strcmp(format, "mm/dd/yy") == 0) return 4;

    return 0;
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

    char buf[32];

    while(1){
        int client = accept(server, NULL, NULL);
        printf("New client connected: %d\n", client);
        
        if(fork() == 0){
            close(server);
            char mes[64] = "Connected successfully!\n";
            send(client, mes, strlen(mes), 0);

            while(1){
                int ret = recv(client, buf, sizeof(buf), 0);

                if(ret <= 0){
                    printf("Client %d disconnected!\n", client);
                    close(client);
                    exit(0);
                }

                buf[ret] = 0;
                buf[strcspn(buf, "\r\n")] = 0;

                char command[10], format[15], temp[50];
                int n = sscanf(buf, "%s %s %s", command, format, temp);
                if(n != 2){
                    send(client, "Syntax error!\n", strlen("Syntax error!\n"), 0);
                    continue;
                }
                else{
                    int form = check_format(command, format);
                    if(!form){
                        send(client, "Syntax error!\n", strlen("Syntax error!\n"), 0);
                        //send(client, mes, strlen(mes), 0);
                        continue;
                    }
                    else{
                        time_t now = time(NULL);
                        struct tm *t = localtime(&now);
                        if(form == 1) strftime(buf, sizeof(buf), "%d/%m/%Y\n", t);
                        else if(form == 2) strftime(buf, sizeof(buf), "%d/%m/%y\n", t);
                        else if(form == 3) strftime(buf, sizeof(buf), "%m/%d/%Y\n", t);
                        else strftime(buf, sizeof(buf), "%m/%d/%y\n", t);

                        send(client, buf, strlen(buf), 0);
                    }
                }
            }
        }
        close(client);
    }
    close(server);
    return 0;
}