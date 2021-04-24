//
// Created by jarakcyc on 23.04.2021.
//

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "network_util.h"

tcp_packet request;

int sock;
struct sockaddr_in addr;

void init_network() {
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("socket creation failed\n");
        exit(1);
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MINIFS_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) != 0) {
        printf("connection failed\n");
        exit(1);
    }
}

void handle_message_response(int sock) {
    tcp_packet server_response;
    server_response.size = 0;
    server_response.buffer = NULL;

    recv_packet(sock, &server_response);
    char buffer[2048];
    read_string(&server_response, 0, buffer);
    printf("%s", buffer);

    free(server_response.buffer);
}

void ls_request() {
    add_int(&request, LS_COMMAND_ID);
}

void mkdir_request(char* dirname) {
    add_int(&request, MKDIR_COMMAND_ID);
    add_bytes(&request, dirname, strlen(dirname) + 1);
}

void cd_request(char* dirname) {
    add_int(&request, CD_COMMAND_ID);
    add_bytes(&request, dirname, strlen(dirname) + 1);
}

void touch_request(char* filename) {
    add_int(&request, TOUCH_COMMAND_ID);
    add_bytes(&request, filename, strlen(filename) + 1);
}

void echo_request(char* content, char* filename, bool eol) {
    if (eol) {
        add_int(&request, ECHOLINE_COMMAND_ID);
    } else {
        add_int(&request, ECHO_COMMAND_ID);
    }

    add_string(&request, filename);
    add_string(&request, content);
}

void cat_request(char* filename) {
    add_int(&request, CAT_COMMAND_ID);
    add_string(&request, filename);
}

void rm_request(char* filename) {
    add_int(&request, RM_COMMAND_ID);
    add_bytes(&request, filename, strlen(filename) + 1);
}

void pull_request(char* src, char* dst) {
    add_int(&request, PULL_COMMAND_ID);
    add_string(&request, src);
    add_string(&request, dst);
}

void push_request(char* src, char* dst) {
    add_int(&request, PUSH_COMMAND_ID);
    add_string(&request, src);
    add_string(&request, dst);
}

int main() {
    init_network();

    char buf[2048];
    memset(buf, 0, 2048);

    while (scanf("%2047s", buf) != EOF) {
        request.size = 0;
        request.buffer = NULL;

        if (strcmp(buf, "ls") == 0) {
            ls_request();
            send_packet(sock, &request);
            handle_message_response(sock);
        } else if (strcmp(buf, "mkdir") == 0) {
            scanf("%2047s", buf);

            mkdir_request(buf);
            send_packet(sock, &request);
        } else if (strcmp(buf, "cd") == 0) {
            scanf("%2047s", buf);

            cd_request(buf);
            send_packet(sock, &request);
        } else if (strcmp(buf, "touch") == 0) {
            scanf("%2047s", buf);

            touch_request(buf);
            send_packet(sock, &request);
        } else if (strcmp(buf, "echo") == 0) {
            char tmp;
            scanf("%c", &tmp);
            scanf("%[^\n]", buf);
            char filename[28];
            scanf("%27s", filename);

            echo_request(buf, filename, false);
            send_packet(sock, &request);
        } else if (strcmp(buf, "echoline") == 0) {
            char tmp;
            scanf("%c", &tmp);
            scanf("%[^\n]", buf);
            char filename[28];
            scanf("%27s", filename);

            echo_request(buf, filename, true);
            send_packet(sock, &request);
        } else if (strcmp(buf, "cat") == 0) {
            scanf("%2047s", buf);

            cat_request(buf);
            send_packet(sock, &request);
            handle_message_response(sock);
        } else if (strcmp(buf, "rm") == 0) {
            scanf("%2047s", buf);

            rm_request(buf);
            send_packet(sock, &request);
        } else if (strcmp(buf, "pull") == 0) {
            char src[100];
            scanf("%99s", src);
            char dest[28];
            scanf("%27s", dest);

            pull_request(src, dest);
            send_packet(sock, &request);
        } else if (strcmp(buf, "push") == 0) {
            char src[28];
            scanf("%27s", src);
            char dest[100];
            scanf("%99s", dest);

            push_request(src, dest);
            send_packet(sock, &request);
        } else {
            printf("unknown command\n");
        }

        free(request.buffer);
    }

    close(sock);
}