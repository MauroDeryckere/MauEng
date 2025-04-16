# Engine CMake file

set(ENGINE_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")
set(ENGINE_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/private")

# Define directories for public and private headers
# Gather all source files
file(GLOB ENGINE_PUBLIC_SOURCES
    "${ENGINE_PUBLIC_DIR}/*.cpp"
    "${ENGINE_PUBLIC_DIR}/**/*.cpp"
)

file(GLOB ENGINE_PRIVATE_SOURCES
    "${ENGINE_PRIVATE_DIR}/**/*.cpp"
    "${ENGINE_PRIVATE_DIR}/*.cpp"
)

# Create the Engine target (as a library)
add_library(Engine STATIC
    ${ENGINE_PUBLIC_SOURCES}
    ${ENGINE_PRIVATE_SOURCES}
)

target_include_directories(Engine
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/../ECS/public"
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
        ECS
    PRIVATE
)

# Enable precompiled header support for Engine
target_precompile_headers(Engine
    PRIVATE 
    "${ENGINE_PRIVATE_DIR}/EnginePCH.h"
    PUBLIC
    "${ENGINE_PUBLIC_DIR}/PCH.h"
)