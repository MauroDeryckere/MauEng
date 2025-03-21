# Top level CMake file for the project

# Documentation
# Currently all libs are in a general "libs" sub dir, this is not the cleanest approach and should be refactored as the engine grows

# @REGION PROJECT SETUP

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required (VERSION 3.27)

# Any output by targets -> binary dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Project name
project ("VulkanEngine")

# Set the name of the executable
set(EXECUTABLE_NAME "VulkanEngine")

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP) # parallelize each target, unless Ninja is the generator
    endif()
endif()

# @ENDREGION PROJECT SETUP


# @REGION SOURCE FILES & LIBRARIES

# Include sub-projects.
add_subdirectory ("Libs")
message(STATUS "Libs dir created! ")

add_subdirectory ("Engine")
message(STATUS "Engine dir created! ")

add_subdirectory ("Game")
message(STATUS "Game dir created! ")

# @ENDREGION SOURCE FILES & LIBRARIES


# @REGION PLATFORMS AND RESOURCES

# Define where the shader files are located

set(SHADER_DIR "${CMAKE_SOURCE_DIR}/resources/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders")

# Find the GLSL compiler (glslc)
find_program(GLSLC glslc REQUIRED)

# List your shaders here
set(SHADERS
    ${SHADER_DIR}/shader.vert
    ${SHADER_DIR}/shader.frag
)

# Declare a list to hold all compiled shader output files
set(COMPILED_SHADERS)

# Compile each shader
foreach(shader ${SHADERS})
    # Determine the output file path for the compiled shader (SPIR-V format)
    get_filename_component(shader_name ${shader} NAME) # Get the base filename without extension
    set(output_shader "${SHADER_OUTPUT_DIR}/${shader_name}.spv")

    # Add the output shader file to the list
    list(APPEND COMPILED_SHADERS ${output_shader})

        # Only add a custom command if this shader file hasn't been processed yet
        add_custom_command(
            OUTPUT ${output_shader}
            COMMAND ${GLSLC} -o ${output_shader} ${shader}
            DEPENDS ${shader}
            COMMENT "Compiling shader ${shader}"
            VERBATIM
        )
endforeach()

# Add a custom target to ensure shaders are compiled during the build
add_custom_target(
    shaders ALL
    DEPENDS ${COMPILED_SHADERS}
)

# Ensure your main project depends on the shader compilation
add_dependencies(${EXECUTABLE_NAME} shaders)

# @ENDREGION PLATFORMS AND RESOURCES

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Game)




