#ifndef NETWORK_UTIL_H_
#define NETWORK_UTIL_H_

#define MINIFS_PORT 22222

#define LS_COMMAND_ID       1
#define MKDIR_COMMAND_ID    2
#define CD_COMMAND_ID       3
#define TOUCH_COMMAND_ID    4
#define ECHO_COMMAND_ID     5
#define ECHOLINE_COMMAND_ID 6
#define CAT_COMMAND_ID      7
#define RM_COMMAND_ID       8
#define PULL_COMMAND_ID     9
#define PUSH_COMMAND_ID     10

#include <netinet/in.h>
#include <string.h>

typedef struct tcp_packet_struct {
    int size;
    void* buffer;
} tcp_packet;

void add_bytes(tcp_packet* packet, void* buffer, int size);
void add_int(tcp_packet* packet, int value);
void add_string(tcp_packet* packet, char* value);

int send_packet(int sock, tcp_packet* packet);
int recv_packet(int sock, tcp_packet* packet);

int read_bytes(tcp_packet* packet, int offset, void* res, int size);
int read_int(tcp_packet* packet, int offset, int* res);
int read_string(tcp_packet* packet, int offset, char* res);

#endif // NETWORK_UTIL_H_