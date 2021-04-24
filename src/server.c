#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <pthread.h>

#include "network_util.h"

#include "api.h"
#include "fs.h"

int image_fd;
char current_path[2048];
char buffer[65356];

int listener;
struct sockaddr_in addr;

void init_network() {
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener == -1) {
        printf("socket creation failed\n");
        exit(-1);
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MINIFS_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listener, (struct sockaddr*)&addr, sizeof(struct sockaddr_in)) != 0) {
        printf("bind failed\n");
        exit(-1);
    }

    if (listen(listener, 100) != 0) {
        printf("listen failed\n");
        exit(-1);
    }
}

void message_response(int sock, char* message) {
    tcp_packet response;
    response.size = 0;
    response.buffer = NULL;

    add_string(&response, message);
    send_packet(sock, &response);

    free(response.buffer);
}

void handle_ls_request(int sock) {
    list_output(current_path, image_fd, buffer);
    message_response(sock, buffer);
}

void handle_echo_request(int sock, tcp_packet* request, bool eol) {
    int offset = 4;

    char filename[28];
    offset = read_string(request, offset, filename);

    char buffer[2048];
    read_string(request, offset, buffer);

    echo(current_path, filename, buffer, eol, image_fd);
}

void handle_cat_request(int sock, tcp_packet* request) {
    char filename[28];
    read_string(request, 4, filename);
    cat_output(current_path, filename, image_fd, buffer);
    message_response(sock, buffer);
}

void handle_pull_request(tcp_packet* packet) {
    int offset = 4;

    char src[100];
    offset = read_string(packet, offset, src);
    char dest[28];
    read_string(packet, offset, dest);

    pull(src, dest, current_path, image_fd);
}

void handle_push_request(tcp_packet* packet) {
    int offset = 4;

    char src[28];
    offset = read_string(packet, offset, src);
    char dest[100];
    read_string(packet, offset, dest);

    push(current_path, src, dest, image_fd);
}

pthread_mutex_t mutex;

void* session(void* arg) {
    char session_current_path[2048];
    memset(session_current_path, 0, 2048);
    int sock = *((int*)arg);

    printf("sock: %d\n", sock);

    while (1) {
        message_response(sock, session_current_path);

        tcp_packet packet;
        packet.size = 0;
        packet.buffer = NULL;

        if (recv_packet(sock, &packet) != 0) {
            printf("recieve packet failed\n");
            free(packet.buffer);
            break;
        }

        pthread_mutex_lock(&mutex);
        strcpy(current_path, session_current_path);

        int command_id;
        read_int(&packet, 0, &command_id);

        if (command_id == LS_COMMAND_ID) {
            printf("handling ls command...\n");
            handle_ls_request(sock);
        } else if (command_id == MKDIR_COMMAND_ID) {
            printf("handling mkdir command...\n");
            make_dir(current_path, packet.buffer + 4, image_fd);
        } else if (command_id == CD_COMMAND_ID) {
            printf("handling cd command...\n");
            change_directory(current_path, packet.buffer + 4, image_fd);
        } else if (command_id == TOUCH_COMMAND_ID) {
            printf("handling touch command...\n");
            touch(current_path, packet.buffer + 4, image_fd);
        } else if (command_id == ECHO_COMMAND_ID) {
            printf("handling echo command...\n");
            handle_echo_request(sock, &packet, false);
        } else if (command_id == ECHOLINE_COMMAND_ID) {
            printf("handling echoline command...\n");
            handle_echo_request(sock, &packet, true);
        } else if (command_id == CAT_COMMAND_ID) {
            printf("handling cat command...\n");
            handle_cat_request(sock, &packet);
        } else if (command_id == RM_COMMAND_ID) {
            printf("handling cat command...\n");
            remove_file(current_path, packet.buffer + 4, image_fd);
        } else if (command_id == PULL_COMMAND_ID) {
            printf("nadling pull command...\n");
            handle_pull_request(&packet);
        } else if (command_id == PUSH_COMMAND_ID) {
            printf("handling push command...\n");
            handle_push_request(&packet);
        }

        free(packet.buffer);

        strcpy(session_current_path, current_path);
        printf("session_current_path after requery: %s\n", session_current_path);
        pthread_mutex_unlock(&mutex);
    }

    close(sock);
    free(arg);
}

int main(int argc, char* argv[]) {
    bool new_fs = false;
    if (argc > 1) {
        if (strcmp(argv[1], "--new") != 0) {
            printf("unknown option\n");
            return 0;
        } else {
            new_fs = true;
        }
    }

    image_fd = open("/dev/lkm_minifs", O_RDWR);
    if (image_fd == -1) {
        printf("Can't open device. Try to use sudo\n");
        return 0;
    }

    memset(current_path, 0, sizeof(current_path));

    if (new_fs) {
        prepare_image(image_fd);
    } else {
        load_fs(image_fd);
    }

    init_network();
    printf("server listening on port: %d\n", MINIFS_PORT);

    pthread_mutex_init(&mutex, NULL);

    while (1) {
        int* sock = malloc(4);
        *sock = accept(listener, NULL, NULL);
        printf("new connection\n");
        pthread_t thread_id;
        pthread_create(&thread_id, NULL, session, (void *)sock);
    }

    pthread_mutex_destroy(&mutex);
}

