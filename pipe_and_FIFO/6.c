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

// 空管道或空FIFO的open的阻塞
    // 读open，默认阻塞直到有写open打开FIFO，NONBLOCK直接返回
    // 写open，默认阻塞直到有读open打开FIFO，NONBLOCK返回error

int main(int argc, char* argv[])
{
    if(fork() > 0){
        int fd = open("fifo_test", O_WRONLY);
        printf("parent open over\n");
        // char buf[10] = "";
        // read(fd, buf, 10);
        // printf("parent read: %s\n", buf);
    }
    else{
        // sleep(5);
        int fd = open("fifo_test", O_WRONLY);
        printf("childt open over\n");
        // write(fd, "test", 4);
        // close(fd);
        // printf("child write over, now close fd\n");
        exit(0);
    }

    return 0;
}
