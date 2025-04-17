# Renderer CMake file

set(ECS_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Public")
set(ECS_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Private")

# Gather all source files

file(GLOB_RECURSE ECS_PUBLIC_HEADERS 
    "${ECS_PUBLIC_DIR}/*.h"
)
file(GLOB_RECURSE ECS_PRIVATE_HEADERS 
    "${ECS_PRIVATE_DIR}/*.h"
)

file(GLOB_RECURSE ECS_PRIVATE_SOURCES
    "${ECS_PRIVATE_DIR}/*.cpp"
)

# Create the Engine target (as a library)
add_library(ECS STATIC
    ${ECS_PUBLIC_HEADERS}
    ${ECS_PRIVATE_SOURCES}
    ${ECS_PRIVATE_HEADERS}
)

target_link_libraries(ECS
   PUBLIC MauEngCore
)

target_include_directories(ECS
    PUBLIC
        ${ECS_PUBLIC_DIR}        # Public headers for ECS
        ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Entt/single_include
        ${ECS_PRIVATE_DIR}       # Private headers for ECS
    PRIVATE  
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public" PREFIX "Public" FILES ${ECS_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${ECS_PRIVATE_HEADERS} ${ECS_PRIVATE_SOURCES})