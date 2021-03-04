#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define BLOCK_SIZE 2048
#define BLOCK_NUM 1024
#define INODE_NUM 65536

/*
|----------|----------|
   inodes     blocks
*/

int image_fd;

typedef struct inode_struct {
    size_t size;
    size_t blocks[14];

    bool dir;

    size_t counter;
} inode;

typedef struct catalog_record_struct {
    size_t inode_id;
    char name[20];
} catalog_record;

bool free_inodes[INODE_NUM];
bool free_blocks[BLOCK_NUM];

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

int read_all(int fd, void* buf, size_t count) {
    ssize_t total = 0;
    while (total < count) {
        ssize_t cur = read(fd, buf + total, count - total);

        if (cur == 0) {
            return -1;
        }

        total += cur;
    }

    return 0;
}

int write_all(int fd, void* buf, size_t count) {
    ssize_t total = 0;
    while (total < count) {
        ssize_t cur = write(fd, buf + total, count - total);

        if (cur == 0) {
            return -1;
        }

        total += cur;
    }

    return 0;
}

void move(int fd, int offset) {
    lseek(fd, offset, SEEK_SET);
}

void move_to_inode(int fd, int inode_id) {
    move(fd, inode_id * sizeof(inode));
}

void move_to_block(int fd, int block_id) {
    move(fd, INODE_NUM * sizeof(inode) + block_id * BLOCK_SIZE);
}

// ******************* API *******************

void get(inode* node, size_t inode_id) {
    move_to_inode(image_fd, inode_id);
    read_all(image_fd, node, sizeof(node));
}

size_t find_inode_id(size_t dir_inode_id, char* filename) {
    // TODO
    return 0;
}

void list(char* path) {
    size_t inode_id = 0; // root inode_id

    char name[20];

    if (strlen(path) > 0) { // if is not root
        size_t id = 1;
        while (path[id] != 0) {
            size_t r = id;
            memset(name, 0, sizeof(name));

            while (path[r] != 0 && path[r] != '/') {
                name[r - id] = path[r];
                r++;
            }

            inode_id = find_inode_id(inode_id, name);
            id = r + 1;
        }
    }



    inode node;
    get(&node, inode_id);

    if (node.dir == false) {
        printf("Something strange happened. This is not a directory");
        return;
    }

    // LIST FILES FROM DIRECTORY WITH THAT INODE
    // ITERATE THROUGH BLOCKS AND FIND ALL FILES
    // TODO
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