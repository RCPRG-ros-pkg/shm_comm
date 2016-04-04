
#include "shm_comm.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#define READERS 10
#define DATA_SIZE 100

volatile int stop;

reader_t re[READERS];
writer_t wr;

channel_t *shm_data;


static void hdl(int sig)
{
	stop = 1;
}

void *reader_thread(void *t) {		
	int ret = create_reader(shm_data, &re[(size_t)t]);
	
	if (ret != 0) {

		if (ret == -1)
			printf("invalid reader_t pointer\n");

		if (ret == -2)
			printf("no reader slots avalible\n");
		return 0;
	}
	
	while(stop == 0) {
		char *buf = (char*)reader_buffer_get(&re[(size_t)t]);
		
		if (buf == NULL) {
			printf("reader get NULL buffer\n");
		} else {
		
		
			char c = buf[0];
			for (size_t i=0; i < DATA_SIZE; i++) {
				if (c != buf[i]) {
					abort();
				} 
			}
		}
	}
	
	release_reader(&re[(size_t)t]);
	return 0;
}

void *reader_thread2(void *t) {


	while(stop == 0) {
		int ret = create_reader(shm_data, &re[(size_t)t]);

		if (ret != 0) {
			printf("unable to create reader\n");
			abort();
		}

		for (size_t i = 0; i < READERS; i++) {
			if (i != (size_t)t) {

			}
		}

		release_reader(&re[(size_t)t]);
	}

	return 0;
}

int main() {

	pthread_t readers[READERS];
	
	stop = 0;

    signal(SIGINT, hdl);
	
    shm_data = malloc(CHANNEL_DATA_SIZE(DATA_SIZE, READERS));

    if (shm_data == NULL) {
    	abort();
    }

    init_channel(DATA_SIZE, READERS, shm_data);

	create_writer((channel_t *)shm_data, &wr);
	char *buf = writer_buffer_get(&wr);
	
	memset(buf, 'd', DATA_SIZE);
	
	writer_buffer_write(&wr);
	
	for (size_t i = 0; i < 1; ++i) {
		pthread_create(&readers[i], NULL, reader_thread, (void *)i);
	}
	
	char c = 0;
	
	while (stop == 0) {
		char *buf = writer_buffer_get(&wr);

		if (wr.index == ((channel_t *)shm_data)->reading[0]) {
			abort();
		}

		memset(buf, c, DATA_SIZE);

		writer_buffer_write(&wr);
		c++;
	}
	
	for (size_t i = 0; i < 1; ++i) {
		void *status;
		pthread_join(readers[i], &status);
	}

	release_writer(&wr);

	return 0;
}
