#ifndef FS_H_
#define FS_H_

#include <stdbool.h>
#include <stddef.h>

// #define BLOCK_SIZE 2048
// #define BLOCK_NUM 1024
// #define INODE_NUM 65536

#define MAX_BLOCK_NUM 1024
#define MAX_INODE_NUM 65536

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
    char name[28];
} catalog_record;

typedef struct superblock_struct {
    int block_size;
    int block_num;
    int inode_num;
} superblock;

int get_block_size();
int get_block_num();
int get_inode_num();

void move_to_inode(int fd, int inode_id);

void move_to_block(int fd, int block_id);

void get_inode(inode* node, int inode_id, int image_fd);

void save_inode(inode* node, int inode_id, int image_fd);

int find_inode_id_by_dir(int dir_inode_id, char* filename, int image_fd);
int find_inode_id(char* path, int image_fd);

void read_block(void* buf, int block_id, int image_fd);
void read_block_by_inode_id(void* buf, int inode_id, int block_ind, int image_fd);

int init_inode(bool dir, int image_fd);

void write_by_inode_id(int inode_id, void* buf, int count, int image_id);

void make_free(int inode_id, int parent_inode_id, int image_fd);

// ********************************************************

void prepare_image(int image_fd);

void load_fs(int image_fd);

#endif // FS_H_