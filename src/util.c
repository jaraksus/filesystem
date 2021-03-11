#include "util.h"
#include <unistd.h>

int read_all(int fd, void* buf, int count) {
    int total = 0;
    while (total < count) {
        int cur = read(fd, buf + total, count - total);

        if (cur == 0) {
            return -1;
        }

        total += cur;
    }

    return 0;
}

int write_all(int fd, void* buf, int count) {
    int total = 0;
    while (total < count) {
        int cur = write(fd, buf + total, count - total);

        if (cur == 0) {
            return -1;
        }

        total += cur;
    }

    return 0;
}