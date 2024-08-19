target_sources(${PROJECT_NAME} PRIVATE
               main.cpp)

add_subdirectory(shaders)

# include_directories(${Vulkan_INCLUDE_DIRS})
# target_link_directories(${Vulkan_SDK_PATH} PRIVATE)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glfw
    fmt::fmt)
