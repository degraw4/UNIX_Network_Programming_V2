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

// 线程cond_wait时被取消，会自动上锁，然后调用清理函数
// 因此在wait前后加上push和pop，pop(0)为wait成功返回后取消清理函数

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void clean_wait(void *argv){
    pthread_mutex_unlock((pthread_mutex_t *)argv);
}

void *thread(void *argv){
    pthread_cleanup_push(clean_wait, (void*)&mutex);
    pthread_cond_wait(&cond, &mutex);
    pthread_cleanup_pop(0); 
    return NULL;
}

int main(int argc, char* argv[])
{
    

    return 0;
}
