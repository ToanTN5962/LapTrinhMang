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

    struct pollfd fds[64];
    int nfds = 1;

    fds[0].fd = server;
    fds[0].events = POLLIN;

    char clients_name[FD_SETSIZE][100];
    char mes[128] = "Please enter your username and password in the form: 'username password'\n";
    char buf[128];

    while(1){
        int ret = poll(fds, nfds, -1);
        if(ret < 0){
            printf("poll() failed");
            break;
        }
        if(ret == 0){
            printf("Timed out.\n");
            continue;
        }

        for(int i = 0; i < nfds; i++){
            if(fds[i].revents & POLLIN){
                if(fds[i].fd == server){
                    int client = accept(server, NULL, NULL);
                    printf("New client connected: %d\n", client);
                    if(client < FD_SETSIZE){
                        clients_login[client] = 0;
                        fds[nfds].fd = client;
                        fds[nfds].events = POLLIN;
                        nfds++;
                        send(client, mes, strlen(mes), 0);
                    }
                    else{
                        close(client);
                    }
                }
                else {
                    int ret = recv(fds[i].fd, buf, sizeof(buf), 0);

                    if(ret <= 0){
                        printf("Client %d disconnected!\n", i);
                        fds[i] = fds[nfds -  1];
                        strcpy(clients_name[i], clients_name[nfds - 1]);
                        clients_login[fds[i].fd] = clients_login[fds[nfds - 1].fd];
                        nfds--;
                        i--;
                        continue;
                    }
                    else{
                        buf[ret] = 0;
                        buf[strcspn(buf, "\r\n")] = 0;

                        if(clients_login[i] == 0){
                            char username[50], password[50], temp[50];
                            int n = sscanf(buf, "%s %s %s", username, password, temp);
                            if(n != 2){
                                send(fds[i].fd, "Syntax error!\n", strlen("Syntax error!\n"), 0);
                                send(fds[i].fd, mes, strlen(mes), 0);
                                continue;
                            }
                            else{
                                if(check_login(username, password)){
                                    send(fds[i].fd, "Login succesfully!\n", strlen("Login succesfully!\n"), 0);
                                    strcpy(clients_name[i], username);
                                    clients_login[i] = 1;
                                }
                            }

                            if(clients_login[i] == 0){
                                send(fds[i].fd, "Cannot find username or password!\n", strlen("Cannot find username or password!\n"), 0);
                                send(fds[i].fd, mes, strlen(mes), 0);
                            }
                        }
                        else{
                            char command[256];
                            snprintf(command, sizeof(command), "%s > out.txt", buf);

                            printf("Executing %s\n", command);
                            system(command);

                            FILE *f = fopen("out.txt", "r");
                            if(!f){
                                send(fds[i].fd, "Cannot open output file\n", strlen("Cannot open output file\n"), 0);
                                continue;
                            }

                            char line[128];
                            char result[4096] = "";

                            while(fgets(line, sizeof(line), f)){
                                strcat(result, line);
                            }

                            fclose(f);
                            send(fds[i].fd, result, strlen(result), 0);
                        }
                    }
                }
            }
        }
    }

    close(server);
    return 0;
}