# Core CMakeLists.txt

set(CORE_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Public")
set(CORE_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Private")
set(CORE_SHARED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shared")

# Gather all source files
file(GLOB_RECURSE CORE_PRIVATE_SOURCES
    "${CORE_PRIVATE_DIR}/*.cpp"
)

file(GLOB_RECURSE CORE_PRIVATE_HEADERS
    "${CORE_PRIVATE_DIR}/*.h"
)

file(GLOB_RECURSE CORE_SHARED_HEADERS
    "${CORE_SHARED_DIR}/*.h"
)

file(GLOB_RECURSE CORE_PUBLIC_HEADERS
    "${CORE_PUBLIC_DIR}/*.h"
)

add_library(MauEngCore STATIC
    ${CORE_PRIVATE_SOURCES}
    ${CORE_PRIVATE_HEADERS}
    ${CORE_SHARED_HEADERS}
    ${CORE_PUBLIC_HEADERS}
)

# Pass configuration options to the compiler
target_compile_definitions(MauEngCore PUBLIC
    $<$<BOOL:${MAUENG_DISTRIBUTION}>:MAUENG_DISTRIBUTION>

    $<$<BOOL:${MAUENG_ENABLE_DEBUG_RENDERING}>:MAUENG_ENABLE_DEBUG_RENDERING>

    $<$<BOOL:${MAUENG_LOG_TO_FILE}>:MAUENG_LOG_TO_FILE>
    $<$<BOOL:${MAUENG_ENABLE_ASSERTS}>:MAUENG_ENABLE_ASSERTS>
    $<$<BOOL:${MAUENG_USE_IMGUI}>:MAUENG_USE_IMGUI>

    $<$<BOOL:${MAUENG_ENABLE_PROFILER}>:MAUENG_ENABLE_PROFILER>
)

set(MAU_UUID_BUILD_TESTS OFF)
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Libs/UUID")

# Tests & similar
set(DSDL_TESTS OFF)
set(DSDL_TEST_LIBRARY OFF)
set(DSDL_DISABLE_INSTALL_DOCS OFF)
set(DSDL_INSTALL_TESTS OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_TESTS OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_DISABLE_INSTALL_DOCS OFF)
set(SDL_INSTALL_DOCS OFF)
set(SDL_INSTALL_TESTS OFF)

set(SDL_EXAMPLES OFF)

# Rendering 
set(SDL_RENDER OFF)
set(SDL_RENDER_GPU OFF)
set(SDL_RENDER_METAL OFF)
set(SDL_RENDER_D3D OFF)
set(SDL_RENDER_D3D11 OFF)
set(SDL_RENDER_VULKAN OFF)

# Window
set(SDL_CAMERA OFF)


# We don't want to build shared libraries
set(SDL_SHARED OFF)
set(SDL_STATIC ON)

# Add sub directories for any third party libraries

# We use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Libs/SDL" EXCLUDE_FROM_ALL)	
set(SDL3_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/SDL/include/SDL3")

get_property(all_targets DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Libs/SDL" PROPERTY BUILDSYSTEM_TARGETS)
foreach(target ${all_targets})
    set_target_properties(${target} PROPERTIES FOLDER "Libs/SDL")
endforeach()

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Libs/glm")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Libs/fmt")

# Profiler library
include(FetchContent)
set(PROFILER_ENABLED ${MAUENG_ENABLE_PROFILER} CACHE BOOL "" FORCE)
set(PROFILER_USE_OPTICK ${MAUENG_USE_OPTICK} CACHE BOOL "" FORCE)
set(PROFILER_BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(PROFILER_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    Profiler
    GIT_REPOSITORY https://github.com/MauroDeryckere/Profiler.git
    GIT_TAG 47f203a0746b992200d09107606e78d726780b5d
)
FetchContent_MakeAvailable(Profiler)

target_link_libraries(MauEngCore
    PUBLIC
        SDL3::SDL3
        fmt::fmt # While it is not necessary to link against this publicly, users could prefer using this so might as well keep it public
        Profiler::Profiler
        MAU_UUID
    PRIVATE
)

target_include_directories(MauEngCore
    PUBLIC
        ${CORE_PUBLIC_DIR}        # Public headers for Core
        ${CMAKE_CURRENT_SOURCE_DIR}/Libs/glm
        ${SDL3_INCLUDE_DIRS}
    PRIVATE
        ${CORE_PRIVATE_DIR}       # Private headers for Core
        "${CMAKE_CURRENT_SOURCE_DIR}/Shared"
)

target_precompile_headers(MauEngCore PRIVATE 
    "${CORE_PUBLIC_DIR}/CorePCH.h"
)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Public" PREFIX "Public" FILES ${CORE_PUBLIC_HEADERS})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Private" PREFIX "Private" FILES ${CORE_PRIVATE_HEADERS} ${CORE_PRIVATE_SOURCES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Shared" PREFIX "Shared" FILES ${CORE_SHARED_HEADERS})
