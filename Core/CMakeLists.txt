# Core CMakeLists.txt

set(CORE_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Public")
set(CORE_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Private")

# Gather all source files
file(GLOB CORE_PUBLIC_SOURCES
    "${RENDERER_PUBLIC_DIR}/*.cpp"
)

file(GLOB CORE_PRIVATE_SOURCES
    "${CORE_PUBLIC_DIR}/Vulkan/*.cpp"
    "${CORE_PRIVATE_DIR}/*.cpp"
)

add_library(MauEngCore STATIC
    ${CORE_PUBLIC_SOURCES}
    ${CORE_PRIVATE_SOURCES}
)

target_link_libraries(MauEngCore
    PRIVATE
        Libs  # Link against Libs (or any other dependencies in Libs)
)


target_include_directories(MauEngCore
    PUBLIC
        ${CORE_PUBLIC_DIR}        # Public headers for Core
    PRIVATE
        ${CORE_PRIVATE_DIR}       # Private headers for Core
)

target_precompile_headers(MauEngCore PRIVATE 
    "${CORE_PUBLIC_DIR}/CorePCH.h"
)
