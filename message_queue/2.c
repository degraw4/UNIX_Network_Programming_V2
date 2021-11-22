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

int main(int argc, char* argv[])
{
    mqd_t mq = mq_open("mqueue_test", O_RDWR, FILE_MODE, NULL);
    struct mq_attr attr;
    mq_getattr(mq, &attr);
    printf("msg number %ld\n", attr.mq_curmsgs);

    char* buf1 = (char*)calloc(attr.mq_maxmsg, sizeof(char));
    char* buf2 = (char*)malloc(attr.mq_maxmsg);
    int prio = -1;
    mq_send(mq, buf1, attr.mq_maxmsg, 3);
    int len = mq_receive(mq, buf2, attr.mq_maxmsg, &prio);
    printf("receive %s\nlen is %d\npriority is %d\n", buf2, len, prio);
    pause();

    return 0;
}
