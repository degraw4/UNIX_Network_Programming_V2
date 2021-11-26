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

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH)

int main(int argc, char* argv[])
{
    // shm只能只读或者读写
    int fd = shm_open("shm_test", O_CREAT | O_RDWR | O_EXCL, FILE_MODE);
    shm_unlink("shm_test");

    ftruncate(fd, 1024);
    int *ptr = mmap(fd, 1024, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    return 0;
}
