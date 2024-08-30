project(third_party LANGUAGES C CXX)

# sdl3
# add_subdirectory(sdl EXCLUDE_FROM_ALL)

# sdl2
find_package(SDL2 REQUIRED)

# glm
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

target_compile_definitions(
  glm INTERFACE GLM_FORCE_SWIZZLE GLM_FORCE_RADIANS GLM_FORCE_CTOR_INIT
                GLM_ENABLE_EXPERIMENTAL)

# fastgltf
add_library(fastgltf INTERFACE)
target_include_directories(fastgltf INTERFACE fastgltf)

# vulkan
find_package(Vulkan REQUIRED)

# vma
add_subdirectory(vma)

# volk
add_subdirectory(volk EXCLUDE_FROM_ALL)

# imgui
add_library(imgui STATIC)
target_include_directories(imgui PUBLIC imgui)

target_sources(
  imgui
  PRIVATE imgui/imgui.h
          imgui/imgui.cpp
          imgui/imgui_demo.cpp
          imgui/imgui_draw.cpp
          imgui/imgui_widgets.cpp
          imgui/imgui_tables.cpp
          imgui/backends/imgui_impl_vulkan.cpp
          # imgui/backends/imgui_impl_sdl3.cpp
          imgui/backends/imgui_impl_sdl2.cpp)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan SDL2::SDL2)

# fmt
find_package(fmt REQUIRED)

# vk-bootstrap
add_subdirectory(vk-bootstrap EXCLUDE_FROM_ALL)
