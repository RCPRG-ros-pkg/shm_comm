###############################################################################
# CMakeLists.txt
#
# Defines build structure for examples to shm_comm library
###############################################################################

add_executable(shm_comm_example example.c)
target_link_libraries(shm_comm_example PRIVATE shm_comm pthread rt)

# add_executable(shm_comm_diag diag.c)
# target_link_libraries(shm_comm_diag PRIVATE shm_comm pthread rt)

# add_executable(shm_comm_test src/test.c)
# target_link_libraries(shm_comm_test PRIVATE shm_comm pthread rt)

# add_executable(shm_comm_test2 src/test2.c)
# target_link_libraries(shm_comm_test2 PRIVATE shm_comm pthread rt)

###############################################################################
# Intallation config
###############################################################################

install(TARGETS shm_comm_example
    RUNTIME
        DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
