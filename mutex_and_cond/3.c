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
#include <errno.h>
#include <mqueue.h>

#define NITEM 10000
#define NTHREAD 10

// 生产者和消费者之间的轮询同步等待

struct {
    pthread_mutex_t mutex;
    int buf[NITEM];
    int nput;
    int nval;
}shared = {
    PTHREAD_MUTEX_INITIALIZER
};

void *producer(void*);
void *consumer(void*);
void consume_wait(int);

int main(int argc, char* argv[])
{
    int count[NTHREAD];
    pthread_t tid[NTHREAD], tid_c;

    for(int i = 0; i < NTHREAD; ++i){
        count[i] = 0;
        pthread_create(&tid[i], NULL, producer, (void*)&count[i]);
    }
    pthread_create(&tid_c, NULL, consumer, NULL);

    for(int i = 0; i < NTHREAD; ++i){
        pthread_join(tid[i], NULL);
        printf("count[%d] = %d\n", i, count[i]);
    }
    pthread_join(tid_c, NULL);

    return 0;
}

void *producer(void* argv){
    while(1){
        pthread_mutex_lock(&shared.mutex);
        if(shared.nput >= NITEM){
            pthread_mutex_unlock(&shared.mutex);
            return NULL;
        }
        shared.buf[shared.nput] = shared.nval;
        shared.nput++;
        shared.nval++;
        pthread_mutex_unlock(&shared.mutex);
        *(int*)argv += 1;
    }
}

void consume_wait(int i){
    while(1){
        pthread_mutex_lock(&shared.mutex);
        if(i < shared.nput){
            pthread_mutex_unlock(&shared.mutex);
            return;
        }
        pthread_mutex_unlock(&shared.mutex);
    }
}

void *consumer(void* argv){
    for(int i = 0; i < NITEM; i++){
        consume_wait(i);
        if(shared.buf[i] != i)
            printf("buff[%d] is %d\n", i, shared.buf[i]);
    }
    return NULL;
}
