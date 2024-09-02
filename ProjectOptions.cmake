include(CMakeDependentOption)
include(CheckCXXCompilerFlag)

if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES
                                                 ".*GNU.*") AND NOT WIN32)
  set(SUPPORTS_UBSAN ON)
else()
  set(SUPPORTS_UBSAN OFF)
endif()

if((CMAKE_CXX_COMPILER_ID MATCHES ".*Clang.*" OR CMAKE_CXX_COMPILER_ID MATCHES
                                                 ".*GNU.*") AND WIN32)
  set(SUPPORTS_ASAN OFF)
else()
  set(SUPPORTS_ASAN ON)
endif()

if(NOT PROJECT_IS_TOP_LEVEL OR myproject_PACKAGING_MAINTAINER_MODE)
  option(myproject_WARNINGS_AS_ERRORS "Treat Warnings As Errors" OFF)
  option(myproject_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer" OFF)
  option(myproject_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
  option(myproject_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer" OFF)
  option(myproject_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
  option(myproject_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
  option(myproject_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
  option(myproject_ENABLE_CLANG_TIDY "Enable clang-tidy" OFF)
  option(myproject_ENABLE_CPPCHECK "Enable cpp-check analysis" OFF)
  option(myproject_ENABLE_PCH "Enable precompiled headers" OFF)
  option(myproject_ENABLE_CACHE "Enable ccache" OFF)
else()
  option(myproject_WARNINGS_AS_ERRORS "Treat Warnings As Errors" ON)
  option(myproject_ENABLE_SANITIZER_ADDRESS "Enable address sanitizer"
         ${SUPPORTS_ASAN})
  option(myproject_ENABLE_SANITIZER_LEAK "Enable leak sanitizer" OFF)
  option(myproject_ENABLE_SANITIZER_UNDEFINED "Enable undefined sanitizer"
         ${SUPPORTS_UBSAN})
  option(myproject_ENABLE_SANITIZER_THREAD "Enable thread sanitizer" OFF)
  option(myproject_ENABLE_SANITIZER_MEMORY "Enable memory sanitizer" OFF)
  option(myproject_ENABLE_UNITY_BUILD "Enable unity builds" OFF)
  option(myproject_ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
  option(myproject_ENABLE_CPPCHECK "Enable cpp-check analysis" ON)
  option(myproject_ENABLE_PCH "Enable precompiled headers" OFF)
  option(myproject_ENABLE_CACHE "Enable ccache" ON)
endif()

if(NOT PROJECT_IS_TOP_LEVEL)
  mark_as_advanced(
    myproject_WARNINGS_AS_ERRORS
    myproject_ENABLE_SANITIZER_ADDRESS
    myproject_ENABLE_SANITIZER_LEAK
    myproject_ENABLE_SANITIZER_UNDEFINED
    myproject_ENABLE_SANITIZER_THREAD
    myproject_ENABLE_SANITIZER_MEMORY
    myproject_ENABLE_UNITY_BUILD
    myproject_ENABLE_CLANG_TIDY
    myproject_ENABLE_CPPCHECK
    myproject_ENABLE_PCH
    myproject_ENABLE_CACHE)
endif()

macro(myproject_local_options)
  if(PROJECT_IS_TOP_LEVEL)
    include(cmake/StandardProjectSettings.cmake)
  endif()

  add_library(myproject_warnings INTERFACE)
  add_library(myproject_options INTERFACE)

  include(cmake/CompilerWarnings.cmake)
  myproject_set_project_warnings(
    myproject_warnings
    ${myproject_WARNINGS_AS_ERRORS}
    ""
    ""
    ""
    "")

  include(cmake/Sanitizers.cmake)
  myproject_enable_sanitizers(
    myproject_options
    ${myproject_ENABLE_SANITIZER_ADDRESS}
    ${myproject_ENABLE_SANITIZER_LEAK}
    ${myproject_ENABLE_SANITIZER_UNDEFINED}
    ${myproject_ENABLE_SANITIZER_THREAD}
    ${myproject_ENABLE_SANITIZER_MEMORY})

  set_target_properties(myproject_options
                        PROPERTIES UNITY_BUILD ${myproject_ENABLE_UNITY_BUILD})

  if(myproject_ENABLE_PCH)
    target_precompile_headers(
      myproject_options
      INTERFACE
      <vector>
      <string>
      <utility>)
  endif()

  if(myproject_ENABLE_CACHE)
    include(cmake/Cache.cmake)
    myproject_enable_cache()
  endif()

  include(cmake/StaticAnalyzers.cmake)
  if(myproject_ENABLE_CLANG_TIDY)
    myproject_enable_clang_tidy(myproject_options
                                ${myproject_WARNINGS_AS_ERRORS})
  endif()

  if(myproject_ENABLE_CPPCHECK)
    myproject_enable_cppcheck(
      ${myproject_WARNINGS_AS_ERRORS} "" # override cppcheck options
    )
  endif()

  if(myproject_WARNINGS_AS_ERRORS)
    check_cxx_compiler_flag("-Wl,--fatal-warnings" LINKER_FATAL_WARNINGS)
    if(LINKER_FATAL_WARNINGS)
      # This is not working consistently, so disabling for now
      # target_link_options(myproject_options INTERFACE -Wl,--fatal-warnings)
    endif()
  endif()

endmacro()
