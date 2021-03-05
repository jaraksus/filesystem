#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "api.h"
#include "fs.h"

int image_fd;

void append(char* path, char* name) {
    int id = strlen(path);
    path[id] = '/';
    id++;

    for (int i = 0; i < strlen(name); ++i) {
        path[id + i] = name[i];
    }
}

void pop(char* path) {
    int id = strlen(path) - 1;
    while (id >= 0 && path[id] != '/') {
        path[id] = 0;
        id--;
    }

    if (id >= 0) {
        path[id] = 0;
    }
}

void show(char* path) {
    printf("%s", path);
    printf("/ ");
}

int main(int argc, char* argv[]) {
    image_fd = open(argv[1], O_RDWR);
    char* buf = malloc(2048);
    memset(buf, 0, 2048);

    char current_path[2048];
    memset(current_path, 0, sizeof(current_path));

    show(current_path);
    while (scanf("%2048s", buf) != EOF) {
        if (strcmp(buf, "ls") == 0) {
        } else {
            printf("unknown command\n");
        }

        show(current_path);
    }

    free(buf);
    return 0;
}