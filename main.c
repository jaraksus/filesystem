#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "api.h"
#include "fs.h"

int image_fd;

int main(int argc, char* argv[]) {
    image_fd = open(argv[1], O_RDWR);
    char* buf = malloc(2048);
    memset(buf, 0, 2048);

    char current_path[2048];
    memset(current_path, 0, sizeof(current_path));

    prepare_image(image_fd);

    /*inode node;
    get(&node, 0, image_fd);
    printf("%d\n", node.size);*/

    show(current_path);
    while (scanf("%2048s", buf) != EOF) {
        if (strcmp(buf, "ls") == 0) {
            list(current_path, image_fd);
        } else if (strcmp(buf, "mkdir") == 0) {
            scanf("%2048s", buf);
            make_dir(current_path, buf, image_fd);
        } else if (strcmp(buf, "cd") == 0) {
            scanf("%2048s", buf);
            change_directory(current_path, buf, image_fd);
        }
        else {
            printf("unknown command\n");
        }

        show(current_path);
    }

    free(buf);
    return 0;
}