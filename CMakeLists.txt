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
add_executable(shm_comm_diag src/diag.c src/shm_comm.c src/shm_channel.c)

target_link_libraries(shm_comm ${catkin_LIBRARIES} pthread rt)
target_link_libraries(shm_comm_example ${catkin_LIBRARIES} pthread rt)
target_link_libraries(shm_comm_diag ${catkin_LIBRARIES} pthread rt)

### Orocos Package Exports and Install Targets ###

install(TARGETS shm_comm shm_comm_example shm_comm_diag
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
)

