
include_directories(../src)

add_executable(main main.c)

target_link_libraries(main shm_comm pthread rt)