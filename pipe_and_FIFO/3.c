#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <errno.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

// FIFO可以打开既读又写?

int main(int argc, char* argv[])
{
    // 创建FIFO
    mkfifo("fifo_test", FILE_MODE);
    // 打开FIFO
    //int fd = open("fifo_test", O_RDONLY);
    int fd = open("fifo_test", O_RDWR);
    char buf[100] = "";
    write(fd, "Ss", 2);
    read(fd, buf, 10);
    printf("fifo: %s\n", buf);

    return 0;
}
