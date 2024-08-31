add_executable(
  tutorial
  main.cpp
  engine.cpp
  engine.hpp
  object.cpp
  object.hpp
  struct.cpp
  struct.hpp
  helpers.cpp
  helpers.hpp
  impl.cpp
  vulkan/util.cpp
  vulkan/util.hpp
  vulkan/pipelinebuilder.cpp
  vulkan/pipelinebuilder.hpp)

target_compile_definitions(
  tutorial PUBLIC # glm was developed for opengl. vulkan differs. google
                  GLM_FORCE_LEFT_HANDED GLM_FORCE_DEPTH_ZERO_TO_ONE)

# target_sources(${PROJECT_NAME} PUBLIC FILE_SET all_my_modules TYPE CXX_MODULES
# FILES engine.cxx)

find_package(Vulkan REQUIRED)
#
# add_subdirectory(shaders)

target_link_libraries(
  tutorial
  PRIVATE Vulkan::Vulkan
          vk-bootstrap
          glm
          GPUOpen::VulkanMemoryAllocator
          tinyobjloader
          fastgltf
          imgui
          volk
          SDL2
          fmt::fmt
          spdlog::spdlog
          CLI11::CLI11)

target_include_directories(
  tutorial PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
