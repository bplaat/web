// gcc -Wall -Wextra -Wpedantic -Werror -Wshadow -std=c11 web.c -lpthread -o web && ./web 8080 public

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

char *root;

char *mime_types[] = {
    "txt",  "text/plain",
    "html", "text/html",
    "css",  "text/css",
    "js",   "application/javascript",
    "ico",  "image/vnd.microsoft.icon",
    "jpg",  "image/jpg",
    "png",  "image/png",
    "gif",  "image/gif",
    0
};

struct thread_args {
    int client_socket;
};

void *connection_handler(void *args) {
    struct thread_args *thread_args = args;
    int client_socket = thread_args->client_socket;

    char *buffer = malloc(BUFFER_SIZE);
    char *path_buffer = malloc(500);

    size_t request_bytes_read = read(client_socket, buffer, BUFFER_SIZE);
    if (request_bytes_read == 0) {
        puts("Client request read error");
    }
    else {
        char *http_type = strtok(buffer, " ");
        char *file_name = strtok(NULL, " ");
        char *protocol = strtok(NULL, "\r\n");

        if (strcmp(http_type, "GET") == 0 && strcmp(protocol, "HTTP/1.1") == 0) {
            strcpy(path_buffer, root);
            strcpy(&path_buffer[strlen(path_buffer)], file_name);

            struct stat stats;
            stat(path_buffer, &stats);
            if (S_ISDIR(stats.st_mode)) {
                strcpy(&path_buffer[strlen(path_buffer)], "index.html");
            }

            puts(path_buffer);

            char *ext;
            char *dot = strrchr(path_buffer, '.');
            if (!dot || dot == path_buffer) ext = "";
            ext = dot + 1;

            char *mime;
            for (int i = 0; mime_types[i] != 0; i += 2) {
                if (strcmp(ext, mime_types[i]) == 0) {
                    mime = mime_types[i + 1];
                    break;
                }
            }

            FILE *file;
            if ((file = fopen(path_buffer, "r")) != NULL) {
                fseek(file, 0, SEEK_END);
                size_t file_length = ftell(file);
                rewind(file);
                sprintf(buffer, "HTTP/1.1 200 OK\r\n"
                    "Connection: close\r\n"
                    "Content-Length: %ld\t\n"
                    "Content-Type: %s\r\n\r\n", file_length, mime);
                write(client_socket, buffer, strlen(buffer));

                size_t file_bytes_read;
                while ((file_bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                    write(client_socket, buffer, file_bytes_read);
                }
                fclose(file);
            }
            else {
                strcpy(path_buffer, root);
                strcpy(&path_buffer[strlen(path_buffer)], "/404.html");
                if ((file = fopen(path_buffer, "r")) != NULL) {
                    fseek(file, 0, SEEK_END);
                    size_t file_length = ftell(file);
                    rewind(file);
                    sprintf(buffer, "HTTP/1.1 404 Not Found\r\n"
                        "Connection: close\r\n"
                        "Content-Length: %ld\r\n"
                        "Content-Type: %s\r\n\r\n", file_length, mime);
                    write(client_socket, buffer, strlen(buffer));

                    size_t file_bytes_read;
                    while ((file_bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
                        write(client_socket, buffer, file_bytes_read);
                    }
                    fclose(file);
                }
                else {
                    char *not_found_reponse_body = "404 Not Found!\n";
                    sprintf(buffer, "HTTP/1.1 404 Not Found\r\n"
                        "Connection: close\r\n"
                        "Content-Length: %ld\r\n"
                        "Content-Type: text/plain\r\n\r\n", strlen(not_found_reponse_body));
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

int main(int argc, char *argv[]) {
    if (argc >= 2) {
        if (argc >= 3) {
            root = argv[2];
        } else {
            root = ".";
        }

        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1) {
            puts("Could not create socket");
            return EXIT_FAILURE;
        }

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(atoi(argv[1]));

        if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
            puts("Bind failed");
            return EXIT_FAILURE;
        }
        puts("Bind done");

        listen(server_socket, 1000);

        int client_socket;
        struct sockaddr_in client_address;
        socklen_t client_address_size = sizeof(struct sockaddr_in);

        while ((client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_size)) >= 0) {
            pthread_t client_thread;
            struct thread_args *thread_args = malloc(sizeof(thread_args));
            thread_args->client_socket = client_socket;
            if (pthread_create(&client_thread, NULL, connection_handler, thread_args) > 0) {
                puts("Could not create thread");
                return EXIT_FAILURE;
            }
        }

        if (client_socket < 0) {
            puts("Accept failed");
            return EXIT_FAILURE;
        }

        close(server_socket);
    }
    return EXIT_SUCCESS;
}
