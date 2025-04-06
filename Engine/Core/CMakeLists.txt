# Core CMakeLists.txt

set(CORE_PUBLIC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Public")
set(CORE_PRIVATE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Private")

# Gather all source files
file(GLOB CORE_PUBLIC_SOURCES
    "${CORE_PUBLIC_DIR}/*.cpp"
)

file(GLOB CORE_PRIVATE_SOURCES
    "${CORE_PRIVATE_DIR}/*.cpp"
)

add_library(MauEngCore STATIC
    ${CORE_PUBLIC_SOURCES}
    ${CORE_PRIVATE_SOURCES}
)

# Tests & similar
set(DSDL_TESTS OFF)
set(DSDL_TEST_LIBRARY OFF)
set(DSDL_DISABLE_INSTALL_DOCS OFF)
set(DSDL_INSTALL_TESTS OFF)

set(SDL_TESTS OFF)
set(SDL_TEST_LIBRARY OFF)
set(SDL_DISABLE_INSTALL_DOCS OFF)
set(SDL_INSTALL_DOCS OFF)
set(SDL_INSTALL_TESTS OFF)

set(SDL_EXAMPLES OFF)

# We don't want to build shared libraries
set(SDL_SHARED OFF)
set(SDL_STATIC ON)

# We use EXCLUDE_FROM_ALL so that its install targets and configs don't
# pollute upwards into our configuration.
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Libs/SDL" EXCLUDE_FROM_ALL)	
set(SDL3_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/SDL/include/SDL3")

# Add sub directories for any third party libraries
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Libs/glm")

target_link_libraries(MauEngCore
    PUBLIC
        SDL3::SDL3
        glm
)

target_include_directories(MauEngCore
    PUBLIC
        ${CORE_PUBLIC_DIR}        # Public headers for Core
        ${CMAKE_CURRENT_SOURCE_DIR}/Libs/glm
        ${SDL3_INCLUDE_DIRS}
    PRIVATE
        ${CORE_PRIVATE_DIR}       # Private headers for Core
)

target_precompile_headers(MauEngCore PRIVATE 
    "${CORE_PUBLIC_DIR}/CorePCH.h"
)
