cmake_minimum_required(VERSION 3.29)
project(
  vulkan
  VERSION 0.0.1
  LANGUAGES CXX)

if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds are not allowed.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(NDEBUG)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-nullability-extension")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

find_package(fmt REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_executable(vulkan)

add_subdirectory(src)
install(TARGETS vulkan DESTINATION bin)
