#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>

double cal(double a, double b, char* op){
    if(strcmp(op, "add") == 0){
        return a + b;
    }
    else if(strcmp(op, "sub") == 0){
        return a - b;
    }
    else if(strcmp(op, "mul") == 0){
        return a * b;
    }
    return a / b;
}

char convertOp(char* op){
    if(strcmp(op, "add") == 0){
        return '+';
    }
    else if(strcmp(op, "sub") == 0){
        return '-';
    }
    else if(strcmp(op, "mul") == 0){
        return '*';
    }
    return '/';
}

int main() {
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
    char buf[2048];
    char op[5];
    double a, b;

    while(1){
        int client = accept(server, NULL, NULL);

        printf("New client connected: %d\n", client);

        char* mes = "Please enter your operator (add, sub, mul, div) and 2 operands: ";
        send(client, mes, strlen(mes), 0);

        int res = recv(client, buf, sizeof(buf), 0);

        if(res <= 0){
            printf("Client %d disconnected!\n", client);
            close(client);
            continue;
        }

        if(strncmp(buf, "GET", 3) == 0)
        {
            char *q = strstr(buf, "?");

            if(q)
            {
                sscanf(q, "?op=%19[^&]&a=%lf&b=%lf", op, &a, &b);
            }
        }
        else if(strncmp(buf, "POST", 4) == 0)
        {
            char *body = strstr(buf, "\r\n\r\n");

            if(body)
            {
                body += 4;

                sscanf(body, "op=%19[^&]&a=%lf&b=%lf", op, &a, &b);
            }
        }

        double result = cal(a, b, op);

        char html[2048];

        sprintf(html,
            "<html>"
            "<body>"
            "<h1>Calculator Result</h1>"
            "<p>%.2lf %c %.2lf = %.2lf</p>"
            "</body>"
            "</html>",
            a, convertOp(op), b, result);

        char response[4096];

        sprintf(response,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n"
            "\r\n"
            "%s",
            strlen(html), html);

        send(client, response, strlen(response), 0);
        close(client);
    }

    close(server);
    return 0;
}