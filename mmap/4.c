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
#include <semaphore.h>
#include <errno.h>
#include <mqueue.h>

// mmap处理大小持续增长的文件的方法
#define SIZE 32768
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH)

int main(int argc, char* argv[])
{
    int fd = open("test", O_CREAT | O_RDWR | O_TRUNC, FILE_MODE);
    int *ptr = mmap(NULL, 0, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    for(int i = 4096; i <= SIZE; i += 4096){
        printf("set file size to %d\n", i);
        ftruncate(fd, i);
        ptr[i-1] = 0;
        printf("ptr[%d] is %d\n", i-1, ptr[i-1]);
    }

    return 0;
}
