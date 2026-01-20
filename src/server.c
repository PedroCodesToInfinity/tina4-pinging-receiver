#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define BUFFER_SIZE 8192
#define DATA_FILE "data/requests.txt"

/* Extract Content-Length from POST headers */
static int get_content_length(const char *req) {
    const char *cl = strstr(req, "Content-Length:");
    if (!cl) return 0;

    cl += 15;
    while (*cl == ' ') cl++;
    return atoi(cl);
}

/* Minimal URL decode */
static void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {

            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';

            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';

            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

void server_start(int port) {
    int server_fd;
    int client_fd;
    struct sockaddr_in addr;
    int opt;
    char buffer[BUFFER_SIZE];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    /* Allow immediate reuse of the port */
    opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons((unsigned short)port);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
        close(server_fd);
        exit(1);
    }

    printf("Listening on port %d...\n", port);

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }

        int bytes;
        int content_length;
        int body_len;
        char *body;
        char params[BUFFER_SIZE];
        FILE *f;

        memset(buffer, 0, BUFFER_SIZE);
        bytes = read(client_fd, buffer, BUFFER_SIZE - 1);
        if (bytes <= 0) {
            close(client_fd);
            continue;
        }
        buffer[bytes] = '\0';

        printf("\n----- Incoming Request -----\n%s\n----------- End ------------\n", buffer);

        if (strncmp(buffer, "POST", 4) == 0) {
            content_length = get_content_length(buffer);
            body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4;
                body_len = bytes - (body - buffer);

                /* Read remaining POST body if needed */
                while (body_len < content_length && bytes < BUFFER_SIZE - 1) {
                    int more = read(client_fd, buffer + bytes, BUFFER_SIZE - bytes - 1);
                    if (more <= 0) break;
                    bytes += more;
                    body_len += more;
                }

                /* Null-terminate exactly at body length */
                if (body_len > content_length) body_len = content_length;
                body[body_len] = '\0';

                url_decode(params, body);

                f = fopen(DATA_FILE, "a");
                if (f) {
                    fprintf(f, "%s\n", params);
                    fclose(f);
                } else {
                    perror("fopen");
                }
            }
        }

        /* Minimal HTTP response */
        const char *resp =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 2\r\n"
            "Connection: close\r\n"
            "\r\n"
            "OK";
        write(client_fd, resp, strlen(resp));

        close(client_fd);
    }

    close(server_fd);
}
