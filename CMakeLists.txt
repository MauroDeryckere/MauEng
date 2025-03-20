# Top level CMake file for the project

# Documentation
# Currently all libs are in a general "libs" sub dir, this is not the cleanest approach and should be refactored as the engine grows

# @REGION PROJECT SETUP

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

cmake_minimum_required (VERSION 3.27)

# Any output by targets -> binary dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Project name
project ("VulkanEngine")

# Set the name of the executable
set(EXECUTABLE_NAME "VulkanEngine")

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP) # parallelize each target, unless Ninja is the generator
    endif()
endif()

# @ENDREGION PROJECT SETUP


# @REGION SOURCE FILES & LIBRARIES

# Include sub-projects.
add_subdirectory ("Libs")
message(STATUS "Libs dir created! ")

add_subdirectory ("Engine")
message(STATUS "Engine dir created! ")

add_subdirectory ("Game")
message(STATUS "Game dir created! ")

# @ENDREGION SOURCE FILES & LIBRARIES


# @REGION PLATFORMS AND RESOURCES
# Nothing here yet
# @ENDREGION PLATFORMS AND RESOURCES

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Game)




