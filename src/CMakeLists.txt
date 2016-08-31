cmake_minimum_required(VERSION 2.8.4)

set(SHM_SRC shm_comm.c shm_channel.c)

add_library(shm_comm SHARED ${SHM_SRC})

set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

install(TARGETS shm_comm
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)
        
install(FILES shm_channel.h DESTINATION include/shm_comm)