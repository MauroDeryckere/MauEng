# Renderer CMake file

set(RENDERER_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/public")
set(RENDERER_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/private")

# Define directories for public and private headers

# Gather all source files
file(GLOB RENDERER_PUBLIC_SOURCES
    "${RENDERER_PUBLIC_DIR}/*.cpp"
)

file(GLOB RENDERER_PRIVATE_SOURCES
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
        Vulkan::Vulkan # Link Vulkan SDK
)

target_include_directories(Renderer
    PUBLIC
        ${RENDERER_PUBLIC_DIR}        # Public headers for Renderer
    PRIVATE
        ${RENDERER_PRIVATE_DIR}       # Private headers for Renderer
)