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

// 当mmap映射内存小于文件大小时，访问映射内存外的区域会SIGSEGV

int main(int argc, char* argv[])
{
    int fd = open("file", O_RDWR);
    char *ptr = mmap(NULL, 5, PROT_READ | PROT_WRITE, 0, fd, 0);
    printf("page size is %ld\n",sysconf(_SC_PAGE_SIZE));
    ptr += 5;
    *ptr = 'a';

    return 0;
}
