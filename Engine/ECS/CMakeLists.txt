# Renderer CMake file

set(CMAKE_CXX_STANDARD 20)  # C++20 is the closest to C++2, since C++2 is not an official version
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ECS_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Public")
set(ECS_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Private")

# Gather all source files
file(GLOB ECS_PUBLIC_SOURCES
    "${ECS_PUBLIC_DIR}/*.cpp"
    "${ECS_PUBLIC_DIR}/**/*.cpp"
)

file(GLOB ECS_PRIVATE_SOURCES
    "${ECS_PRIVATE_DIR}/*.cpp"
    "${ECS_PRIVATE_DIR}/**/*.cpp"
)

# Create the Engine target (as a library)
add_library(ECS STATIC
    ${ECS_PUBLIC_SOURCES}
    ${ECS_PRIVATE_SOURCES}
)

add_library(entt INTERFACE)
target_include_directories(entt INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Entt/single_include)

target_link_libraries(ECS
   PUBLIC MauEngCore
   PRIVATE entt
)

target_include_directories(ECS
    PUBLIC
        ${ECS_PUBLIC_DIR}        # Public headers for ECS
    PRIVATE  
        ${ECS_PRIVATE_DIR}       # Private headers for ECS
)