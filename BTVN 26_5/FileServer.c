#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>

#define PORT 9000
#define BUF_SIZE 8192
#define ROOT_DIR "./FILES"

/* ================= MIME TYPE ================= */
const char* get_mime_type(const char *path)
{
    char *ext = strrchr(path, '.');
    if (!ext) return "application/octet-stream";

    if (!strcmp(ext, ".html")) return "text/html";
    if (!strcmp(ext, ".txt"))  return "text/plain";
    if (!strcmp(ext, ".jpg"))  return "image/jpeg";
    if (!strcmp(ext, ".jpeg")) return "image/jpeg";
    if (!strcmp(ext, ".png"))  return "image/png";
    if (!strcmp(ext, ".gif"))  return "image/gif";
    if (!strcmp(ext, ".mp3"))  return "audio/mpeg";
    if (!strcmp(ext, ".mp4"))  return "video/mp4";

    return "application/octet-stream";
}

/* ================= URL DECODE ================= */
void url_decode(char *dst, const char *src)
{
    char a, b;

    while (*src)
    {
        if (*src == '%' &&
            ((a = src[1]) && (b = src[2])) &&
            isxdigit(a) && isxdigit(b))
        {
            char hex[3] = { a, b, 0 };
            *dst++ = (char) strtol(hex, NULL, 16);
            src += 3;
        }
        else if (*src == '+')
        {
            *dst++ = ' ';
            src++;
        }
        else
        {
            *dst++ = *src++;
        }
    }

    *dst = 0;
}

/* ================= 404 ================= */
void send_404(int client)
{
    char *body = "<html><body><h1>404 Not Found</h1></body></html>";

    char response[1024];

    sprintf(response,
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n\r\n%s",
            strlen(body),
            body);

    send(client, response, strlen(response), 0);
}

/* ================= DIRECTORY LISTING ================= */
void send_directory_listing(int client, const char *path, const char *url_path)
{
    DIR *dir = opendir(path);

    if (!dir)
    {
        send_404(client);
        return;
    }

    char html[65536];

    strcpy(html,
        "<html><body>"
        "<h2>Welcome to HTTP file server</h2>"
        "<p>Here is the folder of server:</p>");

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!strcmp(entry->d_name, "."))
            continue;

        char fullpath[1024];

        snprintf(fullpath, sizeof(fullpath),
                 "%s/%s", path, entry->d_name);

        struct stat st;
        stat(fullpath, &st);

        char link[2048];

        if (!strcmp(url_path, "/"))
            snprintf(link, sizeof(link), "/%s", entry->d_name);
        else
            snprintf(link, sizeof(link), "%s/%s", url_path, entry->d_name);

        if (S_ISDIR(st.st_mode))
        {
            strcat(html, "<a href=\"");
            strcat(html, link);
            strcat(html, "\"><b>");
            strcat(html, entry->d_name);
            strcat(html, "</b></a><br>");
        }
        else
        {
            strcat(html, "<a href=\"");
            strcat(html, link);
            strcat(html, "\"><i>");
            strcat(html, entry->d_name);
            strcat(html, "</i></a><br>");
        }
    }

    closedir(dir);

    strcat(html, "</body></html>");

    char header[1024];

    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: %ld\r\n\r\n",
            strlen(html));

    send(client, header, strlen(header), 0);
    send(client, html, strlen(html), 0);
}

/* ================= FILE SENDER ================= */
void send_file(int client, const char *path)
{
    FILE *f = fopen(path, "rb");

    if (!f)
    {
        send_404(client);
        return;
    }

    fseek(f, 0, SEEK_END);
    long filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    const char *mime = get_mime_type(path);

    char header[1024];

    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: %s\r\n"
            "Content-Length: %ld\r\n\r\n",
            mime,
            filesize);

    send(client, header, strlen(header), 0);

    char buffer[4096];
    size_t n;

    while ((n = fread(buffer, 1, sizeof(buffer), f)) > 0)
    {
        send(client, buffer, n, 0);
    }

    fclose(f);
}

/* ================= MAIN ================= */
int main()
{
    int server = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server, (struct sockaddr*)&addr, sizeof(addr));
    listen(server, 10);

    printf("HTTP File Server listening on %d\n", PORT);

    while (1)
    {
        int client = accept(server, NULL, NULL);

        char request[BUF_SIZE];
        int len = recv(client, request, sizeof(request) - 1, 0);

        if (len <= 0)
        {
            close(client);
            continue;
        }

        request[len] = 0;

        char method[16];
        char url[1024];

        sscanf(request, "%15s %1023s", method, url);

        printf("%s %s\n", method, url);

        /* ===== decode URL ===== */
        char decoded_url[1024];
        url_decode(decoded_url, url);

        char path[2048];

        /* root fix */
        if (!strcmp(decoded_url, "/"))
        {
            snprintf(path, sizeof(path), "%s", ROOT_DIR);
        }
        else
        {
            snprintf(path, sizeof(path), "%s%s", ROOT_DIR, decoded_url);
        }

        struct stat st;

        if (stat(path, &st) < 0)
        {
            send_404(client);
        }
        else if (S_ISDIR(st.st_mode))
        {
            send_directory_listing(client, path, decoded_url);
        }
        else
        {
            send_file(client, path);
        }

        close(client);
    }

    close(server);
    return 0;
}