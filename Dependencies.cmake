include(cmake/CPM.cmake)

# Done as a function so that updates to variables like
# CMAKE_CXX_FLAGS don't propagate out to other
# targets
function(myproject_setup_dependencies)

  if(NOT TARGET fmtlib::fmtlib)
    cpmaddpackage("gh:fmtlib/fmt#9.1.0")
  endif()

  if(NOT TARGET spdlog::spdlog)
    cpmaddpackage(
      NAME
      spdlog
      VERSION
      1.11.0
      GITHUB_REPOSITORY
      "gabime/spdlog"
      OPTIONS
      "SPDLOG_FMT_EXTERNAL ON")
  endif()

  if(NOT TARGET Catch2::Catch2WithMain)
    cpmaddpackage("gh:catchorg/Catch2@3.3.2")
  endif()

  if(NOT TARGET CLI11::CLI11)
    cpmaddpackage("gh:CLIUtils/CLI11@2.3.2")
  endif()

  # don't work
  # if(NOT TARGET fastgltf::fastgltf)
  #   cpmaddpackage("gh:spnda/fastgltf@0.8.0")
  # endif()

  if(NOT TARGET SDL2::SDL2)
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
  endif()
  find_package(SDL2 REQUIRED)

  # if(NOT TARGET GLM::GLM)
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
  # endif()

endfunction()

add_subdirectory(third_party)
