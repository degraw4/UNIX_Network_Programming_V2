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

// 单个生产者和消费者

#define NITEM 1000
#define NBUF 10
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int buf[NBUF];
sem_t *mutex, *empty, *full;

void *producer(void* argv){
    for(int i = 0; i < NITEM; ++i){
        sem_wait(empty);
        sem_wait(mutex);
        buf[i % NBUF] = i;
        sem_post(mutex);
        sem_post(full);
    }
    return NULL;
}

void *consumer(void* argv){
    for(int i = 0; i < NITEM; ++i){
        sem_wait(full);
        sem_wait(mutex);
        if(buf[i % NBUF] != i)
            printf("buf[%d] is %d\n", i % NBUF, buf[i % NBUF]);
        sem_post(mutex);
        sem_post(empty);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t pro, con;
    mutex = sem_open("mutex", O_CREAT | O_EXCL, FILE_MODE, 1);
    empty = sem_open("empty", O_CREAT | O_EXCL, FILE_MODE, NBUF);
    full = sem_open("full", O_CREAT | O_EXCL, FILE_MODE, 0);

    pthread_create(&pro, NULL, producer, NULL);
    pthread_create(&con, NULL, consumer, NULL);

    pthread_join(pro, NULL);
    pthread_join(con, NULL);

    sem_unlink("mutex");
    sem_unlink("empty");
    sem_unlink("full");

    return 0;
}
