#include "fs.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

bool inodes_mask[INODE_NUM];
bool blocks_mask[BLOCK_NUM];

int read_all(int fd, void* buf, int count) {
    int total = 0;
    while (total < count) {
        int cur = read(fd, buf + total, count - total);

        if (cur == 0) {
            return -1;
        }

        total += cur;
    }

    return 0;
}

int write_all(int fd, void* buf, int count) {
    int total = 0;
    while (total < count) {
        int cur = write(fd, buf + total, count - total);

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

void get_inode(inode* node, int inode_id, int image_fd) {
    move_to_inode(image_fd, inode_id);
    read_all(image_fd, node, sizeof(inode));
}

void save_inode(inode* node, int inode_id, int image_fd) {
    move_to_inode(image_fd, inode_id);
    write_all(image_fd, node, sizeof(inode));
}

void read_block(void* buf, int block_id, int image_fd) {
    move_to_block(image_fd, block_id);
    read_all(image_fd, buf, BLOCK_SIZE);
}

int init_inode(bool dir, int image_fd) {
    for (int i = 0; i < INODE_NUM; ++i) {
        if (inodes_mask[i] == false) {
            inodes_mask[i] = true;

            inode node;
            node.size = 0;
            memset(node.blocks, 0, sizeof(node.blocks));
            node.dir = dir;
            node.counter = 1;

            int blocks_found = 0;
            for (int j = 0; j < BLOCK_NUM, blocks_found < 14; ++j) {
                if (blocks_mask[j] == false) {
                    blocks_mask[j] = true;
                    node.blocks[blocks_found] = j;
                    blocks_found++;
                }
            }

            move_to_inode(image_fd, i);
            write_all(image_fd, &node, sizeof(inode));

            return i;
        }
    }

    return -1;
}

int find_inode_id_by_dir(int dir_inode_id, char* filename, int image_fd) {
    inode node;
    get_inode(&node, dir_inode_id, image_fd);

    int rem = node.size;
    for (int i = 0; i < 14; ++i) {
        if (rem == 0) {
            break;
        }
        int cur = rem;
        if (cur > BLOCK_SIZE) {
            cur = BLOCK_SIZE;
        }

        rem -= cur;

        char block[BLOCK_SIZE];
        read_block(block, node.blocks[i], image_fd);

        int total_read = 0;
        while (total_read < cur) {
            catalog_record record;
            memcpy(&record, block + total_read, sizeof(catalog_record));
            total_read += sizeof(catalog_record);

            if (strcmp(filename, record.name) == 0) {
                return record.inode_id;
            }
        }
    }

    return -1;
}

int find_inode_id(char* path, int image_fd) {
    int inode_id = 0; // root inode_id

    char name[20];

    if (strlen(path) > 0) { // if is not root
        int id = 1;
        while (path[id] != 0) {
            int r = id;
            memset(name, 0, sizeof(name));

            while (path[r] != 0 && path[r] != '/') {
                name[r - id] = path[r];
                r++;
            }

            inode_id = find_inode_id_by_dir(inode_id, name, image_fd);

            if (inode_id == -1) {
                return inode_id;
            }

            id = r + 1;
        }
    }

    return inode_id;
}

void write_to_block(int block_id, int offset, char* buf, int count, int image_fd) {
    move(image_fd, INODE_NUM * sizeof(inode) + block_id * BLOCK_SIZE + offset);
    write_all(image_fd, buf, count);
}

void write_by_inode_id(int inode_id, void* buf, int count, int image_fd) {
    inode node;
    get_inode(&node, inode_id, image_fd);

    int block_ind = (node.size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    int block_rem = BLOCK_SIZE - (node.size - BLOCK_SIZE * block_ind);

    int total_wrote = 0;
    while (total_wrote < count) {
        int can_write = count - total_wrote;
        if (block_rem < can_write) {
            can_write = block_rem;
        }

        write_to_block(node.blocks[block_ind], BLOCK_SIZE - block_rem, buf + total_wrote, can_write, image_fd);
        total_wrote += can_write;

        block_ind++;
        block_rem = BLOCK_SIZE;
    }

    node.size += count;
    save_inode(&node, inode_id, image_fd);
}

// *************************************************************

void prepare_image(int image_fd) {
    int fs_size = INODE_NUM * sizeof(inode) + BLOCK_SIZE * BLOCK_NUM;
    char* ptr = malloc(fs_size);
    memset(ptr, 0, fs_size);
    write_all(image_fd, ptr, fs_size);

    int root_inode_id = init_inode(true, image_fd);

    free(ptr);
}