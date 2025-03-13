# Top level CMake file for the project

# @REGION PROJECT SETUP

cmake_minimum_required (VERSION 3.27)

# Any output by targets -> binary dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# Project name
project ("VulkanProject")

# Set the name of the executable
set(EXECUTABLE_NAME ${PROJECT_NAME})

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
add_subdirectory ("VulkanProject")
message(STATUS "VulkanProject created! ")

# @ENDREGION SOURCE FILES & LIBRARIES


# @REGION PLATFORMS AND RESOURCES
# Nothing here yet
# @ENDREGION PLATFORMS AND RESOURCES

# on Visual Studio, set our app as the default project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VulkanProject)




