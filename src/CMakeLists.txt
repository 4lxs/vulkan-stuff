add_executable(
  ${PROJECT_NAME}
  main.cpp
  engine.cpp
  object.cpp
  struct.cpp
  helpers.cpp
  impl.cpp
  vulkan/util.cpp
  vulkan/pipelinebuilder.cpp)

target_compile_definitions(
  ${PROJECT_NAME} PUBLIC # glm was developed for opengl. vulkan differs. google
                         GLM_FORCE_LEFT_HANDED GLM_FORCE_DEPTH_ZERO_TO_ONE)

# target_sources(${PROJECT_NAME} PUBLIC FILE_SET all_my_modules TYPE CXX_MODULES
# FILES engine.cxx)

find_package(Vulkan REQUIRED)
#
# add_subdirectory(shaders)

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE Vulkan::Vulkan
          fmt
          vk-bootstrap
          glm
          GPUOpen::VulkanMemoryAllocator
          tinyobjloader
          fastgltf
          imgui
          volk
          SDL2)
