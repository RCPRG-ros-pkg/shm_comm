###############################################################################
# CMakeLists.txt
#
# Contains definition of test targets for shm_comm library
###############################################################################

###############################################################################
# Test executable target definition
###############################################################################

add_executable(shm_comm_test
    Channel_test.cpp
    Reader_test.cpp
    Writer_test.cpp

    test_main.cpp
)

target_link_libraries(shm_comm_test
    PRIVATE
        shm_comm
        doctest
)

target_compile_options(shm_comm_test
    PRIVATE
        -Wall -Wextra -pedantic
)

###############################################################################
# Test definition
###############################################################################

add_test(shm_comm_test shm_comm_test)

###############################################################################
# Installation configuration
###############################################################################

message(STATUS "CATKIN_PACKAGE_BIN_DESTINATION=${CATKIN_PACKAGE_BIN_DESTINATION}")

install(TARGETS shm_comm_test
    RUNTIME
        DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)
