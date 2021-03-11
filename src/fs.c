#include "fs.h"
#include "util.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>


superblock fs_info;

bool inodes_mask[MAX_INODE_NUM];
bool blocks_mask[MAX_BLOCK_NUM];

int get_block_size() {
    return fs_info.block_size;
}

int get_block_num() {
    return fs_info.block_num;
}

int get_inode_num() {
    return fs_info.inode_num;
}

void move(int fd, int offset) {
    lseek(fd, offset, SEEK_SET);
}

void move_to_superblock(int fd) {
    move(fd, 0);
}

void move_to_inode(int fd, int inode_id) {
    move(fd, fs_info.block_size + inode_id * sizeof(inode));
}

void move_to_block(int fd, int block_id) {
    move(fd, fs_info.block_size + fs_info.inode_num * sizeof(inode) + block_id * fs_info.block_size);
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
    read_all(image_fd, buf, fs_info.block_size);
}

void read_block_by_inode_id(void* buf, int inode_id, int block_ind, int image_fd) {
    inode node;
    get_inode(&node, inode_id, image_fd);
    read_block(buf, node.blocks[block_ind], image_fd);
}

int init_inode(bool dir, int image_fd) {
    for (int i = 0; i < fs_info.inode_num; ++i) {
        if (inodes_mask[i] == false) {
            inodes_mask[i] = true;

            inode node;
            node.size = 0;
            memset(node.blocks, 0, sizeof(node.blocks));
            node.dir = dir;
            node.counter = 1;

            int blocks_found = 0;
            for (int j = 0; j < fs_info.block_num, blocks_found < 14; ++j) {
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
        if (cur > fs_info.block_size) {
            cur = fs_info.block_size;
        }

        rem -= cur;

        char block[fs_info.block_size];
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

void write_to_block(int block_id, int offset, void* buf, int count, int image_fd) {
    // move(image_fd, INODE_NUM * sizeof(inode) + block_id * BLOCK_SIZE + offset);
    move_to_block(image_fd, block_id);
    lseek(image_fd, offset, SEEK_CUR);
    write_all(image_fd, buf, count);
}

void write_by_inode_id(int inode_id, void* buf, int count, int image_fd) {
    inode node;
    get_inode(&node, inode_id, image_fd);

    int block_ind = node.size / fs_info.block_size;
    int block_rem = fs_info.block_size - (node.size % fs_info.block_size);

    int total_wrote = 0;
    while (total_wrote < count) {
        int can_write = count - total_wrote;
        if (block_rem < can_write) {
            can_write = block_rem;
        }

        write_to_block(node.blocks[block_ind], fs_info.block_size - block_rem, buf + total_wrote, can_write, image_fd);
        total_wrote += can_write;

        block_ind++;
        block_rem = fs_info.block_size;
    }

    node.size += count;
    save_inode(&node, inode_id, image_fd);
}

void make_free(int inode_id, int parent_inode_id, int image_fd) {
    inode node;
    get_inode(&node, inode_id, image_fd);

    char buffer[fs_info.block_size];

    // Recursive delete strategy
    if (node.dir) {
        int total_read = 0;
        int block_ind = 0;
        catalog_record record;
        while (total_read < node.size) {
            read_block_by_inode_id(buffer, inode_id, block_ind, image_fd);

            int current_read = 0;
            while (total_read < node.size && current_read < fs_info.block_size) {
                memcpy(&record, buffer + current_read, sizeof(catalog_record));
                total_read += sizeof(catalog_record);
                current_read += sizeof(catalog_record);

                if (record.inode_id != parent_inode_id) {
                    make_free(record.inode_id, inode_id, image_fd);
                }
            }

            block_ind++;
        }
    }

    node.counter--;
    save_inode(&node, inode_id, image_fd);
    for (int i = 0; i < 14; ++i) {
        blocks_mask[node.blocks[i]] = false;
    }
    inodes_mask[inode_id] = false;

    // Deleting from parent record
    inode parent_node;
    get_inode(&parent_node, parent_inode_id, image_fd);

    catalog_record last_record;
    int last_block_ind = parent_node.size / fs_info.block_size;
    int last_block_offset = parent_node.size % fs_info.block_size;
    if (last_block_offset == 0) {
        read_block_by_inode_id(buffer, parent_inode_id, last_block_ind - 1, image_fd);
        memcpy(&last_record, buffer + (fs_info.block_size - sizeof(catalog_record)), sizeof(catalog_record));
    } else {
        read_block_by_inode_id(buffer, parent_inode_id, last_block_ind, image_fd);
        memcpy(&last_record, buffer + (last_block_offset - sizeof(catalog_record)), sizeof(catalog_record));
    }

    if (last_record.inode_id != inode_id) {
        int total_read = 0;
        int block_ind = 0;
        bool found = false;
        catalog_record record;
        while (total_read < parent_node.size && !found) {
            read_block_by_inode_id(buffer, parent_inode_id, block_ind, image_fd);

            int current_read = 0;
            while (total_read < parent_node.size && current_read < fs_info.block_size) {
                memcpy(&record, buffer + current_read, sizeof(catalog_record));

                if (record.inode_id == inode_id) {
                    write_to_block(
                        parent_node.blocks[block_ind], current_read, &last_record, sizeof(catalog_record), image_fd
                    );

                    found = true;
                    break;
                }

                total_read += sizeof(catalog_record);
                current_read += sizeof(catalog_record);
            }

            block_ind++;
        }

        if (!found) {
            printf("Something going wrong. Can't find inode to delte in parent directory\n");
        }
    }

    parent_node.size -= sizeof(catalog_record);
    save_inode(&parent_node, parent_inode_id, image_fd);
}

// *******************************************************************

void prepare_image(int image_fd) {
    /*
    #define BLOCK_SIZE 2048
    #define BLOCK_NUM 1024
    #define INODE_NUM 65536
    */

    fs_info.block_size = 2048;
    fs_info.block_num = 1024;
    fs_info.inode_num = 65536;

    int fs_size = fs_info.block_size + fs_info.inode_num * sizeof(inode) + fs_info.block_size * fs_info.block_num;
    char* ptr = malloc(fs_size);
    memset(ptr, 0, fs_size);
    write_all(image_fd, ptr, fs_size);

    move_to_superblock(image_fd);
    write_all(image_fd, &fs_info, sizeof(superblock));

    int root_inode_id = init_inode(true, image_fd);

    free(ptr);
}

void load_fs(int image_fd) {
    move_to_superblock(image_fd);
    read_all(image_fd, &fs_info, sizeof(superblock));

    inode node;
    for (int i = 0; i < fs_info.inode_num; ++i) {
        get_inode(&node, i, image_fd);
        if (node.counter == 1) {
            inodes_mask[i] = true;
            for (int j = 0; j < 14; ++j) {
                if (node.blocks[j] != -1) {
                    blocks_mask[node.blocks[j]] = true;
                }
            }
        }
    }
}