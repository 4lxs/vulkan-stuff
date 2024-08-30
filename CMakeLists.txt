cmake_minimum_required(VERSION 3.29)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
endif()

set(CMAKE_CXX_EXTENSIONS ON)

project(
  vulkan
  VERSION 0.0.1
  LANGUAGES CXX)

include(cmake/PreventInSourceBuilds.cmake)
include(cmake/ProjectOptions.cmake)

add_subdirectory(third_party)
add_subdirectory(src)
install(TARGETS vulkan DESTINATION bin)
