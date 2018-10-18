#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>

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
    printf("usage: shm_comm_test channel_name1 channel_name2\n");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printUsage();
        return 0;
    }

    const size_t data_size=10;

    char *shm_name1 = argv[1];
    char *shm_name2 = argv[2];


    shm_reader_t* re_1;
    void *buf_prev_1;

    void *pbuf = NULL;

    // reader

    printf("** READER **\n");
    int create_channel_1 = FALSE;
    printf( "trying to connect to channel\n" );
    int result = shm_connect_reader(shm_name1, &re_1);
    if (result == SHM_INVAL) {
        printf( "ERROR: shm_connect_reader('1'): invalid parameters\n" );
        return -5;
    }
    else if (result == SHM_FATAL) {
        printf( "ERROR: shm_connect_reader('1'): memory error\n" );
        return -4;
    }
    else if (result == SHM_NO_CHANNEL) {
        printf( "WARNING: shm_connect_reader('1'): could not open shm object, trying to initialize the channel...\n" );
        create_channel_1 = TRUE;
    }
    else if (result == SHM_CHANNEL_INCONSISTENT) {
        printf( "WARNING: shm_connect_reader('1'): shm channel is inconsistent, trying to initialize the channel...\n" );
        create_channel_1 = TRUE;
    }
    else if (result == SHM_ERR_INIT) {
        printf( "WARNING: shm_connect_reader('1'): could not initialize channel\n" );
        return -3;
    }
    else if (result == SHM_ERR_CREATE) {
        printf( "WARNING: shm_connect_reader('1'): could not create reader\n" );
        create_channel_1 = TRUE;
    }
    else {
        printf( "INFO: connected to channel\n" );
    }
    if (!create_channel_1) {
        printf( "INFO: trying to read from channel\n" );
        pbuf = NULL;
        result = shm_reader_buffer_get(re_1, &pbuf);
        if (result < 0) {
            printf( "WARNING: shm_reader_buffer_get('1'): error: %d\n", result );
            create_channel_1 = TRUE;
        }
        else {
            printf( "INFO: test channel get successful\n" );
        }
    }
    if (create_channel_1) {
        printf( "INFO: trying to create channel\n" );
        result = shm_create_channel(shm_name1, data_size, 1, TRUE);
        if (result != 0) {
            printf( "ERROR: create_shm_object('1'): error: %d, errno: %d\n", result, errno );
            return -1;
        }
        else {
            printf( "INFO: created channel\n" );
        }
        printf( "INFO: trying to connect to channel\n" );
        result = shm_connect_reader(shm_name1, &re_1);
        if (result != 0) {
            printf( "ERROR: shm_connect_reader('1'): error: %d\n", result);
            return -2;
        }
        else {
            printf( "INFO: connected to channel\n" );
        }
    }
    pbuf = NULL;
    result = shm_reader_buffer_get(re_1, &pbuf);
    if (result < 0) {
        printf( "ERROR: shm_reader_buffer_get('1'): error: %d\n", result);
        return -6;
    }
    else {
        printf( "INFO: test channel get successful\n" );
    }
    buf_prev_1 = pbuf;





    // writer
    printf("** WRITER **\n");

    shm_writer_t* wr_2;

    int create_channel_2 = FALSE;
    result = shm_connect_writer(shm_name2, &wr_2);
    if (result == SHM_INVAL) {
        printf( "ERROR: shm_connect_writer(hi_st): invalid parameters\n" );
        return -8;
    }
    else if (result == SHM_FATAL) {
        printf( "ERROR: shm_connect_writer(hi_st): memory error\n" );
        return -9;
    }
    else if (result == SHM_NO_CHANNEL) {
        printf( "WARNING: shm_connect_writer(hi_st): could not open shm object, trying to initialize the channel...\n" );
        create_channel_2 = TRUE;
    }
    else if (result == SHM_CHANNEL_INCONSISTENT) {
        printf( "WARNING: shm_connect_writer(hi_st): shm channel is inconsistent, trying to initialize the channel...\n" );
        create_channel_2 = TRUE;
    }
    else if (result == SHM_ERR_INIT) {
        printf( "ERROR: shm_connect_writer(hi_st): could not initialize channel\n" );
        return -10;
    }
    else if (result == SHM_ERR_CREATE) {
        printf( "ERROR: shm_connect_writer(hi_st): could not create reader\n" );
        return -11;
    }
    else {
        printf( "INFO: connected to channel\n" );
    }
    if (create_channel_2) {
        result = shm_create_channel(shm_name2, data_size, 1, TRUE);
        if (result != 0) {
            printf( "ERROR: create_shm_object(hi_st): error: %d, errno: %d\n", result, errno );
            return -12;
        }
        else {
            printf( "INFO: created channel\n" );
        }
        result = shm_connect_writer(shm_name2, &wr_2);
        if (result != 0) {
            printf( "ERROR: shm_connect_writer(hi_st): error: %d\n", result );
            return -13;
        }
        else {
            printf( "INFO: connected to channel\n" );
        }
    }

    while (TRUE) {
        printf("write\n");
        shm_writer_buffer_write(wr_2);
        void *pbuf2 = NULL;
        shm_writer_buffer_get(wr_2, &pbuf2);



        printf("read\n");
        void *pbuf1 = NULL;
        double timeout_s = 0.1;
        int read_status;
        struct timespec ts;
        read_status = -1;
        clock_gettime(CLOCK_REALTIME, &ts);
        int timeout_sec = (int)timeout_s;
        int timeout_nsec = (int)((timeout_s - (double)timeout_sec) * 1000000000.0);
        ts.tv_sec += timeout_sec;
        ts.tv_nsec += timeout_nsec;
        if (ts.tv_nsec >= 1000000000) {
            ts.tv_nsec -= 1000000000;
            ++ts.tv_sec;
        }
        read_status = shm_reader_buffer_timedwait(re_1, &ts, &pbuf1);
    }

    return 0;
}

