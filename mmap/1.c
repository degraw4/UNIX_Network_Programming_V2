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

// 内存映射+1 with 有名信号量
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH)

int main(int argc, char* argv[])
{
    sem_t *sem;
    int *ptr, zero = 0;
    int fd = open("file", O_CREAT | O_RDWR, FILE_MODE);
    write(fd, &zero, sizeof(int));
    ptr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    sem = sem_open("sem_test", O_CREAT | O_EXCL, FILE_MODE, 1);
    // 防止忘记unlink，先提前操作，但会延迟unlink
    sem_unlink("sem_test");
    // 设置无缓冲
    setbuf(stdout, NULL);

    if(fork() == 0){
        for(int i = 0; i < 1000; ++i){
            sem_wait(sem);
            printf("child: %d\n", (*ptr)++);
            sem_post(sem);
        }
        exit(0);
    }    
    for(int i = 0; i < 1000; ++i){
        sem_wait(sem);
        printf("parent: %d\n", (*ptr)++);
        sem_post(sem);
    }

    return 0;
}
