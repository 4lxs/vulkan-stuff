set(SOURCES
    basic.vert
    basic.frag)

file (GLOB SHADER_SOURCES ${SOURCES})

set(SHADERS_DIR ${CMAKE_BINARY_DIR}/shaders)

add_custom_command (
    OUTPUT ${SHADERS_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR})

add_custom_target(shaders Vulkan::glslc -c ${SHADER_SOURCES}
    WORKING_DIRECTORY ${SHADERS_DIR}
    DEPENDS ${SHADER_SOURCES}
    VERBATIM)

add_dependencies(${PROJECT_NAME} shaders)
