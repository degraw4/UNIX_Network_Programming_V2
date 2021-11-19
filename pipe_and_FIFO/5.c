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

// 对于FIFO，当写端全部关闭的时候，read端会返回0

int main(int argc, char* argv[])
{
    if(fork() > 0){
        int fd = open("fifo_test", O_RDWR);
        char buf[1024] = "";
        int ret;
        if(ret = (read(fd, buf, 10)) > 0)
            printf("read 1 get\n");
        else    
            printf("read 1: %d\n", ret);
        if(ret = (read(fd, buf, 10)) > 0)
            printf("read 2 get\n");
        else    
            printf("read 2: %d\n", ret);
    }
    else{
        int fd = open("fifo_test", O_WRONLY);
        write(fd, "test", 4);
        exit(0);
    }

    return 0;
}
