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

// 内存映射+1 with 内存信号量
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IROTH)

struct share{
    sem_t sem;
    int count;
}shared;

int main(int argc, char* argv[])
{
    struct share *ptr = NULL;
    int fd = open("file", O_CREAT | O_RDWR, FILE_MODE);
    write(fd, &shared, sizeof(struct share));
    ptr = (struct share*)mmap(NULL, sizeof(struct share), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    // 初始化, sem要设为1来保证进程间共享
    ptr->count = 0;
    sem_init(&ptr->sem, 1, 1);
    // 设置无缓冲
    setbuf(stdout, NULL);

    if(fork() == 0){
        for(int i = 0; i < 1000; ++i){
            sem_wait(&ptr->sem);
            printf("child: %d\n", ptr->count++);
            sem_post(&ptr->sem);
        }
        exit(0);
    }    
    for(int i = 0; i < 1000; ++i){
        sem_wait(&ptr->sem);
        printf("parent: %d\n", ptr->count++);
        sem_post(&ptr->sem);
    }

    return 0;
}
