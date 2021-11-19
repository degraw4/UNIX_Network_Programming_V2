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

// Linux中管道默认为半双工，读写同一端会EBADF

int main(int argc, char* argv[])
{
    int fd[2];
    pipe(fd);
    int ret = write(fd[0], "d",2);
    if(ret == -1){
        perror("write");
    }

    return 0;
}
