#include <stdbool.h>
#include <stddef.h>

#ifndef FS_H_
#define FS_H_

#define BLOCK_SIZE 2048
#define BLOCK_NUM 1024
#define INODE_NUM 65536

/*
|----------|----------|
   inodes     blocks
*/

typedef struct inode_struct {
    int size;
    int blocks[14];

    bool dir;

    int counter;
} inode;

typedef struct catalog_record_struct {
    int inode_id;
    char name[20];
} catalog_record;

void move_to_inode(int fd, int inode_id);

void move_to_block(int fd, int block_id);

void get_inode(inode* node, int inode_id, int image_fd);

void save_inode(inode* node, int inode_id, int image_fd);

int find_inode_id_by_dir(int dir_inode_id, char* filename, int image_fd);
int find_inode_id(char* path, int image_fd);

void read_block(void* buf, int block_id, int image_fd);

int init_inode(bool dir, int image_fd);

void write_by_inode_id(int inode_id, void* buf, int count, int image_id);

// ********************************************************

void prepare_image(int image_fd);

#endif // FS_H_