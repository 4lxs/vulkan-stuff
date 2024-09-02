#---------------------------------
# Download CPM.cmake
#---------------------------------

set(CPM_USE_LOCAL_PACKAGES true)
# set(CPM_LOCAL_PACKAGES_ONLY true)

file(
  DOWNLOAD
  https://github.com/cpm-cmake/CPM.cmake/releases/download/v0.38.3/CPM.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake
  EXPECTED_HASH
    SHA256=cc155ce02e7945e7b8967ddfaff0b050e958a723ef7aad3766d368940cb15494)
include(${CMAKE_CURRENT_BINARY_DIR}/cmake/CPM.cmake)

#---------------------------------
# Dependencies
#---------------------------------

cpmaddpackage("gh:fmtlib/fmt#9.1.0")

cpmaddpackage(
  VERSION
  1.11.0
  GITHUB_REPOSITORY
  "gabime/spdlog"
  OPTIONS
  "SPDLOG_FMT_EXTERNAL ON")

cpmaddpackage("gh:catchorg/Catch2@3.3.2")

cpmaddpackage("gh:CLIUtils/CLI11@2.3.2")

# don't work
#   cpmaddpackage("gh:spnda/fastgltf@0.8.0")

cpmaddpackage(
  NAME
  SDL2
  GITHUB_REPOSITORY
  libsdl-org/SDL
  GIT_TAG
  release-2.30.6
  OPTIONS
  "SDL2_DISABLE_INSTALL ON"
  "SDL_SHARED OFF"
  "SDL_STATIC ON"
  "SDL_STATIC_PIC ON"
  "SDL_WERROR OFF")

#   cpmaddpackage(
#     NAME
#     glm
#     GITHUB_REPOSITORY
#     gh:g-truc/glm
#     GIT_TAG
#     bf71a834948186f4097caa076cd2663c69a10e1e)
#
#   target_compile_definitions(
#     glm::glm
#     INTERFACE GLM_FORCE_SWIZZLE
#               GLM_FORCE_RADIANS
#               GLM_FORCE_CTOR_INIT
#               GLM_ENABLE_EXPERIMENTAL)

cpmaddpackage("gh:GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator@3.1.0")

find_package(Vulkan REQUIRED)

add_subdirectory(third_party)
