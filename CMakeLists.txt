# Top level CMake file for the project

 cmake_minimum_required(VERSION 3.29)
 set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# @REGION PROJECT SETUP
include("${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake")

# Project name
project ("MauEng")

# Set the name of the executable
set(EXECUTABLE_NAME "MauEng")
add_executable(${EXECUTABLE_NAME})
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${EXECUTABLE_NAME})
set_target_properties(${EXECUTABLE_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

# @ENDREGION PROJECT SETUP

# @REGION SOURCE FILES & LIBRARIES

# Include sub-projects.
add_subdirectory ("Engine")
message(STATUS "Engine dir created! ")

add_subdirectory ("Game")
message(STATUS "Game dir created! \n")

if(${MAUENG_ENABLE_TESTS})
    add_subdirectory("Tests")
    message(STATUS "Tests dir created! \n")
else()
     message(STATUS "Tests are disabled!")
endif()

# @ENDREGION SOURCE FILES & LIBRARIES


# @REGION PLATFORMS AND RESOURCES

# Define where the layer settings file is located
set(LAYER_SETTINGS_SRC "${CMAKE_SOURCE_DIR}/Engine/Renderer/Config/vk_layer_settings.txt")
set(LAYER_SETTINGS_DEST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/vk_layer_settings.txt")

add_custom_command(
    OUTPUT ${LAYER_SETTINGS_DEST}
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${LAYER_SETTINGS_SRC}" "${LAYER_SETTINGS_DEST}"
    DEPENDS ${LAYER_SETTINGS_SRC}
    COMMENT "Copying Vulkan layer settings file"
    VERBATIM
)

add_custom_target(
    copy_layer_settings ALL
    DEPENDS ${LAYER_SETTINGS_DEST}
)

add_dependencies(${EXECUTABLE_NAME} copy_layer_settings)

# Define source and destination for the entire Resources folder
set(RESOURCE_SRC_DIR "${CMAKE_SOURCE_DIR}/Resources")
set(RESOURCE_DST_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Resources")

# Copy the entire Resources folder, including subfolders and files
add_custom_command(
    OUTPUT ${RESOURCE_DST_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${RESOURCE_SRC_DIR}" "${RESOURCE_DST_DIR}"
    COMMENT "Copying all Resources to output directory"
    VERBATIM
)

# Add a custom target for copying the entire Resources folder
add_custom_target(
    copy_resources ALL
    DEPENDS ${RESOURCE_DST_DIR}
)

# Ensure main project depends on the resources copy step
add_dependencies(${EXECUTABLE_NAME} copy_resources)


# Compile shaders (unique step)
set(SHADER_DIR "${CMAKE_SOURCE_DIR}/Resources/Shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Resources/Shaders")
find_program(GLSLC glslc REQUIRED)

# List your shaders here
file(GLOB SHADERS
    "${SHADER_DIR}/*.vert"
    "${SHADER_DIR}/*.frag"
    "${SHADER_DIR}/*.comp"
)

set(COMPILED_SHADERS "")

# Compile each shader
foreach(shader ${SHADERS})
    get_filename_component(shader_name ${shader} NAME)
    set(output_shader "${SHADER_OUTPUT_DIR}/${shader_name}.spv")

    add_custom_command(
        OUTPUT ${output_shader}
        COMMAND ${GLSLC} ${shader} -o ${output_shader} || echo "Shader compilation failed: ${shader}"
        DEPENDS ${shader}
        COMMENT "Compiling shader ${shader}"
        VERBATIM
    )

    list(APPEND COMPILED_SHADERS ${output_shader})
endforeach()

add_custom_target(
    shaders ALL
    DEPENDS ${COMPILED_SHADERS}
)

add_dependencies(${EXECUTABLE_NAME} shaders)

# @ENDREGION PLATFORMS AND RESOURCES
