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

// 测试持有写锁的情况下，等待的读锁和写锁谁更优先
// linux下测试，不管读锁和写锁谁先等待，都是先启动写锁

int lock_reg(int fd, int cmd, int type, off_t offset, int whence, off_t len){
    struct flock lock;
    lock.l_type = type;
    lock.l_start = offset;
    lock.l_whence = whence;
    lock.l_len = len;
    return fcntl(fd, cmd, &lock);
}

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#define read_lock(fd, offset, whence, len)      lock_reg(fd, F_SETLK, F_RDLCK, (offset), (whence), (len))
#define readw_lock(fd, offset, whence, len)     lock_reg(fd, F_SETLKW, F_RDLCK, (offset), (whence), (len))
#define write_lock(fd, offset, whence, len)     lock_reg(fd, F_SETLK, F_WRLCK, (offset), (whence), (len))
#define writew_lock(fd, offset, whence, len)    lock_reg(fd, F_SETLKW, F_WRLCK, (offset), (whence), (len))
#define un_lock(fd, offset, whence, len)        lock_reg(fd, F_SETLK, F_UNLCK, (offset), (whence), (len))

int main(int argc, char* argv[])
{
    int fd = open("file", O_RDWR, FILE_MODE);
    write_lock(fd, 0, SEEK_SET, 0);
    printf("parent get read lock\n");

    if(fork() == 0){
        // first child
        sleep(3);
        printf("first child try to wrlock\n");
        writew_lock(fd, 0, SEEK_SET, 0);
        printf("first child get write lock\n");
        sleep(4);
        un_lock(fd, 0, SEEK_SET, 0);
        printf("first child unlock\n");
        exit(0);
    }

    if(fork() == 0){
        // second child
        sleep(1);
        printf("second child try to rdlock\n");
        readw_lock(fd, 0, SEEK_SET, 0);
        printf("second child get read lock\n");
        sleep(2);
        un_lock(fd, 0, SEEK_SET, 0);
        printf("second child unlock\n");
        exit(0);
    }

    sleep(5);
    un_lock(fd, 0, SEEK_SET, 0);
    printf("parent unlock\n");

    return 0;
}
