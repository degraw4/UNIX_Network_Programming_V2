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

// 多个生产者和消费者
    // 此处的empty和full信号量可以由条件变量和nready取代

#define NITEM 1000
#define NBUF 10
#define NTHREAD 10
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int buf[NBUF];
int nput = 0, nval = 0;
sem_t mutex, empty, full;

void *producer(void* argv){
    while(1){
        sem_wait(&empty);
        sem_wait(&mutex);
        if(nput >= NITEM){
            // 生产结束后要取消占有的信号量，否则当生产者线程比NBUF多时，会有NTHREAD-NBUF个线程阻塞无法终止
            sem_post(&mutex);
            sem_post(&full);
            return NULL;
        }
        buf[nput % NBUF] = nval;
        nput++;
        nval++;
        sem_post(&mutex);
        sem_post(&full);
        *(int*)argv += 1;
    }
    return NULL;
}

void *consumer(void* argv){
    for(int i = 0; i < NITEM; ++i){
        sem_wait(&full);
        sem_wait(&mutex);
        if(buf[i % NBUF] != i)
            printf("buf[%d] is %d\n", i % NBUF, buf[i % NBUF]);
        sem_post(&mutex);
        sem_post(&empty);
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t pro[NTHREAD], con;
    int count[NTHREAD];
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, NBUF);
    sem_init(&full, 0, 0);

    for(int i = 0; i< NTHREAD; ++i){
        count[i] = 0;
        pthread_create(&pro[i], NULL, producer, (void*)&count[i]);
    }
    pthread_create(&con, NULL, consumer, NULL);

    for(int i = 0; i< NTHREAD; ++i){
        pthread_join(pro[i], NULL);
    }
    pthread_join(con, NULL);

    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}
