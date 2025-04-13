# Configuration CMake file for the project


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Any output by targets -> binary dir
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

if(MSVC)
    if(NOT CMAKE_GENERATOR STREQUAL "Ninja")
        add_definitions(/MP) # parallelize each target, unless Ninja is the generator
    endif()
    # add_compile_options(/W4 /permissive-)
else()
    # add_compile_options(-Wall)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)


option(MAUENG_ENABLE_DEBUG_RENDERING "Enable debug rendering" ON)
option(MAUENG_LOG_TO_FILE "Log to file" OFF)
option(MAUENG_ENABLE_ASSERTS "Enable asserts" ON)

option(MAUENG_ENABLE_PROFILER "Enable profiling" ON)
option(MAUENG_USE_OPTICK "Use Optick instead of custom profiler" ON)

# set(MAUENG_ENABLE_DEBUG_RENDERING ${MAUENG_ENABLE_DEBUG_RENDERING} CACHE BOOL "Enable debug rendering")
# set(MAUENG_LOG_TO_FILE ${MAUENG_LOG_TO_FILE} CACHE BOOL "Log to file")
# set(MAUENG_ENABLE_ASSERTS ${MAUENG_ENABLE_ASSERTS} CACHE BOOL "Enable asserts")

# set(MAUENG_ENABLE_PROFILER ${MAUENG_ENABLE_PROFILER} CACHE BOOL "Enable profiling")
# set(MAUENG_USE_OPTICK ${MAUENG_USE_OPTICK} CACHE BOOL "Use Optick instead of custom profiler")

message(STATUS "Debug config: ")
message(STATUS "MAUENG_ENABLE_DEBUG_RENDERING: ${MAUENG_ENABLE_DEBUG_RENDERING}")
message(STATUS "MAUENG_LOG_TO_FILE: ${MAUENG_LOG_TO_FILE}")
message(STATUS "MAUENG_ENABLE_ASSERTS: ${MAUENG_ENABLE_ASSERTS} \n")

message(STATUS "Profiling config: ")
message(STATUS "MAUENG_ENABLE_PROFILER: ${MAUENG_ENABLE_PROFILER}")
message(STATUS "MAUENG_USE_OPTICK: ${MAUENG_USE_OPTICK} \n")