#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

char buf[512];

void cp(int src_fd, int dst_fd) {
    int bytes_read;

    while ((bytes_read = read(src_fd, buf, sizeof buf)) > 0) {
        if (write(dst_fd, buf, bytes_read) != bytes_read) {
            printf(2, "cp: write error\n");
            exit();
        }
    }
    if (bytes_read < 0) {
        printf(2, "cp: read error\n");
        exit();
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf(2, "Usage: cp source destination\n");
        exit();
    }

    int src_fd = open(argv[1], O_RDONLY);
    int dst_fd = open(argv[2], O_CREATE | O_WRONLY);

    if (src_fd < 0) {
        printf(2, "cp: unable to open source file %s to read\n", argv[1]);
        exit();
    }
    if (dst_fd < 0) {
        printf(2, "cp: unable to open destination file %s to write\n", argv[2]);
        exit();
    }

    cp(src_fd, dst_fd);
    exit();
}
