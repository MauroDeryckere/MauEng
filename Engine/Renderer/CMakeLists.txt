# Renderer CMake file

set(RENDERER_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Public")
set(RENDERER_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Private")
set(RENDERER_SHARED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shared")

# Gather all source files
file(GLOB_RECURSE RENDERER_PRIVATE_SOURCES
    "${RENDERER_PRIVATE_DIR}/*.cpp"
)

file(GLOB_RECURSE RENDERER_PRIVATE_HEADERS
    "${RENDERER_PRIVATE_DIR}/*.h"
)

file(GLOB_RECURSE RENDERER_SHARED_HEADERS
    "${RENDERER_SHARED_DIR}/*.h"
)

file(GLOB_RECURSE RENDERER_PUBLIC_HEADERS
    "${RENDERER_PUBLIC_DIR}/*.h"
)

# Create the Engine target (as a library)
add_library(Renderer STATIC
    ${RENDERER_PRIVATE_SOURCES}
    ${RENDERER_PRIVATE_HEADERS}
    ${RENDERER_SHARED_HEADERS}
    ${RENDERER_PUBLIC_HEADERS}
)

# Find Vulkan package
find_package(Vulkan REQUIRED)

# Add the Libs/ directory to the include paths

# for loading models, only used in the renderer.
add_library(tinyobjloader INTERFACE)
target_include_directories(tinyobjloader INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Libs/tinyobjloader)

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Libs/stb)

target_link_libraries(Renderer
    PRIVATE
        tinyobjloader # for loading models, only used in the renderer.
        stb
        MauEngCore
        Vulkan::Vulkan # Link Vulkan SDK
)

target_include_directories(Renderer
    PUBLIC
        ${RENDERER_PUBLIC_DIR}        # Public headers for Renderer
    PRIVATE  
        ${RENDERER_PRIVATE_DIR}       # Private headers for Renderer
        ${RENDERER_SHARED_DIR}
        "../Core/Shared"
)

# Enable precompiled header support for Engine
target_precompile_headers(Renderer PRIVATE 
    "${RENDERER_PRIVATE_DIR}/RendererPCH.h"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public" PREFIX "Public" FILES ${RENDERER_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${RENDERER_PRIVATE_HEADERS} ${RENDERER_PRIVATE_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Shared" PREFIX "Shared" FILES ${RENDERER_SHARED_HEADERS})
