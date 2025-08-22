cmake_minimum_required(VERSION 3.8)
project(concentric_circles_rviz_plugin)


if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
add_compile_options(-Wall -Wextra -Wpedantic)
endif()


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)


find_package(ament_cmake_auto REQUIRED)

ament_auto_find_build_dependencies()


include_directories(include)


ament_auto_add_library(${PROJECT_NAME} SHARED
  src/concentric_circles_display.cpp
  include/${PROJECT_NAME}/concentric_circles_display.hpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)


install(TARGETS ${PROJECT_NAME}
ARCHIVE DESTINATION lib
LIBRARY DESTINATION lib
RUNTIME DESTINATION bin
)


install(DIRECTORY include/
DESTINATION include
)


install(FILES plugin_description.xml
DESTINATION share/${PROJECT_NAME}
)

# Register plugin description for rviz_common pluginlib discovery
pluginlib_export_plugin_description_file(rviz_common plugin_description.xml)

ament_auto_package()
