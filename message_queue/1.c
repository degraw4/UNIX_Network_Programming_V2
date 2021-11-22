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

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

// link with lrt

int main(int argc, char* argv[])
{
    mqd_t queue = mq_open("mq", O_RDWR, FILE_MODE, NULL);
    mq_unlink("mq");
    queue = mq_open("mq", O_RDWR);
    // mq_unlink后name已被删除，无法再次使用
    if(queue = -1){
        printf("open 2 error\n");
    }
    mq_close(queue);

    queue = mq_open("mq", O_RDWR);
    if(queue = -1){
        printf("open 3 error\n");
    }

    queue = mq_open("mq", O_RDWR, FILE_MODE, NULL);
    if(queue = -1){
        printf("open 4 error\n");
    }

    return 0;
}
