#include "shm_channel.h"

#include "shm_comm.h"

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int shm_create_channel (char name[NAME_LEN], int size, int readers)
{
  if (size < 1)
  {
    return SHM_INVAL;
  }
  
  if (readers < 1)
  {
    return SHM_INVAL;
  }
  
  printf("creating channel [%s] size: %d readers: %d \n", name, size, readers);

  char name_tmp[NAME_LEN+5];

  strcpy(name_tmp, name);
  strcat(name_tmp, "_hdr");

  int shm_hdr_fd = shm_open(name_tmp, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (shm_hdr_fd < 0) {
    fprintf(stderr, "shm_open failed\n");
    perror(NULL);
    return -1;
  }

  if (ftruncate(shm_hdr_fd, CHANNEL_HDR_SIZE(size, readers)) != 0) {
    fprintf(stderr, "ftruncate failed\n");
    shm_unlink(name_tmp);
    return -1;
  }

  void *shm_hdr = mmap(NULL, CHANNEL_HDR_SIZE(size, readers), PROT_READ | PROT_WRITE, MAP_SHARED, shm_hdr_fd, 0);

  if (shm_hdr == MAP_FAILED) {
    fprintf(stderr, "mmap failed\n");
    shm_unlink(name_tmp);
    return -1;
  }

  strcpy(name_tmp, name);
  strcat(name_tmp, "_data");

  int shm_data_fd = shm_open(name_tmp, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
  if (shm_data_fd < 0) {
    fprintf(stderr, "shm_open failed\n");
    perror(NULL);
    return -1;
  }

  if (ftruncate(shm_data_fd, CHANNEL_DATA_SIZE(size, readers)) != 0) {
    fprintf(stderr, "ftruncate failed\n");
    shm_unlink(name_tmp);
    return -1;
  }

  init_channel_hdr(size, readers, SHM_SHARED, shm_hdr);
  
  munmap(shm_hdr, CHANNEL_HDR_SIZE(size, readers));
  close(shm_hdr_fd);
  close(shm_data_fd);
  
  return 0;
}

int shm_remove_channel(char name[NAME_LEN])
{
  char shm_name_tmp[NAME_LEN+5];

  strcpy(shm_name_tmp, name);
  strcat(shm_name_tmp, "_data");
  shm_unlink(shm_name_tmp);

  strcpy(shm_name_tmp, name);
  strcat(shm_name_tmp, "_hdr");
  shm_unlink(shm_name_tmp);
  
  return 0;
}
