cmake_minimum_required(VERSION 3.10)
project(max_inscribed_rectangle_detector CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

find_package(OpenCV REQUIRED)

add_executable(max_inscribed_rect src/main.cpp)
target_include_directories(max_inscribed_rect PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(max_inscribed_rect ${OpenCV_LIBS})
