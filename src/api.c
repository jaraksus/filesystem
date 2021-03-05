#include "api.h"
#include "fs.h"

#include <stdio.h>
#include <string.h>

void list(char* path, int image_fd) {
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
    get(&node, inode_id, image_fd);

    if (node.dir == false) {
        printf("Something strange happened. This is not a directory");
        return;
    }

    // LIST FILES FROM DIRECTORY WITH THAT INODE
    // ITERATE THROUGH BLOCKS AND FIND ALL FILES
    // TODO
}