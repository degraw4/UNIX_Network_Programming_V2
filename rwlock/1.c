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

// mutex和cond实现rwlock

typedef struct{
    pthread_mutex_t mutex;
    pthread_cond_t cond_readers;
    pthread_cond_t cond_writers;
    int reader_wait;
    int writer_wait;
    int lock;
} pthread_rwlock_t;

typedef int pthread_rwlockattr_t;

int pthread_rwlock_init(pthread_rwlock_t *rw, pthread_rwlockattr_t *attr){
    pthread_mutex_init(&rw->lock, NULL);
    pthread_cond_init(&rw->cond_readers, NULL);
    pthread_cond_init(&rw->cond_writers, NULL);
    rw->reader_wait = 0;
    rw->writer_wait = 0;
    rw->lock = 0;
    return 0;
}

int pthread_rwlock_destroy(pthread_rwlock_t *rw){
    if(rw->lock != 0 || rw->reader_wait != 0 || rw->writer_wait != 0)
        return EBUSY;
    pthread_mutex_destroy(&rw->mutex);
    pthread_cond_destroy(&rw->cond_readers);
    pthread_cond_destroy(&rw->cond_writers);
    return 0;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rw){
    pthread_mutex_lock(&rw->mutex);
    while(rw->lock < 0 || rw->writer_wait > 0){
        rw->reader_wait++;
        pthread_cond_wait(&rw->cond_readers, &rw->mutex);
        rw->reader_wait--;
    }
    rw->lock++;
    pthread_mutex_unlock(&rw->mutex);
    return 0;
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rw){
    pthread_mutex_lock(&rw->mutex);
    if(rw->lock < 0 || rw->writer_wait > 0)
        return EBUSY;
    rw->lock++;
    pthread_mutex_unlock(&rw->mutex);
    return 0;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rw){
    pthread_mutex_lock(&rw->mutex);
    while(rw->lock != 0){
        rw->writer_wait++;
        pthread_cond_wait(&rw->cond_writers, &rw->mutex);
        rw->writer_wait--;
    }
    rw->lock = -1;
    pthread_mutex_unlock(&rw->mutex);
    return 0;
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *rw){
    pthread_mutex_lock(&rw->mutex);
    if(rw->lock != 0)
        return EBUSY;
    rw->lock = -1;
    pthread_mutex_unlock(&rw->mutex);
    return 0;
}

int pthread_rwlock_unlock(pthread_rwlock_t *rw){
    pthread_mutex_lock(&rw->mutex);
    if(rw->lock == -1)
        rw->lock = 0;
    else if(rw->lock > 0)
        rw->lock --;
    
    if(rw->writer_wait > 0)
        if(rw->lock == 0)
            // 两个if不能合成一个，否则有writer wait且为读锁时，直接跳到else岛主writer饥饿
            // 检验rw->lock是为了防止存在reader lock时的无效signal
            pthread_cond_signal(&rw->cond_writers);
    else if(rw->reader_wait > 0)
        pthread_cond_broadcast(&rw->cond_writers);
        
    pthread_mutex_unlock(&rw->mutex);
    return 0;
}

int main(int argc, char* argv[])
{
    

    return 0;
}
