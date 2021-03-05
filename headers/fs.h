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
    size_t size;
    size_t blocks[14];

    bool dir;

    size_t counter;
} inode;

typedef struct catalog_record_struct {
    size_t inode_id;
    char name[20];
} catalog_record;

void move_to_inode(int fd, int inode_id);

void move_to_block(int fd, int block_id);

void get(inode* node, size_t inode_id, int image_fd);

size_t find_inode_id(size_t dir_inode_id, char* filename);


#endif // FS_H_