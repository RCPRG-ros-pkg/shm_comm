#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#include "shm_comm/shm_channel.h"
#include "shm_comm.h"

#define TRUE 1
#define FALSE 0

void printUsage() {
    printf("usage: shm_comm_diag channel_name\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printUsage();
        return 0;
    }

    char *shm_name = argv[1];
//    shm_writer_t *ret;

//    ret = malloc (sizeof(shm_writer_t));

//    if (ret == NULL)
//    {
//        printf("could not allocate memory for shm_writer_t\n");
//        return -1;
//    }

    int hdr_fd;
    int data_fd;
//    channel_t channel;

    channel_hdr_t *shm_hdr;
    void *shm_data;

    char name_hdr[NAME_LEN + 6];
    strcpy (name_hdr, shm_name);
    strcat (name_hdr, "_hdr");

    char name_data[NAME_LEN + 6];
    strcpy (name_data, shm_name);
    strcat (name_data, "_data");

    hdr_fd = shm_open (name_hdr, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (hdr_fd < 0)
    {
//        free (ret);
        printf("could not open channel header\n");
        return SHM_NO_CHANNEL;
    }

    data_fd = shm_open (name_data, O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
    if (data_fd < 0)
    {
        close (hdr_fd);
//        free (ret);
        printf("could not open channel data\n");
        return SHM_NO_CHANNEL;
    }

    struct stat sb;
    fstat (hdr_fd, &sb);

    shm_hdr = mmap (NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, hdr_fd, 0);

    if (shm_hdr == MAP_FAILED)
    {
        close (hdr_fd);
        close (data_fd);
//        free (ret);
        printf("could not mmap channel header\n");
        return SHM_FATAL;
    }

    // check header channel consistency
    if (CHANNEL_HDR_SIZE(shm_hdr->max_readers) != sb.st_size) {
        close (hdr_fd);
        close (data_fd);
//        free (ret);
        printf("channel header is inconsistent: CHANNEL_HDR_SIZE(shm_hdr->max_readers) != sb.st_size\n");
        return SHM_CHANNEL_INCONSISTENT;
    }

    // check data channel consistency
    struct stat sb_data;
    fstat (data_fd, &sb_data);
    if (CHANNEL_DATA_SIZE(shm_hdr->size, shm_hdr->max_readers) != sb_data.st_size) {
        close (hdr_fd);
        close (data_fd);
//        free (ret);
        printf("channel data is inconsistent: CHANNEL_DATA_SIZE(shm_hdr->size, shm_hdr->max_readers) != sb_data.st_size\n");
        return SHM_CHANNEL_INCONSISTENT;
    }

    shm_data = mmap (NULL, CHANNEL_DATA_SIZE(shm_hdr->size, shm_hdr->max_readers), PROT_READ | PROT_WRITE, MAP_SHARED,
      data_fd, 0);

    if (shm_data == MAP_FAILED)
    {
        munmap (shm_hdr, sb.st_size);
        close (hdr_fd);
        close (data_fd);
        printf("could not mmap channel data\n");
        return SHM_FATAL;
    }

    // check channel header
    printf("trying to lock hdr mutex...\n");
    int ret = robust_mutex_lock(&shm_hdr->mtx);
    if (ret != 0) {
        printf("could not lock header mutex\n");
        munmap (shm_data, CHANNEL_DATA_SIZE(shm_hdr->size, shm_hdr->max_readers));
        munmap (shm_hdr, sb.st_size);
        close (hdr_fd);
        close (data_fd);
        return ret;
    }
    printf("hdr mutex locked\n");

    printf("shm_hdr->latest: %d\n", shm_hdr->latest);
    printf("shm_hdr->max_readers: %d\n", shm_hdr->max_readers);
    printf("shm_hdr->size: %u\n", shm_hdr->size);

    channel_t chan;
    chan.hdr = shm_hdr;
    chan.reader_ids = (pthread_mutex_t *) ((char*) shm_hdr + sizeof(channel_hdr_t));
    chan.reading = (int *) ((char *) shm_hdr + sizeof(channel_hdr_t) + shm_hdr->max_readers * sizeof(pthread_mutex_t));
    chan.buffer = shm_data;


  for (int i = 0; i < chan.hdr->max_readers; i++)
  {
      int lock_status = pthread_mutex_trylock(&chan.reader_ids[i]);
      int acquired = FALSE;
      switch (lock_status)
      {
      case 0:
        acquired = TRUE;
        printf("acquired reader mutex for i=%d\n", i);
        break;
      case EBUSY:
        // do nothing:
        // other reader acquired the mutex
        // the loop should be continued
        printf("mutex for i=%d is already acquired\n", i);
        break;
      case EINVAL:
        printf("mutex for i=%d error: EINVAL\n", i);
        break;
      case EAGAIN:
        printf("mutex for i=%d error: EAGAIN\n", i);
        break;
      case EDEADLK:
        printf("mutex for i=%d error: EDEADLK\n", i);
        break;
      case EOWNERDEAD:
        // the reader that acquired the mutex is dead
        //TODO: make the state consistent
        printf("mutex for i=%d error: EOWNERDEAD (mutex is locked and owner is dead)\n", i);
        break;
      default:
        // other error
        printf("mutex for i=%d error: UNKNOWN\n", i);
        break;
      }

      if (acquired)
      {
        pthread_mutex_unlock(&chan.reader_ids[i]);
        printf("released reader mutex for i=%d\n", i);
      }

      printf("reading[%d]: %d\n", i, chan.reading[i]);
  }




    pthread_mutex_unlock(&shm_hdr->mtx);
    printf("hdr mutex unlocked\n");

    // cleanup
    munmap (shm_data, CHANNEL_DATA_SIZE(shm_hdr->size, shm_hdr->max_readers));
//    writer->channel.buffer = NULL;
    close (data_fd);
    data_fd = 0;

    munmap (shm_hdr, sb.st_size);
//    writer->channel.hdr = NULL;
//    writer->channel.reader_ids = NULL;
//    writer->channel.reading = NULL;
    close (hdr_fd);
    hdr_fd = 0;
//    free (writer);

    printf("OK\n");

    return 0;
}

