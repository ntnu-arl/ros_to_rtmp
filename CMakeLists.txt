cmake_minimum_required(VERSION 3.0.2)
project(ros_to_rtmp)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rospy
  sensor_msgs
  cv_bridge
  image_transport
)

find_package(OpenCV REQUIRED)

catkin_package()


include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
)

add_executable(ros_image_to_rtmp_node
  src/ros_image_to_rtmp.cpp
)

target_link_libraries(ros_image_to_rtmp_node
  ${catkin_LIBRARIES}
  ${OpenCV_LIBRARIES}
)

