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

// 进程和线程持有锁后退出，都不会自动释放锁

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* thread1(void* argv){
    pthread_mutex_lock(&mutex);
    printf("thread 1 get mutex, now exit\n");
    pthread_exit(0);
}

void* thread2(void* argv){
    if(pthread_mutex_trylock(&mutex) == EBUSY){
        printf("thread 2 trylock busy\n");
    }
}

int main(int argc, char* argv[])
{
    pthread_t tid1, tid2;
    pthread_create(&tid1, NULL, thread1, NULL);
    pthread_join(tid1, NULL);
    pthread_create(&tid2, NULL, thread2, NULL);
    pthread_join(tid2, NULL);

    return 0;
}
