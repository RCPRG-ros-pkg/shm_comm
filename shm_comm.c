
#include "shm_comm.h"

#include <stdlib.h>
#include <string.h>

int init_channel(unsigned int size, int readers, channel_t *shdata) {

	shdata->latest = ATOMIC_VAR_INIT(1);
	shdata->readers = ATOMIC_VAR_INIT(0);
	shdata->max_readers = readers;
	shdata->size = size;
	shdata->reader_ids = (atomic_int *)((char *)shdata + sizeof(channel_t));
	shdata->reading = (atomic_int *)((char *)shdata
								+ sizeof(channel_t)
								+ readers * sizeof(atomic_int));
	shdata->buffer = (char *)((char *)shdata
								+ sizeof(channel_t)
								+ readers * sizeof(atomic_int)
								+ readers * sizeof(atomic_int));
	for (size_t i = 0; i < readers; i++) {
		shdata->reader_ids[i] = ATOMIC_VAR_INIT(0);
		shdata->reading[i] = ATOMIC_VAR_INIT(0);
	}

	return 0;
}


int create_writer(channel_t *chan, writer_t *wr) {
	if(chan == NULL) {
		return -1;
	}

	if (wr == NULL) {
		return -1;
	}

	wr->inuse = malloc((chan->max_readers+2)*sizeof(*chan->reading));

	if (wr->inuse == NULL) {
		return -1;
	}

	wr->channel = chan;

	return 0;
};

void release_writer(writer_t *wr) {
	free(wr->inuse);
}

void *writer_buffer_get(writer_t *wr) {
	for (size_t i = 0; i < (wr->channel->max_readers + 2); i++) {
		wr->inuse[i] = FALSE;
	}

	size_t j = atomic_load(&wr->channel->latest) - 1;
	wr->inuse[j] = TRUE;

	for (size_t i = 0; i < wr->channel->max_readers; i++) {
		j = atomic_load(&(wr->channel->reading[i]));
		if ( j != 0) {
			wr->inuse[j-1] = TRUE;
		}
	}

	for (size_t i = 0; i < (wr->channel->max_readers + 2); i++) {
		if(wr->inuse[i] == FALSE) {
			wr->index = i+1;
			return wr->channel->buffer + i * wr->channel->size;
		}
	}

	wr->index = 0;
	return NULL;
}

int writer_buffer_write(writer_t *wr) {
	if (wr == NULL) {
		return -1;
	}

	if (wr->index == 0) {
		return -1;
	}

	atomic_store(&wr->channel->latest, wr->index);

	for (size_t i = 0; i < wr->channel->max_readers; i++) {
		int zero = 0;
		atomic_compare_exchange_strong(&(wr->channel->reading[i]), &zero, wr->index);
	}

	wr->index = 0;

	return 0;
}

int create_reader(channel_t *chan, reader_t *reader) {
	if (chan == NULL) {
		return -1;
	}

	if (reader == NULL) {
		return -1;
	}
	// cheack reader slot avalibility
	int n;
	do {
		n = atomic_load(&chan->readers);

		if (n >= chan->max_readers) {
			return -2;
		}
	} while(!atomic_compare_exchange_strong(&chan->readers, &n, n+1));

	reader->channel = chan;

	// find free reader slot id
	size_t i = 0;
	while (1){
		atomic_int j = atomic_load(&chan->reader_ids[i]);
		if (j == 0) {
			atomic_int zero;
			atomic_store(&zero, 0);
			if (atomic_compare_exchange_strong(&chan->reader_ids[i], &zero, 1)) {
				reader->id = i;
				break;
			}
		}

		if (++i == chan->max_readers) {
			i = 0;
		}
	}

	return 0;
}

void release_reader(reader_t *re) {
	if (re == NULL) {
		return;
	}

	atomic_store(&re->channel->reader_ids[re->id], 0);

	int n;
	do {
		n = atomic_load(&re->channel->readers);
	} while(!atomic_compare_exchange_weak(&re->channel->readers, &n, n-1));
}

void *reader_buffer_get(reader_t * re) {
	int ridx;
	int zero = 0;
	atomic_store(&(re->channel->reading[re->id]),  0);
	ridx = atomic_load(&(re->channel->latest));
	atomic_compare_exchange_strong(&(re->channel->reading[re->id]), &zero, ridx);
	ridx = atomic_load(&(re->channel->reading[re->id]));
	if (ridx == 0) {
		abort();
	}
	return re->channel->buffer + (ridx - 1) * re->channel->size;
}



