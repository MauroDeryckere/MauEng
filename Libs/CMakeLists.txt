# Add sub directories for any third party libraries

cmake_minimum_required(VERSION 3.27)
project(Libs)

add_library(Libs INTERFACE)

add_subdirectory(glm)
target_link_libraries(Libs INTERFACE glm)

# Add the Libs/ directory to the include paths
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_SOURCE_DIR}/Libs/stb)
target_link_libraries(Libs INTERFACE stb)


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
add_subdirectory(SDL EXCLUDE_FROM_ALL)	
set(SDL3_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/SDL/include/SDL3")

target_link_libraries(Libs INTERFACE  SDL3::SDL3)
