#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define TRUE 1
#define FALSE 0

typedef struct {
	pthread_cond_t cond;
	pthread_mutex_t mtx;
    int predicate;
} channel_hdr_t;

typedef struct {
    int fd;
    channel_hdr_t *hdr;
} channel_t;

void printUsage() {
    printf("usage: shm_comm_test2 channel_name1 channel_name2\n");
}

int robust_mutex_lock(pthread_mutex_t *mutex) {
  // lock hdr mutex in the safe way
  int lock_status = pthread_mutex_lock (mutex);
  int acquired = FALSE;
  int err = -18;
  switch (lock_status)
  {
  case 0:
    acquired = TRUE;
    break;
  case EINVAL:
    printf("**** EINVAL ****\n");
    err = -12;
    break;
  case EAGAIN:
    printf("**** EAGAIN ****\n");
    err = -13;
    break;
  case EDEADLK:
    printf("**** EDEADLK ****\n");
    err = -14;
    break;
  case EOWNERDEAD:
    // the reader that acquired the mutex is dead
    printf("**** EOWNERDEAD ****\n");

    // recover the mutex
    if (pthread_mutex_consistent(mutex) == EINVAL) {
    printf("**** EOWNERDEAD, EINVAL ****\n");
      err = -15;
      break;
    }
    acquired = TRUE;
    break;
  default:
    printf("**** OTHER ****\n");
    // other error
    err = -18;
    break;
  }

  return acquired ? 0 : err;
}

int init_channel(char *shm_name, channel_t *out) {
    int initialize = FALSE;

    int shm_fd = shm_open (shm_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (shm_fd < 0) {
        if (errno == EEXIST) {
            // open again, do not initialize
            shm_fd = shm_open (shm_name, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
            if (shm_fd < 0) {
                printf( "ERROR: could not create %s, errno: %d\n", shm_name, errno );
                return 1;
            }
        }
        else {
            printf( "ERROR: could not create %s, errno: %d\n", shm_name, errno );
            return 2;
        }
    }
    else {
        // the shm object was created, so initialize it
        initialize = TRUE;

        printf("created shm object %s\n", shm_name);
        if (ftruncate (shm_fd, sizeof(channel_hdr_t)) != 0)
        {
            printf( "ERROR: could not ftruncate %s, errno: %d\n", shm_name, errno );
            close (shm_fd);
            shm_unlink (shm_name);
            return 3;
        }
    }

    void *ptr_shm_hdr = mmap (NULL, sizeof(channel_hdr_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (ptr_shm_hdr == MAP_FAILED)
    {
        printf( "ERROR: could not mmap %s, errno: %d\n", shm_name, errno );
        close (shm_fd);
        shm_unlink (shm_name);
        return 4;
    }

    channel_hdr_t *shm_hdr = ptr_shm_hdr;

    if (initialize) {
        // set mutex shared between processes
        pthread_mutexattr_t mutex_attr;
        pthread_mutexattr_init(&mutex_attr);
        pthread_mutexattr_setpshared (&mutex_attr, PTHREAD_PROCESS_SHARED);
        pthread_mutexattr_setrobust (&mutex_attr, PTHREAD_MUTEX_ROBUST);
        pthread_mutexattr_setprotocol(&mutex_attr, PTHREAD_PRIO_INHERIT);

        pthread_mutex_init (&shm_hdr->mtx, &mutex_attr);

        // set condition shared between processes
        pthread_condattr_t cond_attr;
        pthread_condattr_init(&cond_attr);
        pthread_condattr_setpshared (&cond_attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init (&shm_hdr->cond, &cond_attr);
    }

    shm_hdr->predicate = 0;
    out->fd = shm_fd;
    out->hdr = shm_hdr;

    return 0;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printUsage();
        return 0;
    }

    char *shm_1_name = argv[1];
    char *shm_2_name = argv[2];

    channel_t ch_1;
    if (init_channel(shm_1_name, &ch_1) != 0) {
        return 1;
    }

    channel_t ch_2;
    if (init_channel(shm_2_name, &ch_2) != 0) {
        munmap( ch_1.hdr, sizeof(channel_hdr_t) );
        close( ch_1.fd );
        return 2;
    }

    int counter = 0;
    int counter2 = 0;
    while (TRUE) {
        ++counter;
        if (counter == 100000) {
            printf("alive %d\n", counter2);
            ++counter2;
            counter = 0;
        }
        int ret = robust_mutex_lock(&ch_1.hdr->mtx);
        if (ret != 0) {
            return ret;
        }
        ch_1.hdr->predicate = 1;
        pthread_cond_broadcast (&ch_1.hdr->cond);     // deadlock here
        pthread_mutex_unlock (&ch_1.hdr->mtx);



        ret = robust_mutex_lock(&ch_2.hdr->mtx);
        if (ret != 0) {
            return ret;
        }

        while (ch_2.hdr->predicate == 0 && ret == 0)
        {
            ret = pthread_cond_wait (&ch_2.hdr->cond, &ch_2.hdr->mtx);  // deadlock here
        }
        ch_2.hdr->predicate = 0;
        pthread_mutex_unlock (&ch_2.hdr->mtx);
    }

    munmap( ch_1.hdr, sizeof(channel_hdr_t) );
    close( ch_1.fd );

    munmap( ch_2.hdr, sizeof(channel_hdr_t) );
    close( ch_2.fd );

    return 0;
}

