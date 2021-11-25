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

// 共享内存实现Posix有名信号量

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    unsigned int value;
    int magic;
} sem_t;

#define SME_MAGIC 0x67458923
#define MAX_TRIES 10
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

#ifndef SEM_FAILED
#define SEM_FAILED ((sem_t*)(-1))
#endif

sem_t *sem_open(const char *name, int oflag, mode_t mode, unsigned int value){
    int fd, created = 0, save_errno, error;
    sem_t *sem, sem_init;
    struct stat statbuff;
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;

    if((oflag != 0) && (oflag | O_CREAT)){
        // user指定了O_CREAT
        // 对新创建的semaphore开启S_IXUSR，初始化结束后再关闭S_IXUSR，避免create和open进程的竞争状态
        printf("oflag %d\n", oflag);
        printf("oflag | O_CREAT %d\n", oflag | O_CREAT);
        mode &= ~S_IXUSR;
        // 专门指定O_EXCL打开，开启S_IXUSR
        fd = open(name, O_CREAT | O_EXCL | O_RDWR, mode | S_IXUSR);
        if(fd < 0){
            // 已存在且未指定O_EXCL，则为打开
            if(errno == EEXIST && (oflag & O_EXCL) == 0)
                goto exists;
            // 否则为失败
            else
                return SEM_FAILED;
        }
        created = 1;
        // 初始化文件
        bzero(&sem_init, sizeof(sem_t));
        if(write(fd, &sem_init, sizeof(sem_t)) != sizeof(sem_t))
            goto err;
        // mmap
        sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(sem == MAP_FAILED)
            goto err;
        // mutex & cond
        if(pthread_mutexattr_init(&mattr) != 0)
            goto err;
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
        error = pthread_mutex_init(&sem->mutex, &mattr);
        pthread_mutexattr_destroy(&mattr);
        if(error != 0)
            goto err;
        if(pthread_condattr_init(&cattr) != 0)
            goto err;
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
        error = pthread_cond_init(&sem->cond, &cattr);
        pthread_condattr_destroy(&cattr);
        if(error != 0)
            goto err;
        
        sem->value = value;
        // 初始化结束，关闭S_IXUSR，使得open的进程可以打开
        if(fchmod(fd, mode) == -1)
            goto err;
        close(fd);
        sem->magic = SME_MAGIC;
        return sem;
    }       

    // 未指定O_CREAT or O_CREAT创建时发现已存在
    exists:
        if(fd = open(name, O_RDWR) < 0)
            goto err;
        sem = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if(sem == MAP_FAILED)
            goto err;
        // 确保create后初始化完毕再open
        for(int i = 0; i < MAX_TRIES; ++i){
            // 使用stat而不是fstat
            if(stat(name, &statbuff) == -1)
                goto err;
            if((statbuff.st_mode & S_IXUSR) == 0){
                close(fd);
                sem->magic = SME_MAGIC;
                return (sem);
            }
            sleep(1);
        }
        errno = ETIMEDOUT;
        goto err;
    err:
        // 防止munmap或者close改变errno
        save_errno = errno;
        if(created)
            unlink(name);
        if(sem != MAP_FAILED)
            munmap(sem, sizeof(sem_t));
        close(fd);
        errno = save_errno;
        return (SEM_FAILED);
}

int sem_close(sem_t *sem){
    if(sem->magic != SME_MAGIC){
        errno = EINVAL;
        return -1;
    }
    if(munmap(sem, sizeof(sem_t)) == -1)
        return -1;
    return 0;
}

int sem_unlink(const char *name){
    if(unlink(name) == -1)
        return -1;
    return 0;
}

int sem_post(sem_t *sem){
    int n;
    if(sem->magic != SME_MAGIC){
        errno = EINVAL;
        return -1;
    }
    if((n == pthread_mutex_lock(&sem->mutex)) != 0){
        errno = n;
        return -1;
    }
    if(sem->value == 0)
        pthread_cond_signal(&sem->cond);
    sem->value++;
    pthread_mutex_unlock(&sem->mutex);
    return 0;
}

int sem_wait(sem_t *sem){
    int n;
    if(sem->magic != SME_MAGIC){
        errno = EINVAL;
        return -1;
    }
    if((n == pthread_mutex_lock(&sem->mutex)) != 0){
        errno = n;
        return -1;
    }
    // while
    while(sem->value == 0)
        pthread_cond_wait(&sem->cond, &sem->mutex);
    sem->value--;
    pthread_mutex_unlock(&sem->mutex);
    return 0;
}

int sem_trywait(sem_t *sem){
    int n, result;
    if(sem->magic != SME_MAGIC){
        errno = EINVAL;
        return -1;
    }
    if((n == pthread_mutex_lock(&sem->mutex)) != 0){
        errno = n;
        return -1;
    }
    if(sem->value > 0){
        sem->value--;
        result = 0;
    }
    else{
        errno = EAGAIN;
        result = -1;
    }
    pthread_mutex_unlock(&sem->mutex);
    return result;
}

int sem_getvalue(sem_t *sem, int *result){
    int n;
    if(sem->magic != SME_MAGIC){
        errno = EINVAL;
        return -1;
    }
    if((n == pthread_mutex_lock(&sem->mutex)) != 0){
        errno = n;
        return -1;
    }
    *result = sem->value;
    pthread_mutex_unlock(&sem->mutex);
    return 0;
}

int main(int argc, char* argv[])
{
    sem_t *sem = sem_open("test", O_CREAT | O_EXCL, FILE_MODE, 1);
    if(sem == SEM_FAILED){
        printf("sem create failed\n");
    }
    int value, ret;
    sem_getvalue(sem, &value);
    printf("value is %d\n", value);

    ret = sem_wait(sem);
    if(ret == -1){
        printf("sem wait failed\n");
    }

    ret = sem_trywait(sem);
    if(ret == -1){
        printf("sem trywait failed\n");
    }

    ret = sem_post(sem);
    if(ret == -1){
        printf("sem post failed\n");
    }

    ret = sem_close(sem);
    if(ret == -1){
        printf("sem close failed\n");
    }

    sem = sem_open("test", 0, FILE_MODE, 1);
    if(sem == SEM_FAILED){
        printf("sem open failed\n");
    }

    ret = sem_close(sem);
    if(ret == -1){
        printf("sem close failed\n");
    }

    ret = sem_unlink("test");
    if(ret == -1){
        printf("sem unlink failed\n");
    }

    return 0;
}
