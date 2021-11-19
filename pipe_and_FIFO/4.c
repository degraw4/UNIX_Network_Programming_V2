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

// FIFO本质上是一个缓冲区，write在尾部，read在头部
// fork后，子进程获得父进程FIFO fd的副本

int main(int argc, char* argv[])
{
    int fd = open("fifo_test", O_RDWR);
    if(fork() == 0){
        char buf[10] = "";
        read(fd, buf, 3);
        printf("child read: %s\n", buf);
    }
    else{
        write(fd, "sss", 3);
    }

    return 0;
}
