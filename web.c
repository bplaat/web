// gcc -Wall -Wextra -Wpedantic -Werror -Wshadow -std=c11 web.c -lpthread -o web && ./web public 8080

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

char *root;

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
            if (file_name[strlen(file_name) - 1] == '/') {
                strcpy(&path_buffer[strlen(path_buffer)], "index.html");
            }

            puts(path_buffer);

            int file;
            if ((file = open(path_buffer, O_RDONLY)) != -1) {
                char *found_response = "HTTP/1.1 200 OK\r\n\r\n";
                write(client_socket, found_response, strlen(found_response));

                size_t file_bytes_read;
                while ((file_bytes_read = read(file, buffer, BUFFER_SIZE)) > 0) {
                    write(client_socket, buffer, file_bytes_read);
                }
                close(file);
            } else {
                strcpy(path_buffer, root);
                strcpy(&path_buffer[strlen(path_buffer)], "/404.html");
                if ((file = open(path_buffer, O_RDONLY)) != -1) {
                    char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n";
                    write(client_socket, not_found_response, strlen(not_found_response));

                    size_t file_bytes_read;
                    while ((file_bytes_read = read(file, buffer, BUFFER_SIZE)) > 0) {
                        write(client_socket, buffer, file_bytes_read);
                    }
                    close(file);
                } else {
                    char *not_found_response = "HTTP/1.1 404 Not Found\r\n\r\n404 Not Found\n";
                    write(client_socket, not_found_response, strlen(not_found_response));
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
    if (argc >= 3) {
        root = argv[1];
        char *port = argv[2];

        int server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1) {
            puts("Could not create socket");
            return EXIT_FAILURE;
        }

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(atoi(port));

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
