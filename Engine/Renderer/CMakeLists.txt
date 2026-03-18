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


# for loading models, only used in the renderer.
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Disable all importers/exporters by default, then enable only glTF, FBX and OBJ importers
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_FBX_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Libs/Assimp EXCLUDE_FROM_ALL)

# Organize Assimp targets in IDE folder
get_property(assimp_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Libs/Assimp PROPERTY BUILDSYSTEM_TARGETS)
foreach(target ${assimp_targets})
    set_target_properties(${target} PROPERTIES FOLDER "Libs/Assimp")
endforeach()

add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/Libs/stb)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Libs/vma)

target_link_libraries(Renderer
    PUBLIC
        MauENG_ImGUI
        MauENG_ImGUI_Backends
    PRIVATE
        stb
        assimp
        MauEngCore
        Vulkan::Vulkan # Link Vulkan SDK
        GPUOpen::VulkanMemoryAllocator
)

target_include_directories(Renderer
    PUBLIC
        ${RENDERER_PUBLIC_DIR}        # Public headers for Renderer
    PRIVATE  
        ${RENDERER_PRIVATE_DIR}       # Private headers for Renderer
        ${RENDERER_SHARED_DIR}
        "../Core/Shared"
        "${CMAKE_CURRENT_SOURCE_DIR}/Libs/vma/include"
)

# Enable precompiled header support for Engine
target_precompile_headers(Renderer PRIVATE 
    "${RENDERER_PRIVATE_DIR}/RendererPCH.h"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public" PREFIX "Public" FILES ${RENDERER_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${RENDERER_PRIVATE_HEADERS} ${RENDERER_PRIVATE_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Shared" PREFIX "Shared" FILES ${RENDERER_SHARED_HEADERS})
