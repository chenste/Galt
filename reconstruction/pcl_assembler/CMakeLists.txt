cmake_minimum_required(VERSION 2.8.3)
project(pcl_assembler)

add_definitions("-Wall -std=c++11")
find_package(catkin REQUIRED COMPONENTS
    roscpp sensor_msgs tf2 tf2_ros pcl_conversions pcl_ros eigen_conversions
    )

add_service_files(FILES
    SaveToPcdFile.srv
    )

generate_messages(DEPENDENCIES std_msgs)

catkin_package()

include_directories(include ${catkin_INCLUDE_DIRS})

add_executable(pcl_assembler_node src/pcl_assembler_node.cpp)
target_link_libraries(pcl_assembler_node ${catkin_LIBRARIES})

add_executable(pcl_filter_node src/pcl_filter_node.cpp)
target_link_libraries(pcl_filter_node ${catkin_LIBRARIES})

add_dependencies(${PROJECT_NAME}_node
    ${PROJECT_NAME}_gencpp
    ${catkin_EXPORTED_TARGETS}
    )
