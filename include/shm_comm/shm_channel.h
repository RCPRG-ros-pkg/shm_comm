#ifndef __SHM_CHANNEL__
#define __SHM_CHANNEL__

#include <time.h>

#define NAME_LEN 128

struct shm_reader;
struct shm_writer;
typedef struct shm_writer shm_writer_t;
typedef struct shm_reader shm_reader_t;

int shm_create_channel (const char name[NAME_LEN], int size, int readers);
int shm_remove_channel (const char name[NAME_LEN]);

shm_writer_t *shm_connect_writer (const char name[NAME_LEN]);
int shm_release_writer (shm_writer_t *writer);
int shm_writer_buffer_get (shm_writer_t *wr, void** buf);
int shm_writer_buffer_write (shm_writer_t *wr);
int shm_writer_get_size(shm_writer_t *wr);

shm_reader_t *shm_connect_reader (const char name[NAME_LEN]);
int shm_release_reader (shm_reader_t *reader);
int shm_reader_buffer_get (shm_reader_t *reader, void **buf);
int shm_reader_buffer_wait (shm_reader_t *reader, void **buf);
int shm_reader_buffer_timedwait (shm_reader_t *reader, const struct timespec *abstime, void **buf);
#endif // __SHM_CHANNEL__
