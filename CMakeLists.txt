cmake_minimum_required(VERSION 2.8.4)

project(shm_comm)

find_package(catkin REQUIRED COMPONENTS)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES shm_comm
#  DEPENDS roscpp geometry_msgs visualization_msgs message_runtime urdf Eigen orocos_kdl kdl_parser
)

include_directories(include)

add_library(shm_comm src/shm_comm.c src/shm_channel.c)
add_executable(shm_comm_example examples/main.c src/shm_comm.c src/shm_channel.c)

target_link_libraries(shm_comm ${catkin_LIBRARIES} pthread rt)
target_link_libraries(shm_comm_example ${catkin_LIBRARIES} pthread rt)


