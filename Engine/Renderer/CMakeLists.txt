# Renderer CMake file

set(CMAKE_CXX_STANDARD 20)  # C++20 is the closest to C++2, since C++2 is not an official version
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RENDERER_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")
set(RENDERER_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/private")

# Define directories for public and private headers

# Gather all source files
file(GLOB RENDERER_PUBLIC_SOURCES
    "${RENDERER_PUBLIC_DIR}/*.cpp"
)

file(GLOB RENDERER_PRIVATE_SOURCES
    "${RENDERER_PRIVATE_DIR}/Vulkan/*.cpp"
    "${RENDERER_PRIVATE_DIR}/*.cpp"
)

# Create the Engine target (as a library)
add_library(Renderer STATIC
    ${RENDERER_PUBLIC_SOURCES}
    ${RENDERER_PRIVATE_SOURCES}
)

# Find Vulkan package
find_package(Vulkan REQUIRED)


target_link_libraries(Renderer
    PRIVATE
        Libs
        PUBLIC # Link Vulkan publicly because we need it fr the window in the engine, later on this can be cleaned up
        Vulkan::Vulkan # Link Vulkan SDK
)

target_include_directories(Renderer
    PUBLIC
        ${RENDERER_PUBLIC_DIR}        # Public headers for Renderer
    PRIVATE
        ${RENDERER_PRIVATE_DIR}       # Private headers for Renderer
)

# Enable precompiled header support for Engine
target_precompile_headers(Renderer PRIVATE 
    "${RENDERER_PRIVATE_DIR}/RendererPCH.h"
)