#include "fs.h"

#include <sys/types.h>
#include <unistd.h>

bool free_inodes[INODE_NUM];
bool free_blocks[BLOCK_NUM];

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

void get(inode* node, size_t inode_id, int image_fd) {
    move_to_inode(image_fd, inode_id);
    read_all(image_fd, node, sizeof(node));
}

size_t find_inode_id(size_t dir_inode_id, char* filename) {
    // TODO
    return 0;
}