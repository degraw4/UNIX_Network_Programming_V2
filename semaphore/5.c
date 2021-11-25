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

// sem_open打开已存在信号量时，oflag位可以指定为O_CREAT，不必一定为0

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main(int argc, char* argv[])
{
    sem_t *sem = sem_open("sem", O_CREAT | O_EXCL, FILE_MODE, 4);
    if(sem != SEM_FAILED){
        printf("sem create over\n");
    }
    sem_close(sem);

    // sem = sem_open("sem", O_CREAT);
    sem = sem_open("sem", 0);
    if(sem != SEM_FAILED){
        printf("sem open over\n");
    }
    sem_close(sem);

    return 0;
}
