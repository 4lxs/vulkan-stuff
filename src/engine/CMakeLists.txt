set(NAME engine)

add_executable(
  ${NAME}
  main.cpp
  engine.cpp
  engine.hpp
  bootstrap.hpp
  bootstrap.cpp
  helpers.cpp
  helpers.hpp
  common.hpp
  common.cpp
  # object.cpp
  # object.hpp
  struct.cpp
  struct.hpp
  # viking_room.cpp
  # viking_room.hpp
  # monkey_head.cpp
  # monkey_head.hpp
  # loadMesh.hpp
  # loadMesh.cpp
  # gltf.hpp
  # gltf.cpp
  impl.cpp
  # vulkan/util.cpp
  # vulkan/util.hpp
  # vulkan/pipelinebuilder.cpp
  # vulkan/pipelinebuilder.hpp
  vulkan/swapchain.cpp
  vulkan/swapchain.hpp)

target_compile_definitions(
  ${NAME}
  PUBLIC VULKAN_HPP_RAII_NO_EXCEPTIONS
         VULKAN_HPP_NO_EXCEPTIONS
         VULKAN_HPP_NO_CONSTRUCTORS
         VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
         VULKAN_HPP_NO_STRUCT_SETTERS
         VULKAN_HPP_HAS_SPACESHIP_OPERATOR
         # glm was developed for opengl. vulkan differs. google
         # GLM_FORCE_LEFT_HANDED GLM_FORCE_DEPTH_ZERO_TO_ONE
)

target_link_libraries(
  ${NAME}
  PRIVATE Vulkan::Vulkan
          vk-bootstrap
          glm
          GPUOpen::VulkanMemoryAllocator
          # tinyobjloader
          # fastgltf::fastgltf
          imgui
          # volk
          SDL2
          fmt::fmt
          spdlog::spdlog
          # CLI11::CLI11
)

target_include_directories(
  ${NAME} PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include")
