###############################################################################
# CMakeLists.txt
#
# Defines structure of the project
# Declares main library target and usage examples
###############################################################################

cmake_minimum_required(VERSION 3.10.0)
project(shm_comm LANGUAGES CXX C)

###############################################################################
# Project options
###############################################################################

option(SHM_COMM_BUILD_EXAMPLES "Whether to build examples or not" ON)
option(SHM_COMM_BUILD_TESTS "Whether to build tests or not" ON)

###############################################################################
# Project dependencies
###############################################################################

find_package(catkin REQUIRED)

###############################################################################
# Main library target
###############################################################################

add_library(shm_comm
    src/shm_comm.c
    src/shm_channel.c

    src/Channel.cpp
    src/Reader.cpp
    src/Writer.cpp
)

target_include_directories(shm_comm
    PUBLIC
        include
    PRIVATE
        src
)

target_link_libraries(shm_comm
    PRIVATE
        ${catkin_LIBRARIES}
        pthread
        rt
)

set_target_properties(shm_comm
    PROPERTIES
        C_STANDARD 11
        C_EXTENSIONS ON
        C_STANDARD_REQUIRED ON

        CXX_STANDARD 14
        CXX_EXTENSIONS ON
        CXX_STANDARD_REQUIRED ON
)

###############################################################################
# Examples definitions
###############################################################################

if(SHM_COMM_BUILD_EXAMPLES)
    add_executable(shm_comm_example examples/main.c)
    target_link_libraries(shm_comm_example PRIVATE shm_comm pthread rt)

    add_executable(shm_comm_diag src/diag.c)
    target_link_libraries(shm_comm_diag PRIVATE shm_comm pthread rt)

    # add_executable(shm_comm_test src/test.c)
    # target_link_libraries(shm_comm_test PRIVATE shm_comm pthread rt)

    # add_executable(shm_comm_test2 src/test2.c)
    # target_link_libraries(shm_comm_test2 PRIVATE shm_comm pthread rt)
endif()

###############################################################################
# Package configuration
###############################################################################

catkin_package(
    INCLUDE_DIRS include
    LIBRARIES shm_comm
#  DEPENDS roscpp geometry_msgs visualization_msgs message_runtime urdf Eigen orocos_kdl kdl_parser
)

###############################################################################
# External dependencies
###############################################################################

add_subdirectory(extern)

###############################################################################
# Tests definitions
###############################################################################

if(SHM_COMM_BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()

###############################################################################
# Intallation config
###############################################################################

install(TARGETS shm_comm shm_comm_example shm_comm_diag
    ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY include/${PROJECT_NAME}/
    DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)

