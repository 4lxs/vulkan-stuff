cmake_minimum_required(VERSION 3.29)

if(NOT DEFINED CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 23)
endif()

set(CMAKE_CXX_EXTENSIONS OFF)

project(
  vulkanplayground
  VERSION 0.0.1
  DESCRIPTION ""
  LANGUAGES CXX C)

include(cmake/PreventInSourceBuilds.cmake)
include(ProjectOptions.cmake)
include(Dependencies.cmake)

myproject_setup_options()
myproject_global_options()
myproject_setup_dependencies()
myproject_local_options()

set(CMAKE_CXX_VISIBILITY_PRESET hidden)

target_compile_features(myproject_options
                        INTERFACE cxx_std_${CMAKE_CXX_STANDARD})

add_library(myproject::myproject_options ALIAS myproject_options)
add_library(myproject::myproject_warnings ALIAS myproject_warnings)

add_subdirectory(configured_files)

add_subdirectory(src)
