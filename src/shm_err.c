#include "shm_comm/shm_err.h"

const char* shm_strerror(int error_code)
{
    switch(error_code)
    {
        case SHM_INVAL:
            return "Invalid argument";
        case SHM_FATAL:
            return "Fatal error";
        case SHM_NO_CHANNEL:
            return "Channel not found";
        case SHM_CHANNEL_INCONSISTENT:
            return "Channel is inconsistent";
        case SHM_ERR_INIT:
            return "Channel initialization error";
        case SHM_ERR_CREATE:
            return "Create channel error";
        case SHM_TIMEOUT:
            return "Channel access timeout";
        case SHM_NEWDATA:
            return "New data found in the channel";
        case SHM_OLDDATA:
            return "Old data found in the channel";
        case SHM_NODATA:
            return "No data found in the channel";
        default:
            return "Unknown error";
    }
}
