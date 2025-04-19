# Engine CMake file

set(ENGINE_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Public")
set(ENGINE_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Private")

# Define directories for public and private headers
# Gather all source files

file(GLOB_RECURSE ENGINE_PRIVATE_SOURCES
    "${ENGINE_PRIVATE_DIR}/*.cpp"
)

file(GLOB_RECURSE ENGINE_PRIVATE_HEADERS
    "${ENGINE_PRIVATE_DIR}/*.h"
)

file(GLOB_RECURSE ENGINE_PUBLIC_HEADERS
    "${ENGINE_PUBLIC_DIR}/*.h"
)

# Create the Engine target (as a library)
add_library(Engine STATIC
    ${ENGINE_PRIVATE_SOURCES}
    ${ENGINE_PRIVATE_HEADERS}
    ${ENGINE_PUBLIC_HEADERS}
)

target_include_directories(Engine
    PUBLIC
        ${ENGINE_PUBLIC_DIR}        # Public headers for Engine
    PRIVATE
        ${ENGINE_PRIVATE_DIR}       # Private headers for Engine
        "${CMAKE_CURRENT_SOURCE_DIR}/../Renderer/Shared"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Core/Shared"
)

# Link the Renderer to the Engine
target_link_libraries(Engine
    PUBLIC
        MauEngCore
        Renderer
    PRIVATE
)

target_sources(Engine
    PUBLIC
        $<TARGET_OBJECTS:ECS>
)

# Enable precompiled header support for Engine
target_precompile_headers(Engine
    PRIVATE 
    "${ENGINE_PRIVATE_DIR}/EnginePCH.h"
    PUBLIC
    "${ENGINE_PUBLIC_DIR}/PCH.h"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public" PREFIX "Public" FILES ${ENGINE_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${ENGINE_PRIVATE_HEADERS} ${ENGINE_PRIVATE_SOURCES})