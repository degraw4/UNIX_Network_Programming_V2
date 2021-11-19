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

// S_ISFIFO

int main(int argc, char* argv[])
{
    int fd[2];
    pipe(fd);
    struct stat sbuf;
    fstat(fd[0], &sbuf);
    if(S_ISFIFO(sbuf.st_mode)){
        printf("type is FIFO\n");
    }

    return 0;
}
