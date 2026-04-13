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
#include <sys/select.h>
#include <netdb.h>
#include <time.h>

int clients_login[FD_SETSIZE];

int check_login(char *username, char *password){
    FILE *f = fopen("password.txt", "r");
    if(!f) return 0;

    char user[50], pass[50];
    
    while(fscanf(f, "%s %s", user, pass) != EOF){
        if(!strcmp(user, username) && !strcmp(pass, password)){
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

int main(){
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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

    fd_set fdread, fdtest;
    struct timeval tv;
    char buf[128];

    FD_ZERO(&fdread);
    FD_SET(server, &fdread);

    char clients_name[FD_SETSIZE][100];
    char mes[128] = "Please enter your username and password in the form: 'username password'\n";

    while(1){
        fdtest = fdread;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, &tv);
        if(ret < 0){
            printf("select() failed");
            break;
        }
        if(ret == 0){
            printf("Timed out.\n");
            continue;
        }

        for(int i = 0; i < FD_SETSIZE; i++){
            if(FD_ISSET(i, &fdtest)){
                if(i == server){
                    int client = accept(server, NULL, NULL);
                    if(client < FD_SETSIZE){
                        clients_login[client] = 0;
                        FD_SET(client, &fdread);
                        
                        send(client, mes, strlen(mes), 0);
                        
                    }
                    else{
                        close(client);
                    }
                }
                else {
                    int ret = recv(i, buf, sizeof(buf), 0);
                    // time_t now = time(NULL);
                    // struct tm *t = localtime(&now);
                    // char time_str[32];
                    // strftime(time_str, sizeof(time_str), "%Y:%m:%d %H:%M:%S", t);

                    if(ret <= 0){
                        printf("Client %d disconnected!\n", i);
                        FD_CLR(i, &fdread);
                    }
                    else{
                        buf[ret] = 0;
                        buf[strcspn(buf, "\r\n")] = 0;

                        if(clients_login[i] == 0){
                            char username[50], password[50];
                            if(sscanf(buf, "%s %s", username, password) == 2){
                                if(check_login(username, password)){
                                    printf("New client connected: %d\n", i);
                                    send(i, "Login succesfully!\n", strlen("Login succesfully!\n"), 0);
                                    FD_SET(i, &fdread);
                                    strcpy(clients_name[i], username);
                                    clients_login[i] = 1;
                                }
                            }

                            if(clients_login[i] == 0){
                                send(i, "Cannot find username or password!\n", strlen("Cannot find username or password!\n"), 0);
                                send(i, mes, strlen(mes), 0);
                            }
                        }
                        else{
                            char command[256];
                            snprintf(command, sizeof(command), "%s > out.txt", buf);

                            printf("Executing %s\n", command);
                            system(command);

                            FILE *f = fopen("out.txt", "r");
                            if(!f){
                                send(i, "Cannot open output file\n", strlen("Cannot open output file\n"), 0);
                                continue;
                            }

                            char line[128];
                            char result[4096] = "";

                            while(fgets(line, sizeof(line), f)){
                                strcat(result, line);
                            }

                            fclose(f);
                            send(i, result, strlen(result), 0);
                        }
                    }
                }
            }
        }
    }

    close(server);
    return 0;
}