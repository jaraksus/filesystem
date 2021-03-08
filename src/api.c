#include "api.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>

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

void list(char* path, int image_fd) {
    int inode_id = find_inode_id(path, image_fd);

    inode node;
    get_inode(&node, inode_id, image_fd);

    if (node.dir == false) {
        printf("Something strange happened. This is not a directory");
        return;
    }

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

            printf("%s\n", record.name);
        }
    }
}


void make_dir(char* path, char* dirname, int image_fd) {
    int cur_inode_id = find_inode_id(path, image_fd);

    int new_inode_id = init_inode(true, image_fd);

    if (new_inode_id == -1) {
        printf("Can't make directory. No free inodes");
        return;
    }

    catalog_record record;

    record.inode_id = new_inode_id;
    strcpy(record.name, dirname);
    write_by_inode_id(cur_inode_id, &record, sizeof(catalog_record), image_fd);

    record.inode_id = cur_inode_id;
    strcpy(record.name, "..");
    write_by_inode_id(new_inode_id, &record, sizeof(catalog_record), image_fd);
}


void change_directory(char* current_path, char* dirname, int image_fd) {
    int inode_id = find_inode_id(current_path, image_fd);

    if (find_inode_id_by_dir(inode_id, dirname, image_fd) == -1) {
        printf("No such file or directory");
        return;
    }

    if (strcmp(dirname, "..") == 0) {
        pop(current_path);
    } else {
        append(current_path, dirname);
    }
}
