#include "api.h"
#include "fs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void set_white_output() {
    printf("\033[0;37m");
}

void set_blue_output() {
    printf("\033[0;34m");
}

void set_cyan_output() {
    printf("\033[0;36m");
}

void set_purple_output() {
    printf("\033[0;35m");
}

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
    set_cyan_output();
    printf("%s", path);
    printf("/ ");
    set_white_output();
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
        if (cur > get_block_size()) {
            cur = get_block_size();
        }

        rem -= cur;

        char block[get_block_size()];
        read_block(block, node.blocks[i], image_fd);

        int total_read = 0;
        while (total_read < cur) {
            catalog_record record;
            memcpy(&record, block + total_read, sizeof(catalog_record));
            total_read += sizeof(catalog_record);

            inode cur_node;
            get_inode(&cur_node, record.inode_id, image_fd);
            if (cur_node.dir) {
                set_blue_output();
            }
            printf("%s\n", record.name);
            set_white_output();
        }
    }
}


void make_dir(char* path, char* dirname, int image_fd) {
    int cur_inode_id = find_inode_id(path, image_fd);

    if (find_inode_id_by_dir(cur_inode_id, dirname, image_fd) != -1) {
        printf("Directory %s already exists\n", dirname);
        return;
    }

    int new_inode_id = init_inode(true, image_fd);
    if (new_inode_id == -1) {
        printf("Can't make directory. No free inodes\n");
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
    int cur_dir_inode_id = find_inode_id(current_path, image_fd);

    int inode_id = find_inode_id_by_dir(cur_dir_inode_id, dirname, image_fd);
    if (inode_id == -1) {
        printf("No such file or directory\n");
        return;
    }

    inode node;
    get_inode(&node, inode_id, image_fd);
    if (!node.dir) {
        printf("It is not a directory\n");
        return;
    }

    if (strcmp(dirname, "..") == 0) {
        pop(current_path);
    } else {
        append(current_path, dirname);
    }
}

void touch(char* path, char* filename, int image_fd) {
    int cur_dir_inode_id = find_inode_id(path, image_fd);

    if (find_inode_id_by_dir(cur_dir_inode_id, filename, image_fd) != -1) {
        printf("File %s already exists\n", filename);
        return;
    }

    int new_inode_id = init_inode(false, image_fd);
    if (new_inode_id == -1) {
        printf("Can't make file. No free inodes\n");
        return;
    }

    catalog_record record;
    record.inode_id = new_inode_id;
    strcpy(record.name, filename);
    write_by_inode_id(cur_dir_inode_id, &record, sizeof(catalog_record), image_fd);
}


void echo(char* path, char* filename, char* content, bool eof, int image_fd) {
    int cur_dir_inode_id = find_inode_id(path, image_fd);

    int inode_id = find_inode_id_by_dir(cur_dir_inode_id, filename, image_fd);
    if (inode_id == -1) {
        printf("No such file\n");
        return;
    }

    inode node;
    get_inode(&node, inode_id, image_fd);
    if (node.dir) {
        printf("Can't write to a directory\n");
        return;
    }

    write_by_inode_id(inode_id, content, strlen(content), image_fd);
    if (eof == true) {
        write_by_inode_id(inode_id, "\n", 1, image_fd);
    }
}

void cat(char* path, char* filename, int image_fd) {
    int cur_dir_inode_id = find_inode_id(path, image_fd);

    int inode_id = find_inode_id_by_dir(cur_dir_inode_id, filename, image_fd);
    if (inode_id == -1) {
        printf("No such file or directory\n");
        return;
    }

    inode node;
    get_inode(&node, inode_id, image_fd);
    if (node.dir) {
        printf("Can't use cat to a directory\n");
        return;
    }

    char* buffer = malloc(node.size + 1);
    memset(buffer, 0, node.size + 1);

    int total_read = 0;
    int block_ind = 0;
    while (total_read < node.size) {
        char current_buffer[get_block_size()];
        read_block_by_inode_id(current_buffer, inode_id, block_ind, image_fd);

        int can_read = get_block_size();
        if (node.size - total_read < can_read) {
            can_read = node.size - total_read;
        }

        memcpy(buffer + total_read, current_buffer, can_read);

        total_read += can_read;
        block_ind++;
    }

    printf("%s\n", buffer);

    free(buffer);
}

void remove_file(char* path, char* filename, int image_fd) {
    int cur_dir_inode_id = find_inode_id(path, image_fd);

    int inode_id = find_inode_id_by_dir(cur_dir_inode_id, filename, image_fd);
    if (inode_id == -1) {
        printf("No such file or directory\n");
        return;
    }

    if (strcmp(filename, "..") == 0) {
        printf("Can't remove parent directory\n");
        return;
    }

    make_free(inode_id, cur_dir_inode_id, image_fd);
}
