
#include <stdatomic.h>

#define FALSE 0
#define TRUE 1

#define CHANNEL_DATA_SIZE(S, R) (sizeof(channel_t) + (R) * sizeof(int) + (R) * sizeof(int) + ((R) + 2) * (S))

typedef struct {
	atomic_int latest;
	atomic_int readers; // number of connected readers
	int max_readers; // number of allocated readers
	unsigned int size; // size of buffer element
	atomic_int *reader_ids; // contain information which reader ids are in use
	atomic_int *reading; // readers state array
	char *buffer; // data buffers (readers + 2)
} channel_t;

typedef struct {
	unsigned int index; // index of currently used write buffer
	int *inuse; // pointer to inuse array (readers)
	channel_t *channel; // pointer to corresponding channel structure
} writer_t;

typedef struct {
	unsigned int id; // reader id
	channel_t *channel; // pointer to corresponding channel structure
} reader_t;

int init_channel(unsigned int, int, channel_t *);

int create_writer(channel_t *, writer_t *);

void release_writer(writer_t *wr);

void *writer_buffer_get(writer_t *wr);

int writer_buffer_write(writer_t *wr);

int create_reader(channel_t *, reader_t *);

void release_reader(reader_t *);

void *reader_buffer_get(reader_t *);
