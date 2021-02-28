#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int read_all(int fd, void* buf, size_t count) {
    ssize_t total = 0;
    while (total < count) {
        ssize_t cur = read(fd, buf + total, count - total);

        if (cur == 0) {
            return -1;
        }

        total += cur;
    }

    return 0;
}

int write_all(int fd, void* buf, size_t count) {
    ssize_t total = 0;
    while (total < count) {
        ssize_t cur = write(fd, buf + total, count - total);

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

int main(int argc, char* argv[]) {
    int image_fd = open(argv[1], O_RDWR);
    char* buf = malloc(2048);
    memset(buf, 0, 2048);

    free(buf);
    return 0;
}