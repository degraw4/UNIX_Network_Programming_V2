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

// mutex + cond

struct {
    pthread_mutex_t mutex;
    int buf[NITEM];
    int nput;
    int nval;
}put = {
    PTHREAD_MUTEX_INITIALIZER
};

struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int nready;     // 当前可以被消费的数目
}ready = {
    PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER
};

void *producer(void*);
void *consumer(void*);

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
        pthread_mutex_lock(&put.mutex);
        if(put.nput >= NITEM){
            pthread_mutex_unlock(&put.mutex);
            return NULL;
        }
        put.buf[put.nput] = put.nval;
        put.nput++;
        put.nval++;
        pthread_mutex_unlock(&put.mutex);

        pthread_mutex_lock(&ready.mutex);
        if(ready.nready == 0){
            pthread_cond_signal(&ready.cond);
        }
        ready.nready++;
        pthread_mutex_unlock(&ready.mutex);

        *(int*)argv += 1;
    }
}

void *consumer(void* argv){
    for(int i = 0; i < NITEM; i++){
        pthread_mutex_lock(&ready.mutex);
        while(ready.nready == 0)
            pthread_cond_wait(&ready.cond, &ready.mutex);
        ready.nready--;
        pthread_mutex_unlock(&ready.mutex);

        if(put.buf[i] != i)
            printf("buff[%d] is %d\n", i, put.buf[i]);
    }
    return NULL;
}
