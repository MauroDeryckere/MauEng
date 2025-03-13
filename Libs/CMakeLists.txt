# Add sub directories for any third party libraries

cmake_minimum_required(VERSION 3.27)
project(Libs)

add_library(Libs INTERFACE)

add_subdirectory(glm)
target_link_libraries(Libs INTERFACE glm)

add_subdirectory(glfw)
target_link_libraries(Libs INTERFACE glfw)

# Set common include directories if needed

