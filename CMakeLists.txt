cmake_minimum_required(VERSION 3.29)
project(vulkan)

find_package(fmt REQUIRED)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(src)
install(TARGETS vulkan DESTINATION bin)
