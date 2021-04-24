//
// Created by jarakcyc on 23.04.2021.
//

#include <malloc.h>
#include "network_util.h"

int send_packet(int sock, tcp_packet* packet) {
    if (send(sock, &packet->size, sizeof(int), 0) == -1) {
        return -1;
    }
    if (send(sock, packet->buffer, packet->size, 0) == -1) {
        return -1;
    }
    return 0;
}

int recv_packet(int sock, tcp_packet* packet) {
    int size;
    ssize_t total = 0;
    while (total < 4) {
        ssize_t cur = recv(sock, &size + total, 4 - total, 0);
        if (cur <= 0) {
            return -1;
        }
        total += cur;
    }

    packet->size = size;
    packet->buffer = malloc(size);
    memset(packet->buffer, 0, size);

    total = 0;
    while (total < size) {
        ssize_t cur = recv(sock, packet->buffer + total, size - total, 0);
        if (cur <= 0) {
            return -1;
        }
        total += cur;
    }

    return 0;
}

void add_bytes(tcp_packet* packet, void* buffer, int size) {
    packet->buffer = realloc(packet->buffer, packet->size + size);
    memcpy(packet->buffer + packet->size, buffer, size);
    packet->size += size;
}

void add_int(tcp_packet* packet, int value) {
    add_bytes(packet, &value, sizeof(int));
}

void add_string(tcp_packet* packet, char* value) {
    add_int(packet, strlen(value) + 1);
    add_bytes(packet, value, strlen(value) + 1);
}

int read_bytes(tcp_packet* packet, int offset, void* res, int size) {
    memcpy(res, packet->buffer + offset, size);
    return offset + size;
}

int read_int(tcp_packet* packet, int offset, int* res) {
    memcpy(res, packet->buffer + offset, sizeof(int));
    return offset + 4;
}

int read_string(tcp_packet* packet, int offset, char* res) {
    int size;
    offset = read_int(packet, offset, &size);
    offset = read_bytes(packet, offset, res, size);
    return offset;
}
