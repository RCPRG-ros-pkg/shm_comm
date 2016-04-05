#include "shm_comm.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#define READERS 10
#define DATA_SIZE 100

volatile int stop;

reader_t re[READERS];
writer_t wr;

channel_t *shm_data;

static void hdl(int sig) {
	stop = 1;
}

void *reader_thread(void *t) {
	int ret = create_reader(shm_data, &re[(size_t) t]);

	if (ret != 0) {

		if (ret == -1)
			printf("invalid reader_t pointer\n");

		if (ret == -2)
			printf("no reader slots avalible\n");
		return 0;
	}

	while (stop == 0) {
		char *buf = (char*) reader_buffer_get(&re[(size_t) t]);

		if (buf == NULL) {
			printf("reader get NULL buffer\n");
		} else {

			char c = buf[0];
			for (size_t i = 0; i < DATA_SIZE; i++) {
				if (c != buf[i]) {
					abort();
				}
			}
		}
	}

	release_reader(&re[(size_t) t]);
	return 0;
}

void *reader_thread2(void *t) {

	while (stop == 0) {
		int ret = create_reader(shm_data, &re[(size_t) t]);

		if (ret != 0) {
			printf("unable to create reader\n");
			abort();
		}

		for (size_t i = 0; i < READERS; i++) {
			if (i != (size_t) t) {

			}
		}

		release_reader(&re[(size_t) t]);
	}

	return 0;
}

void writer() {
	create_writer((channel_t *) shm_data, &wr);

	char c = 0;
	while (stop == 0) {
		char *buf = writer_buffer_get(&wr);

		if (wr.index == ((channel_t *) shm_data)->reading[0]) {
			abort();
		}

		memset(buf, c, DATA_SIZE);

		writer_buffer_write(&wr);
		c++;
	}

	release_writer(&wr);
}

int main(int argc, char **argv) {
	int size = 100;
	int readers = 10;

	int type; // 0 - channel, 1 - reader, 2 - writer

	char shm_name[200];

	int shm_fd;
	channel_t *shm_data;

	opterr = 0;

	int c;
	while ((c = getopt(argc, argv, "r:s:")) != -1) {
		switch (c) {
		case 'r':
			readers = atoi(optarg);
			break;
		case 's':
			size = atoi(optarg);
			break;
		case '?':
			if (optopt == 'r' || optopt == 's')
				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
			else
				fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
			return 1;
		default:
			abort();
		}
	}

	if ((optind + 2) == argc) {
		if (!strcmp("channel", argv[optind])) {
			type = 0;
		} else if (!strcmp("reader", argv[optind])) {
			type = 1;
		} else if (!strcmp("writer", argv[optind])) {
			type = 2;
		} else {
			return 0;
		}

		if (!strcpy(shm_name, argv[optind+1])) {
			return 0;
		}
	} else {
		printf("not enough arguments\n");
		return 0;
	}

	stop = 0;

	signal(SIGINT, hdl);

	switch (type) {
	case 0:

		printf("creating channel [%s]\n", shm_name);

		shm_fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
		if (shm_fd < 0) {
			fprintf(stderr, "shm_open failed\n");
			perror(NULL);
			return -1;
		}

		if (ftruncate(shm_fd, CHANNEL_DATA_SIZE(size, readers)) != 0) {
			fprintf(stderr, "ftruncate failed\n");
			shm_unlink(shm_name);
			return -1;
		}

		shm_data = mmap(NULL, CHANNEL_DATA_SIZE(size, readers), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

		if (shm_data == MAP_FAILED) {
			fprintf(stderr, "mmap failed\n");
			shm_unlink(shm_name);
			return -1;
		}

		init_channel(size, readers, shm_data);

		while (stop == 0) {
			sleep(1);
		}

		munmap(shm_data, CHANNEL_DATA_SIZE(size, readers));
		shm_unlink(shm_name);
		break;
	case 1:
		break;
	case 2:
		break;
	}




	shm_data = malloc(CHANNEL_DATA_SIZE(DATA_SIZE, READERS));

	if (shm_data == NULL) {
		abort();
	}

	init_channel(DATA_SIZE, READERS, shm_data);


	free(shm_data);



	return 0;
}
