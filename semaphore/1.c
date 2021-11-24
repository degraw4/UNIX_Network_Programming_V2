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

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main(int argc, char* argv[])
{
    // 创建信号量
    sem_t *sem = sem_open("sem_test", O_CREAT | O_EXCL, FILE_MODE, 2);
    // 打开信号量
    //sem_t *sem = sem_open("sem_test", 0);

    if(sem == SEM_FAILED)
        printf("error\n");

    int val;
    sem_wait(sem);
    sem_getvalue(sem, &val);
    printf("value is %d\n", val);
    sem_wait(sem);
    sem_getvalue(sem, &val);
    printf("value is %d\n", val);

    sem_close(sem);
    sem_unlink("sem_test");

    return 0;
}
