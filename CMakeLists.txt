# Top level CMake file for the project

# Documentation
# Currently all libs are in a general "libs" sub dir, this is not the cleanest approach and should be refactored as the engine grows

# @REGION PROJECT SETUP

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required (VERSION 3.27)

# Any output by targets -> binary dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIGURATION>")

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


# Define where the layer settings file is located
set(LAYER_SETTINGS_SRC "${CMAKE_SOURCE_DIR}/Engine/Renderer/Config/vk_layer_settings.txt")
set(LAYER_SETTINGS_DEST "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

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


# Define where the shader files are located
set(SHADER_DIR "${CMAKE_SOURCE_DIR}/Resources/Shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Shaders")

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
            COMMAND ${GLSLC} ${shader} -o ${output_shader} || echo "Shader compilation failed: ${shader}"
            DEPENDS ${shader}
            COMMENT "Compiling shader ${shader}"
            VERBATIM
        )
endforeach()

add_custom_target(
    shaders ALL
    DEPENDS ${COMPILED_SHADERS}
)

# Ensure main project depends on the shader compilation
add_dependencies(${EXECUTABLE_NAME} shaders)

# Define texture source and destination directories
set(TEXTURE_DIR "${CMAKE_SOURCE_DIR}/Resources/Textures")
set(TEXTURE_OUTPUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Textures")

file(GLOB TEXTURES "${TEXTURE_DIR}/*.png" "${TEXTURE_DIR}/*.jpg" "${TEXTURE_DIR}/*.bmp")

if(TEXTURES)
    # Declare a list to hold custom commands for copying textures
    set(COPY_TEXTURE_COMMANDS)
    foreach(texture ${TEXTURES})
        # Get the filename without the directory
        get_filename_component(texture_name ${texture} NAME)

        # Define the full destination path for the texture
        set(destination_texture "${TEXTURE_OUTPUT_DIR}/${texture_name}")

        set(destination_texture "${TEXTURE_OUTPUT_DIR}/${texture_name}")
        add_custom_command(
            OUTPUT ${destination_texture}
            COMMAND ${CMAKE_COMMAND} -E copy ${texture} ${destination_texture} || echo "Texture copy failed: ${texture}"
            DEPENDS ${texture}
            COMMENT "Copying texture ${texture_name}"
            VERBATIM
        )

        # Add the custom command to the list
        list(APPEND COPY_TEXTURE_COMMANDS ${destination_texture})
    endforeach()
else()
    message(STATUS "No textures to copy.")
endif()

# Add a custom target for copying textures
    add_custom_target(
        copy_textures ALL
        DEPENDS ${COPY_TEXTURE_COMMANDS}
    )

add_dependencies(${EXECUTABLE_NAME} copy_textures)

# @ENDREGION PLATFORMS AND RESOURCES

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Game)
