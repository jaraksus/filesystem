#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

#include "api.h"
#include "fs.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("image didn't be provided\n");
        return 0;
    }

    bool new_fs = false;
    if (argc > 2) {
        if (strcmp(argv[2], "--new") != 0) {
            printf("unknown option\n");
            return 0;
        } else {
            new_fs = true;
        }
    }

    int image_fd;
    image_fd = open(argv[1], O_RDWR);
    if (image_fd == -1) {
        printf("Can't find image to open\n");
        return 0;
    }

    char* buf = malloc(2048);
    memset(buf, 0, 2048);

    char current_path[2048];
    memset(current_path, 0, sizeof(current_path));

    if (new_fs) {
        prepare_image(image_fd);
    } else {
        load_fs(image_fd);
    }

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
        } else if (strcmp(buf, "touch") == 0) {
            scanf("%2048s", buf);
            touch(current_path, buf, image_fd);
        } else if (strcmp(buf, "echo") == 0) {
            char tmp;
            scanf("%c", &tmp);
            scanf("%[^\n]", buf);
            char filename[28];
            scanf("%28s", filename);

            echo(current_path, filename, buf, false, image_fd);
        } else if (strcmp(buf, "echoline") == 0) {
            char tmp;
            scanf("%c", &tmp);
            scanf("%[^\n]", buf);
            char filename[28];
            scanf("%28s", filename);

            echo(current_path, filename, buf, true, image_fd);
        } else if (strcmp(buf, "cat") == 0) {
            scanf("%2048s", buf);
            cat(current_path, buf, image_fd);
        } else if (strcmp(buf, "rm") == 0) {
            scanf("%2048s", buf);
            remove_file(current_path, buf, image_fd);
        } else if (strcmp(buf, "pull") == 0) {
            char src[100];
            scanf("%100s", src);
            char dest[28];
            scanf("%28s", dest);

            pull(src, dest, current_path, image_fd);
        } else if (strcmp(buf, "push") == 0) {
            char src[28];
            scanf("%28s", src);
            char dest[100];
            scanf("%100s", dest);

            push(current_path, src, dest, image_fd);
        }
        else {
            printf("unknown command\n");
        }

        show(current_path);
    }

    free(buf);
    return 0;
}