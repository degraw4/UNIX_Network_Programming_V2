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

// 多个生产者和多个消费者

#define NITEM 1000
#define NBUF 10
#define NTHREAD 10
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int buf[NBUF];
int nput = 0, nputval = 0;
int nget = 0, ngetval = 0;
sem_t mutex, empty, full;

void *producer(void* argv){
    while(1){
        sem_wait(&empty);
        sem_wait(&mutex);
        if(nput >= NITEM){
            // 当全部生产和消费完，还没开始下一轮while循环时，empty和full和初始一样，为NBUF和0
            // 因此生产者wait empty会成功然后退出，但是消费者wait full会阻塞
            // 因此生产者退出时要post full一下，使得消费者可以退出
            // 且只要一个生产者post full，全部消费者都可以退出
            sem_post(&full);
            // 生产结束后要取消占有的信号量，否则当生产者线程比NBUF多时，会有NTHREAD-NBUF个线程阻塞无法终止
            sem_post(&mutex);
            sem_post(&empty);
            return NULL;
        }
        buf[nput % NBUF] = nputval;
        nput++;
        nputval++;
        sem_post(&mutex);
        sem_post(&full);
        *(int*)argv += 1;
    }
    return NULL;
}

void *consumer(void* argv){
    while(1){
        sem_wait(&full);
        sem_wait(&mutex);
        if(nget >= NITEM){
            sem_post(&mutex);
            sem_post(&full);
            return NULL;
        }
        if(buf[nget % NBUF] != ngetval)
            printf("buf[%d] is %d\n", nget % NBUF, buf[nget % NBUF]);
        nget++;
        ngetval++;
        sem_post(&mutex);
        sem_post(&empty);
        *(int*)argv += 1;
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    pthread_t pro[NTHREAD], con[NTHREAD];
    int procount[NTHREAD], concount[NTHREAD];
    sem_init(&mutex, 0, 1);
    sem_init(&empty, 0, NBUF);
    sem_init(&full, 0, 0);

    for(int i = 0; i< NTHREAD; ++i){
        procount[i] = 0;
        pthread_create(&pro[i], NULL, producer, (void*)&procount[i]);
    }
    for(int i = 0; i< NTHREAD; ++i){
        concount[i] = 0;
        pthread_create(&con[i], NULL, consumer, (void*)&concount[i]);
    }

    for(int i = 0; i< NTHREAD; ++i){
        pthread_join(pro[i], NULL);
        printf("pro count[%d] is %d\n", i, procount[i]);
    }
    for(int i = 0; i< NTHREAD; ++i){
        pthread_join(con[i], NULL);
        printf("con count[%d] is %d\n", i, concount[i]);
    }

    sem_destroy(&mutex);
    sem_destroy(&empty);
    sem_destroy(&full);

    return 0;
}
