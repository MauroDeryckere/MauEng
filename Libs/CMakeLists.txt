# Add sub directories for any third party libraries

cmake_minimum_required(VERSION 3.27)
project(Libs)

add_library(Libs INTERFACE)

add_subdirectory(glm)
target_link_libraries(Libs INTERFACE glm)

add_subdirectory(glfw)
target_link_libraries(Libs INTERFACE glfw)

# Add the Libs/ directory to the include paths
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE ${CMAKE_SOURCE_DIR}/Libs/stb)
target_link_libraries(Libs INTERFACE stb)


