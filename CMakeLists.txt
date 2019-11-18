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
option(SHM_COMM_VERBOSE "Whether to enable verbose messages or not" OFF)

###############################################################################
# Project dependencies
###############################################################################

find_package(catkin REQUIRED)

###############################################################################
# External dependencies
###############################################################################

add_subdirectory(extern)

###############################################################################
# Package configuration
###############################################################################

catkin_package(
    INCLUDE_DIRS include
    LIBRARIES shm_comm
)

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

if(SHM_COMM_VERBOSE)
    target_compile_definitions(shm_comm
        PRIVATE
            -DSHM_COMM_VERBOSE
    )
endif()

###############################################################################
# Examples definitions
###############################################################################

if(SHM_COMM_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

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

install(TARGETS shm_comm
    LIBRARY
        DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)

install(DIRECTORY include/shm_comm
    DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)

