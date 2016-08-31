#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>

#include "shm_channel.h"

#define READERS 10
#define DATA_SIZE 100

volatile int stop;

static void hdl(int sig) {
    stop = 1;
}


int main(int argc, char **argv) {
    int size = 100;
    int readers = 10;
    int sleep_time = 1;

    int type; // 0 - channel, 1 - reader, 2 - writer

    char shm_name[200];

    shm_reader_t *re;
    shm_writer_t *wr;

    opterr = 0;

    int c;
    while ((c = getopt(argc, argv, "r:s:n:")) != -1) {
        switch (c) {
        case 'r':
            readers = atoi(optarg);
            break;
        case 's':
            size = atoi(optarg);
            break;
        case 'n':
            sleep_time = atoi(optarg);
            break;
        case '?':
            if (optopt == 'r' || optopt == 's' || optopt == 'n')
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

    stop = 1;

    signal(SIGINT, hdl);

    char wdata = 33;

    switch (type) {
    case 0:
		shm_create_channel(shm_name, size, readers);

        while (stop == 0) {
            sleep(sleep_time);
        }

        shm_remove_channel(shm_name);
        break;
    case 1:
        printf("creating reader on channel [%s]\n", shm_name);

        re = shm_connect_reader(shm_name);

        if (re == NULL) {
            return -1;
        }

        while(stop == 0) {
            char* buf = NULL;
            int ret = shm_reader_buffer_wait(re, (void**)&buf);

            if (buf == NULL) {
                printf("reader get NULL buffer\n");
                return -1;
            }
            printf("reading [%c]\n", *buf);
            sleep(sleep_time);
        }

        shm_release_reader(re);
        break;
    case 2:
        printf("creating writer on channel [%s]\n", shm_name);

        wr = shm_connect_writer(shm_name);

        if (wr == NULL) {
            return -1;
        }

        //while(stop == 0) {
            char *buf = NULL;
            shm_writer_buffer_get(wr, (void**)&buf);

            if (buf == NULL) {
                printf("writer get NULL buffer\n");
                return -1;
            }

            memset(buf, wdata, shm_writer_get_size(wr));

            shm_writer_buffer_write(wr);

            printf("writing [%c]\n", wdata);

            wdata++;
            sleep(sleep_time);
        //}

        shm_release_writer(wr);
        break;
    }

    return 0;
}

