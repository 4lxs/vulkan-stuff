set(NAME testengine1)

add_executable(
  ${NAME}
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
  ${NAME} PUBLIC # glm was developed for opengl. vulkan differs. google
                 GLM_FORCE_LEFT_HANDED GLM_FORCE_DEPTH_ZERO_TO_ONE)

target_link_libraries(
  ${NAME}
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
  ${NAME} PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
