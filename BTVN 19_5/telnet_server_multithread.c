#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

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

void* telnet_func(void* arg);

int main(){
    int server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    int opt = 1;
    if(setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsockopt");
    }

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
    pthread_t thread_id;

    while(1){
        int client = accept(server, NULL, NULL);
        printf("New client accepted: %d\n", client);
        pthread_create(&thread_id, NULL, telnet_func, (void *)&client);
        pthread_detach(thread_id);
    }

    close(server);
    return 0;
}

void* telnet_func(void* arg){
    int client = *(int *)arg;
    char buf[128];
    char mes[128] = "Please enter your username and password in the form: 'username password'\n";
    send(client, mes, strlen(mes), 0);
    int logged = 0;
    while(1){
        int ret = recv(client, buf, sizeof(buf), 0);

        if(ret <= 0){
            printf("Client %d disconnected!\n", client);
            close(client);
            break;
        }
        else{
            buf[ret] = 0;
            buf[strcspn(buf, "\r\n")] = 0;

            if(logged == 0){
                char username[50], password[50], temp[50];
                int n = sscanf(buf, "%s %s %[^\n]", username, password, temp);
                if(n != 2){
                    send(client, "Syntax error!\n", strlen("Syntax error!\n"), 0);
                    send(client, mes, strlen(mes), 0);
                    continue;
                }
                else{
                    if(check_login(username, password)){
                        send(client, "Login succesfully!\n", strlen("Login succesfully!\n"), 0);
                        logged = 1;
                    }
                }

                if(logged == 0){
                    send(client, "Cannot find username or password!\n", strlen("Cannot find username or password!\n"), 0);
                    send(client, mes, strlen(mes), 0);
                }
            }
            else{
                char command[256];
                snprintf(command, sizeof(command), "%s > out.txt", buf);

                printf("Executing %s\n", command);
                system(command);

                FILE *f = fopen("out.txt", "r");
                if(!f){
                    send(client, "Cannot open output file\n", strlen("Cannot open output file\n"), 0);
                    continue;
                }

                char line[128];
                char result[4096] = "";

                while(fgets(line, sizeof(line), f)){
                    strcat(result, line);
                }

                fclose(f);
                send(client, result, strlen(result), 0);
            }
        }
    }
}