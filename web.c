// gcc -Wall -Wextra -Wpedantic -Werror -Wshadow -std=c11 web.c -lpthread -o web && ./web public

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

char *mime_types[] = {
    "txt",  "text/plain",

    "htm", "text/html",
    "html", "text/html",
    "css",  "text/css",
    "js",   "application/javascript",

    "ico",  "image/vnd.microsoft.icon",
    "bmp",  "image/bmp",
    "jpg",  "image/jpeg",
    "jpeg", "image/jpeg",
    "png",  "image/png",
    "gif",  "image/gif",
    "svg",  "image/svg+xml",

    "pdf",  "application/pdf",
    "zip",  "application/zip",

    "json", "application/json",
    "xml",  "application/xml",
    0
};

struct thread_args {
    int client_socket;
};

char *root;

void *connection_handler(void *args) {
    struct thread_args *thread_args = args;
    int client_socket = thread_args->client_socket;

    char *buffer = malloc(BUFFER_SIZE);
    char *path_buffer = malloc(256);

    size_t request_bytes_read = read(client_socket, buffer, BUFFER_SIZE);
    if (request_bytes_read == 0) {
        puts("Client request read error");
    }
    else {
        char *http_type = strtok(buffer, " ");
        char *file_name = strtok(NULL, " ");
        char *protocol = strtok(NULL, "\r\n");
        file_name = strtok(strtok(file_name, "#"), "?");

        if (strcmp(http_type, "GET") == 0 && (strcmp(protocol, "HTTP/1.0") == 0 || strcmp(protocol, "HTTP/1.1") == 0)) {
            strcpy(path_buffer, root);
            strcat(path_buffer, file_name);

            struct stat stats;
            stat(path_buffer, &stats);
            if (S_ISDIR(stats.st_mode)) {
                strcat(path_buffer, "index.html");
            }

            puts(path_buffer);

            char *extension = "";
            char *dot = strrchr(path_buffer, '.');
            if (dot) {
                extension = dot + 1;
            }

            char *mime = "application/octet-stream";
            for (int i = 0; mime_types[i] != 0; i += 2) {
                if (strcmp(extension, mime_types[i]) == 0) {
                    mime = mime_types[i + 1];
                    break;
                }
            }

            FILE *file;
            if ((file = fopen(path_buffer, "r")) != NULL) {
                fseek(file, 0, SEEK_END);
                size_t file_length = ftell(file);
                rewind(file);
                sprintf(buffer, "%s 200 OK\r\n"
                    "Connection: close\r\n"
                    "Content-Length: %ld\t\n"
                    "Content-Type: %s\r\n\r\n", protocol, file_length, mime);
                write(client_socket, buffer, strlen(buffer));

                size_t file_bytes_read;
                while ((file_bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                    write(client_socket, buffer, file_bytes_read);
                }
                fclose(file);
            }
            else {
                strcpy(path_buffer, root);
                strcat(path_buffer, "/404.html");
                if ((file = fopen(path_buffer, "r")) != NULL) {
                    fseek(file, 0, SEEK_END);
                    size_t file_length = ftell(file);
                    rewind(file);
                    sprintf(buffer, "%s 404 Not Found\r\n"
                        "Connection: close\r\n"
                        "Content-Length: %ld\r\n"
                        "Content-Type: text/html\r\n\r\n", protocol, file_length);
                    write(client_socket, buffer, strlen(buffer));

                    size_t file_bytes_read;
                    while ((file_bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                        write(client_socket, buffer, file_bytes_read);
                    }
                    fclose(file);
                }
                else {
                    char *not_found_reponse_body = "404 Not Found!\n";
                    sprintf(buffer, "%s 404 Not Found\r\n"
                        "Connection: close\r\n"
                        "Content-Length: %ld\r\n"
                        "Content-Type: text/plain\r\n\r\n", protocol, strlen(not_found_reponse_body));
                    write(client_socket, buffer, strlen(buffer));
                    write(client_socket, not_found_reponse_body, strlen(not_found_reponse_body));
                }
            }
        }
    }

    close(client_socket);

    free(thread_args);
    free(buffer);
    free(path_buffer);

    return EXIT_SUCCESS;
}

int server_socket;

void close_server_socket(int signum) {
    (void)signum;
    close(server_socket);
    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    root = argc >= 2 ? argv[1] : ".";
    int port = argc >= 3 ? atoi(argv[2]) : 8080;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        puts("Could not create socket");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        puts("Bind failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    signal(SIGINT, close_server_socket);

    listen(server_socket, 1000);

    printf("Serving '%s' at http://127.0.0.1:%d/\n", root, port);

    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_address_size = sizeof(struct sockaddr_in);

    while ((client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_size)) >= 0) {
        pthread_t client_thread;
        struct thread_args *thread_args = malloc(sizeof(thread_args));
        thread_args->client_socket = client_socket;
        if (pthread_create(&client_thread, NULL, connection_handler, thread_args) > 0) {
            puts("Could not create thread");
            close(server_socket);
            return EXIT_FAILURE;
        }
    }

    if (client_socket < 0) {
        puts("Accept failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    close(server_socket);
    return EXIT_SUCCESS;
}
